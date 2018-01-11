Summary: Argonne National Laboratory MPI implementation
Name: mpich
Version: 1.2.4
Vendor: Argonne National Laboratory
Packager: William Gropp <gropp@mcs.anl.gov>
Copyright: BSD
Group: System Environment/Libraries 
#
# This spec file was inspired by spec files created by others for MPICH,
# including the Scyld version, written by Frederick (Rick) Niles of Scyld, and 
# included as mpich-scyld.spec in the MPICH source distribution.
#
# Construct the source RPM with
#  mkdir ...
#  rpm mpich.spec ....
# This spec file requires a suitably recent version of RPM. Is there an
# RPM version tag that we should set?

# The first part of this spec file parameterizes the choice of compiler
# It is possible to configure and build MPICH for multiple compilers without 
# requiring a new build for each combination of C, Fortran, and C++ compilers.
# This is covered in the installation and users manual; this spec file 
# only handles the case of a single choice of compiler
# Choose only one compiler
%define gnu    1
%define absoft 0
%define intel  0
%define pgi    0
%define lahey  0
%define nag    0

#
# Pick the device.  The ch_p4mpd device is recommended for clusters of
# uniprocessors; the ch_p4 device with comm=shared is appropriate for
# clusters of SMPs.  
%define device ch_p4mpd
%define other_device_opts %{nil}
# for ch_p4:
# define device ch_p4
# define other_device_opts -comm=shared
# You may also want to set
# define rshcommand /usr/bin/ssh
# define rcpcommand /usr/bin/rcp
# for ch_p4 with bproc (see mpich-scyld.spec instead, but the following is
# a start:)
# define device ch_p4
# define other_device_opts -comm=bproc
# for ch_shmem:
# define device ch_shmem
# define other_device_opts %{nil}

#
# Define this to +pvfs if pvfs is available.
%define other_file_systems %{nil}

%define rel 1

#
# These definitions are approximate.  They are taken in part from the
# mpich-scyld.spec file, though the choices here rely more on the MPICH
# configure to find the correct options for the compilers.  Not all have
# been tested (we don't have access to the Lahey compiler, for example).
%if %{gnu}
    %define c_compiler gcc
    %define compiler_path /usr/bin
    %define setenvs export FC=%{compiler_path}/g77
    %define fopts --disable-f90modules 
    %define extralibs %{nil}
    %define extraldopts %{nil}
    %define release %{rel}
%endif
%if %{absoft}
    %define c_compiler gcc
    %define compiler_path /usr/absoft
    %define setenvs export ABSOFT=%{compiler_path}; export FC=%{compiler_path}/bin/f77; export F90=%{compiler_path}/bin/f90
    %define fopts --without-mpe
    %define extralibs -L%{compiler_path}/lib -lU77 -lf77math
    %define extraldopts %{extralibs}
    %define release %{rel}.absoft
%endif
%if %{intel}
    %define c_compiler icc
    %define compiler_path /opt/intel/compiler50/ia32
    %define setenvs export PATH=$PATH:%{compiler_path}/bin; export LD_LIBRARY_PATH=%{compiler_path}/lib; export FC=%{compiler_path}/bin/ifc; export F90=$FC
    %define fopts -fflags="-Vaxlib "
    %define extralibs %{nil}
    %define extraldopts -L%{compiler_path}/lib -lPEPCF90 -lIEPCF90 -lF90 %{compiler_path}/lib/libintrins.a
    %define release %{rel}.intel
%endif
%if %{pgi}
    %define c_compiler pgicc
    %define compiler_path /usr/pgi/linux86
    %define setenvs export PATH=$PATH:%{compiler_path}/bin; export PGI=%{compiler_path}/..; export CC="%{compiler_path}/bin/pgcc"; export FC="/usr/pgi/linux86/bin/pgf77"; export F90="%{compiler_path}/bin/pgf90"
    %define fopts %{nil}
    %define extralibs -L%{compiler_path}/lib -lpgc
    %define extraldopts -L%{compiler_path}/lib -lpgc
    %define release %{rel}.pgi
