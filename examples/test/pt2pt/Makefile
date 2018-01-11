# Generated automatically from Makefile.in by configure.
ALL: default
##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = cc
CCC         = 
CCLINKER    = $(CCC)
F77         = f77
FLINKER     = f77
AR          = ar clr
RANLIB      = ranlib
PROFILING   = $(PMPILIB)
OPTFLAGS    = -g
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
FLIB_PATH   = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4

### End User configurable options ###

SHELL = /bin/sh

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS =  $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
LIBS = $(LIB_PATH) $(LIB_LIST)
FLIBS = $(FLIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)
EXECS = overtake sendrecv isndrcv third fourth fifth sixth hvectest \
	hvectest2 probe probe1 testtest1 testtypes ssendtest typetest\
	secondf dtypelife issendtest

default: $(EXECS)

testing: default $(MPIR_HOME)/util/mpirun
	  -runtests

linktest: overtake secondf clean

all: testing

fortran_tests: secondf allpair

overtake: overtake.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o overtake overtake.o test.o $(LIBS)

sendrecv: sendrecv.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o sendrecv sendrecv.o test.o $(LIBS)

isndrcv: isndrcv.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o isndrcv isndrcv.o test.o $(LIBS)

typetest: typetest.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o typetest typetest.o test.o $(LIBS)

ssendtest: ssendtest.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o ssendtest ssendtest.o test.o $(LIBS)

issendtest: issendtest.o test.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o issendtest issendtest.o test.o $(LIBS)

testtypes: testtypes.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o testtypes testtypes.o $(LIBS)

dtypelife: dtypelife.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o dtypelife dtypelife.o $(LIBS)

third: third.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o third $(DEVICE_MAIN) third.o $(LIBS)
fourth: fourth.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o fourth $(DEVICE_MAIN) fourth.o $(LIBS)
fifth: fifth.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o fifth $(DEVICE_MAIN) fifth.o $(LIBS)
sixth: sixth.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o sixth $(DEVICE_MAIN) sixth.o $(LIBS)

probe: probe.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o probe $(DEVICE_MAIN) probe.o $(LIBS)

probe1: probe1.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o probe1 $(DEVICE_MAIN) probe1.o $(LIBS)

testtest1: testtest1.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o testtest1 $(DEVICE_MAIN) testtest1.o $(LIBS)

hvectest: hvectest.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o hvectest $(DEVICE_MAIN) hvectest.o $(LIBS)

hvectest2: hvectest2.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o hvectest2 $(DEVICE_MAIN) hvectest2.o $(LIBS)

secondf: secondf.o mpif.h
	$(FLINKER) $(OPTFLAGS) -o secondf secondf.o $(FLIBS)
allpair: allpair.o mpif.h
	$(FLINKER) $(OPTFLAGS) -o allpair allpair.o $(FLIBS)

mpif.h: $(MPIR_HOME)/include/mpif.h
	if test \! -f mpif.h ; then \
	    ln -s $(MPIR_HOME)/include/mpif.h . \
	fi

#
# Make sure that we remove executables for specific architectures
clean:
	/bin/rm -f *.o *~ PI* $(EXECS) pt2pt.out pt2pt.out.orig secondf \
		allpair sendrecv-[012].out isndrcv-[012].out typetest-?.out \
		overtake-?.out core
	for file in $(EXECS) Makefile ; do \
	   /bin/rm -f $$file.sun4 $$file.alpha $$file.IRIX $$file.freebsd ;\
	done

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

$(MPIR_HOME)/util/mpirun:
	(cd $(MPIR_HOME) ; make mpirun )
