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
#PROFILING   = $(PROF_DIR)/$(PROF_FILE) $(PROF_LIB)
OPTFLAGS    = -g
MPE_LIBS    = -L/usr/local/lib -lmpe -lX11 -lm
MPE_DIR     = /tmp_mnt/Net/antireo/antireo6/lusk/mpich/mpe
PROF_LIB    = -lpmpi
PROF_DIR    = $(MPIR_HOME)/profiling/lib
PROF_FILE   = time_wrappers.o
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4  -L/usr/local/lib
# On the meiko, you need to add -lsocket to LIB_LIST
LIB_LIST    = $(PROFILING) -lmpi 
INCLUDE_DIR =  -I$(MPIR_HOME)/include -I/tmp_mnt/Net/antireo/antireo6/lusk/mpich/mpe 
DEVICE      = ch_p4
MPE_GRAPH   = -DMPE_GRAPHICS

### End User configurable options ###

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(MPE_GRAPH)
CCFLAGS = $(CFLAGS)
LIBS = $(LIB_PATH) $(LIB_LIST)
EXECS = pmandel

default: $(EXECS)

all: default

fract_gen.o: fract_gen.h pmandel.h

pm_genproc.o: pmandel.h fract_gen.h

pmandel.o: pmandel.h fract_gen.h lists.h pm_genproc.h

pm_separate_rect.o: pmandel.h pm_genproc.h

pmandel: pmandel.o pm_genproc.o fract_gen.o pm_separate_rect.o lists.o args.o
	$(CLINKER) $(OPTFLAGS) -o pmandel pmandel.o pm_genproc.o \
	  fract_gen.o pm_separate_rect.o lists.o args.o \
	  $(LIB_PATH) $(MPE_LIBS) $(LIB_LIST)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS) *.log *.out

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f
