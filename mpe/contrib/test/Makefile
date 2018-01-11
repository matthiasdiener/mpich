# Generated automatically from Makefile.in by configure.
ALL: default
##### User configurable options #####

ARCH        = rs6000
COMM        = ch_p4
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = $(CC)
CCC         = 
CCLINKER    = $(CCC)
PROF_LIB    = -lpmpi
PROF_DIR    = $(MPIR_HOME)/profiling/lib
PROF_FILE   = log_wrappers.o
F77         = f77
FLINKER     = $(F77)
AR          = ar clr
RANLIB      = ranlib
PROFILING   = $(PROF_DIR)/$(PROF_FILE) $(PROF_LIB)
OPTFLAGS    = -g
MPE_LIBS    = -lmpe -lX11 -lm
MPE_DIR     = $(MPIR_HOME)/mpe
LIB_PATH    = -L/Net/antireo/antireo6/lusk/mpich/lib/rs6000/ch_p4 
LIB_LIST    = $(PROFILING) -lmpi 
INCLUDE_DIR =  -I$(MPIR_HOME)/include -I$(MPIR_HOME)/mpe
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS  = $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)
CCFLAGS = $(CFLAGS)
LIBS = $(LIB_PATH) $(LIB_LIST)
EXECS = testlog

default: $(EXECS)

all: default

testlog: testlog.o /home/lusk/mpich/lib/rs6000/ch_p4/libmpe.a
	$(CLINKER) $(OPTFLAGS) -o testlog testlog.o \
	$(LIBS)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS) *.log

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

