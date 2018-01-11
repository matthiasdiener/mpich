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
FLIB_PATH   = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
MPE_GRAPH   = -DMPE_GRAPHICS
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) 
CFLAGSMPE  = $(CFLAGS) -I$(MPE_DIR) $(MPE_GRAPH)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS =  $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
LIBS = $(LIB_PATH) $(LIB_LIST)
FLIBS = $(FLIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)
EXECS = pi pi2 pi3 cpilog cpi matmult monte
default: $(EXECS)

all: default

cpi: cpi.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o cpi cpi.o \
	$(LIB_PATH) ${MPE_LIBS} $(LIB_LIST) -lm

cpilog: cpilog.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o cpilog cpilog.o \
	$(LIB_PATH) ${MPE_LIBS} $(LIB_LIST) -lm

monte: monte.o $(MPIR_HOME)/include/mpif.h
	$(CLINKER) $(OPTFLAGS) -o monte monte.o \
	$(LIB_PATH) ${MPE_LIBS} $(LIB_LIST) -lm

pi: pi.o $(MPIR_HOME)/include/mpif.h
	$(FLINKER) $(OPTFLAGS) -o pi pi.o $(FLIBS)

pi2: pi2.o $(MPIR_HOME)/include/mpif.h
	$(FLINKER) $(OPTFLAGS) -o pi2 pi2.o $(FLIBS)

pi3: pi3.o $(MPIR_HOME)/include/mpif.h
	$(FLINKER) $(OPTFLAGS) -o pi3 pi3.o $(FLIBS)

matmult: matmult.o $(MPIR_HOME)/include/mpif.h
	$(FLINKER) $(OPTFLAGS) -o matmult matmult.o $(FLIBS)

clean:
	/bin/rm -f *.o *~ PI* $(EXECS)

cpilog.o: cpilog.c
	$(CC) $(CFLAGSMPE) -c cpilog.c
monte.o: monte.c
	$(CC) $(CFLAGSMPE) -c monte.c
.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

