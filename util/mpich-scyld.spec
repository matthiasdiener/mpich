Summary: Argonne National Laboratory MPI implementation
Name: mpich
Version: 1.2.2
Release: 1
Vendor: Scyld Computing Corp.
Distribution: Scyld Beowulf
Packager: Rick Niles <niles@scyld.com>
Copyright: BSD
Group: System Environment/Libraries 
Source0: ftp://ftp.mcs.anl.gov/pub/mpi/%{name}-%{version}.tar.gz
Buildroot: /var/tmp/%{name}-root
BuildRequires: /usr/bin/f77
BuildRequires: bproc-devel bproc-libs
BuildRequires: pvfs pvfs-devel beomap
Requires: bproc-libs
Requires: pvfs beomap
Obsoletes: beompi
Obsoletes: npr < 0.2.0
Obsoletes: mpprun
Provides: libmpi.so.1
Provides: libmpich.so.1

%define prefix   %{?buildroot:%{buildroot}}%{_prefix}
%define sharedir %{?buildroot:%{buildroot}}%{_datadir}/%{name}
%define libdir   %{?buildroot:%{buildroot}}%{_libdir}/%{name}

%package devel
Summary: Static libraries and header files for MPI.
Group: Development/Libraries 
Obsoletes: beompi-devel
PreReq: %{name} = %{version}

%description devel
Static libraries and header files for MPI.

%description
An implementation of the Message-Passing Interface (MPI) by Argonne
National Laboratory.  This version has been configured to work with
Scyld Beowulf.

%prep
%setup -q -n mpich

# Set the right flags for Scyld Beowulf
sed -e 's/move_pgfile_to_master=no/move_pgfile_to_master=yes/' \
    -e 's/rcpcmd=${RCPCOMMAND-rcp}/rcpcmd=bpcp/' \
    mpid/ch_p4/mpirun.ch_p4.in > tmp.scyld
mv tmp.scyld mpid/ch_p4/mpirun.ch_p4.in
sed -e "s@external_allocate_cmd=\"\"@external_allocate_cmd=\"/usr/bin/beomap | sed 's/:/ /g'\"@" util/mpirun.pg.in > tmp.scyld
mv tmp.scyld util/mpirun.pg.in

%build
./configure --with-arch=LINUX --enable-sharedlib \
            --disable-f90modules \
            -rsh=/usr/bin/bpsh --with-comm=bproc  \
	    --with-romio=--file_system=nfs+ufs+pvfs \
	    --lib="-Wl,--undefined=beowulf_sched_shim,--undefined=get_beowulf_job_map -lminipvfs -lbeomap" \
	    --with-device=ch_p4

%{__make}

# Right now the shared libs build by mpich don't do Fortran, 
#  so re-link here.
# Add in FORTRAN stub so it will be easy to build shared C library
cat > fortran-stub.c <<EOF
void getarg_ (long *n, char *s, short ls) __attribute__ ((weak));
void getarg_ (long *n, char *s, short ls) {}
int f__xargc __attribute__ ((weak)) = -1;
EOF
gcc -O -c fortran-stub.c
ar rs lib/libmpich.a fortran-stub.o
rm -f fortran-stub.[co]

# This relinking is only necessary since the Fortran needs to be made to work.
for i in mpich fmpich pmpich
  do
    ld -shared -soname lib${i}.so.1 --whole-archive lib/lib${i}.a --no-whole-archive -u beowulf_sched_shim -u get_beowulf_job_map -lbeomap -lbproc -lbeostat -lminipvfs -ldl -lc -o lib/shared/lib${i}.so.1.0 
  done

%install
if [ $RPM_BUILD_ROOT != "/" ]; then
  rm -rf $RPM_BUILD_ROOT
fi
make PREFIX=%{prefix} install

# The default share dir is "/usr/mpich/share" should be "/usr/share/mpich"
mv %{prefix}/share %{prefix}/share-old
mkdir -p %{sharedir}
mv %{prefix}/share-old/* %{sharedir} 
mv %{prefix}/examples/* %{sharedir}/examples

# Move the shared libs to the main lib dir
mv %{prefix}/lib/shared/* %{prefix}/lib

# Make sym. link for libmpi.a and libmpi.so.1, create libmpi.so linker script
pushd %{prefix}/lib
ln -s libmpich.a libmpi.a
ln -s libmpich.so.1 libmpi.so.1
cat > libmpi.so <<EOF 
/* GNU ld script 
   This should point to the shared library of the default MPI
   implementation.  This file is only used at compile time.   */
INPUT ( /usr/lib/libmpich.so.1.0 )
EOF
popd

# Remove invalid symlinks
rm -f %{sharedir}/examples/MPI-2-C++/mpirun
rm -f %{sharedir}/examples/mpirun

# Fix the paths in the shell scripts
pushd %{prefix}/bin
for i in *
 do
    sed 's@%{prefix}@%{_prefix}@g' $i > tmpfile
    mv tmpfile $i
    chmod 0755 $i
 done
popd

%post	-p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
if [ $RPM_BUILD_ROOT != "/" ]; then
  rm -rf $RPM_BUILD_ROOT
fi

%files
%defattr(-,root,root)
%doc README COPYRIGHT doc/guide.ps.gz doc/install.ps.gz
/usr/bin/*
#/usr/sbin/*  # doesn't seem to have anything useful for us.
/usr/lib/*.so.*
/usr/man/man1/*

%files devel
%defattr(-,root,root)
/usr/share/*
/usr/include/*
/usr/lib/*.a
/usr/lib/*.so
/usr/man/man3/*
/usr/man/man5/*

%changelog
* Thu Apr 26 2001 Frederick (Rick) A Niles <niles@scyld.com>
- Initial version
