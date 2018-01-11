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
CLINKER     = $(CC)
CCC         = 
CCLINKER    = $(CCC)
F77         = f77
FLINKER     = $(F77)
AR          = ar clr
RANLIB      = ranlib
PROFILING   = $(PMPILIB)
OPTFLAGS    = -g
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
MPE_LIBS    = -L/usr/local/lib -lmpe -lX11 -lm
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS =  $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
LIBS = $(LIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)
EXECS = life

default: $(EXECS)

testing: default
	runtests

all: testing

life: life.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o life life.o $(MPE_LIBS) $(LIBS)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

