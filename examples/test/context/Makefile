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
EXECS = attrtest attrt

default: $(EXECS)

fortran_tests: attrtest

testing: default
	runtests 

linktest: default clean

all: testing

attrtest: attrtest.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(FLINKER) $(OPTFLAGS) -o attrtest $(DEVICE_MAIN) attrtest.o $(FLIBS)

attrt: attrt.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o attrt $(DEVICE_MAIN) attrt.o $(LIBS)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS) core

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f
