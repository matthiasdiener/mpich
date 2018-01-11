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
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
PETSC_AVAIL = @PETSC_AVAIL@
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include -I$(MPE_DIR)
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(PETSC_AVAIL)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS = $(INCLUDE_DIR) $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = -lmpi++
LIBS = $(LIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)
EXECS = cpi++ life++
default: $(EXECS)

all: default

cpi++: cpi++.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CCLINKER) $(OPTFLAGS) -o cpi++ cpi++.o ${MPE_LIBS} $(LIBSPP)

life++: life++.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CCLINKER) $(OPTFLAGS) -o life++ life++.o $(LIBSPP)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS)

.C.o:
	$(CCC) $(CCFLAGS) -c $*.C

