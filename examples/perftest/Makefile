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
FFLAGS =  $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPELIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
FLIBS = $(FLIB_PATH) $(LIB_LIST)
LIBS = $(LIB_PATH) $(LIB_LIST) -lm
LIBSPP = $(MPIPPLIB) $(LIBS)
EXECS = mpptest goptest

default: $(EXECS)

OBJS = gopf.o grph.o ovlap.o pair.o pattern.o util.o getopts.o \
	tstauto.o rate.o

mpptest: mpptest.o $(OBJS) $(MPILIB) $(MPELIB)
	$(CLINKER) $(OPTFLAGS) -o mpptest mpptest.o $(OBJS) $(MPELIB) $(LIBS)

goptest: goptest.o $(OBJS) $(MPILIB) $(MPELIB)
	$(CLINKER) $(OPTFLAGS) -o goptest goptest.o $(OBJS) $(MPELIB) $(LIBS)

tunepkt: tunepkt.o $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o tunepkt tunepkt.o $(LIBS)

clean:
	/bin/rm -f *.o *~ $(EXECS) tunepkt core TAGS

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

