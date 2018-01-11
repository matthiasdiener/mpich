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
MPE_LIBS    = -L/usr/local/lib -lmpe -lX11 -lm
MPE_DIR     = /tmp_mnt/Net/antireo/antireo6/lusk/mpich/mpe
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4  -L/usr/local/lib
FLIB_PATH   = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
MPE_GRAPH   = -DMPE_GRAPHICS
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS	  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) \
	    $(PROF_FLG)
CFLAGSMPE = $(CFLAGS) -I$(MPE_DIR) $(MPE_GRAPH)
CCFLAGS	  = $(CFLAGS)
#FFLAGS	  = '-qdpc=e' 
FFLAGS	  =  $(OPTFLAGS)
MPILIB	  = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB  = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a

LIBS	  = $(LIB_PATH) $(LIB_LIST)
FLIBS	  = $(FLIB_PATH) $(LIB_LIST)
LIBSPP	  = $(MPIPPLIB) $(LIBS)
EXECS	  = systest

default: $(EXECS)

all: default

#include $(MPIR_HOME)/profiling/lib/Makefile.timing
include $(MPIR_HOME)/profiling/lib/Makefile.logging
#include $(MPIR_HOME)/profiling/lib/Makefile.vismess
#include $(MPIR_HOME)/profiling/lib/Makefile.allprof

systest: systest.o $(MPIR_HOME)/include/mpir.h $(PROF_OBJ) $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o systest systest.o \
	$(LIB_PATH) $(PROF_LIB) $(LIB_LIST) -lm

clean:
	/bin/rm -f *.o *~ PI* $(EXECS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