%endif
%if %{lahey}
    %define c_compiler gcc
    %define compiler_path /usr/local/lf9561
    %define setenvs export PATH=%{compiler_path}/bin:$PATH; export LD_LIBRARY_PATH=%{compiler_path}/lib:$LD_LIBRARY_PATH; export FC=%{compiler_path}/bin/lf95; export F90=%{compiler_path}/bin/lf95
    %define fopts %{nil}
    %define extralibs %{nil}
    %define extraldopts %{nil}
    %define release %{rel}.lahey
%endif
%if %{nag}
    %define c_compiler gcc
    %define compiler_path /usr/local
    %define setenvs export FC=%{compiler_path}/bin/f95; export F90=%{compiler_path}/bin/f95
    %define fopts --with-f95nag
    %define extralibs %{nil}
    %define extraldopts %{nil}
    %define release %{rel}.nag
%endif

Release: %{release}
Source0: ftp://ftp.mcs.anl.gov/pub/mpi/%{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-root
#BuildRequires: %{c_compiler}
Provides: libmpich.so.1

%define prefixdir   %{?buildroot:%{buildroot}}%{_prefix}
%define sharedir %{?buildroot:%{buildroot}}%{_datadir}/%{name}
%define libdir   %{?buildroot:%{buildroot}}%{_libdir}/%{name}
%define mandir   %{?buildroot:%{buildroot}}%{_mandir}

%package devel
Summary: Static libraries and header files for MPI.
Group: Development/Libraries 
PreReq: %{name} = %{version}

%description devel
Static libraries and header files for MPI.

%description
An implementation of the Message-Passing Interface (MPI) by Argonne
National Laboratory. 

%prep
%setup -q -n mpich-%{version}

#
# The MPICH configure is based on Autoconf version 1, and cannot be rebuilt
# with %%configure.
# These options do the following:
#   Pass the various installation directories into configure
#   Select the MPICH "device"
#   Build the shared libraries, and install them into the same directory
#   as the regular (.a) libraries
#   Build MPI-IO (ROMIO) with the specified file systems, including
#   PVFS if specified by setting other_file_systems to +pvfs.
%build
%setenvs
./configure --prefix=%{prefixdir} \
        --exec-prefix=%{_exec_prefix} \
        --bindir=%{_bindir} \
	--sbindir=%{_sbindir} \
        --datadir=%{_datadir}/mpich/ \
        --includedir=%{_includedir} \
	--libdir=%{libdir} \
        --sharedstatedir=%{_sharedstatedir} \
        --mandir=%{mandir} \
        --with-device=%{device} --enable-sharedlib=%{libdir} \
        %{other_device_opts} \
        --with-romio=--file_system=nfs+ufs%{other_file_systems}
        
%{__make}

%install
%setenvs
if [ $RPM_BUILD_ROOT != "/" ]; then
  rm -rf $RPM_BUILD_ROOT
fi
make install

# Examples are installed in the wrong location 
mkdir -p %{sharedir}/examples
mv %{prefixdir}/examples/* %{sharedir}/examples

# Remove invalid symlinks
rm -f %{sharedir}/examples/MPI-2-C++/mpirun
rm -f %{sharedir}/examples/mpirun

# Fix the paths in the shell scripts
for i in `find %{sharedir} %{prefixdir}/bin -type f`
 do
    sed 's@%{?buildroot:%{buildroot}}@@g' $i > tmpfile
    sed 's@%{_builddir}/%{name}-%{version}@/usr@g' tmpfile > $i
    if [ -x $i ]; then
        chmod 0755 $i
    else
        chmod 0644 $i
    fi
 done

%post	-p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
if [ $RPM_BUILD_ROOT != "/" ]; then
  rm -rf $RPM_BUILD_ROOT
fi

%files
%defattr(-,root,root)
%doc README COPYRIGHT doc/mpichman-chp4.ps.gz doc/mpichman-chp4mpd.ps.gz
%doc doc/mpichman-chshmem.ps.gz doc/mpeman.ps.gz
/usr/bin/*
#/usr/sbin/*  
/usr/lib/%{name}/*.so.*
%{_mandir}/man1/*

%files devel
%defattr(-,root,root)
/usr/share/*
/usr/include/*
/usr/lib/%{name}/*.a
/usr/lib/%{name}/*.so
%{_mandir}/man3/*
%{_mandir}/man4/*

%changelog
* Thu Apr 25 2002 William (Bill) Gropp <gropp@mcs.anl.gov>
- Initial version
