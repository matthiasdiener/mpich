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
EXECS = coll1 coll2 coll3 coll4 coll5 coll6 coll7 coll8 coll9 \
	coll10 coll11 coll12 bcastbug bcastbug2 grouptest redtst \
	barrier bcast

default: $(EXECS)

testing: default $(MPIR_HOME)/util/mpirun
	runtests

linktest: default clean

all: testing

coll1: coll1.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll1 $(DEVICE_MAIN) coll1.o $(LIBS)
coll2: coll2.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll2 $(DEVICE_MAIN) coll2.o $(LIBS)
coll3: coll3.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll3 $(DEVICE_MAIN) coll3.o $(LIBS)
coll4: coll4.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll4 $(DEVICE_MAIN) coll4.o $(LIBS)
coll5: coll5.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll5 $(DEVICE_MAIN) coll5.o $(LIBS)
coll6: coll6.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll6 $(DEVICE_MAIN) coll6.o $(LIBS)
coll7: coll7.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll7 $(DEVICE_MAIN) coll7.o $(LIBS)
coll8: coll8.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll8 $(DEVICE_MAIN) coll8.o $(LIBS)
coll9: coll9.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll9 $(DEVICE_MAIN) coll9.o $(LIBS)
coll10: coll10.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll10 $(DEVICE_MAIN) coll10.o $(LIBS)
coll11: coll11.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll11 $(DEVICE_MAIN) coll11.o $(LIBS)
coll12: coll12.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o coll12 $(DEVICE_MAIN) coll12.o $(LIBS)
bcastbug2: bcastbug2.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o bcastbug2 $(DEVICE_MAIN) bcastbug2.o $(LIBS)
bcastbug: bcastbug.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o bcastbug $(DEVICE_MAIN) bcastbug.o $(LIBS)
grouptest: grouptest.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o grouptest $(DEVICE_MAIN) grouptest.o $(LIBS)
redtst: redtst.o $(MPIR_HOME)/include/mpir.h $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o redtst $(DEVICE_MAIN) redtst.o $(LIBS)
barrier: barrier.o test.o
	$(CLINKER) $(OPTFLAGS) -o barrier barrier.o test.o $(LIBS)
bcast: bcast.o test.o
	$(CLINKER) $(OPTFLAGS) -o bcast bcast.o test.o $(LIBS)
clean:
	/bin/rm -f *.o *~ PI* $(EXECS) barrier-?.out bcast-?.out core

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

$(MPIR_HOME)/util/mpirun:
	(cd $(MPIR_HOME) ; make mpirun )
