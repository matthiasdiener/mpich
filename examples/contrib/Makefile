# Generated automatically from Makefile.in by configure.
DIRS = usingmpi life C++ nuclei

clean:
	/bin/rm -f $(EXECS) $(OBJS)
	for dir in $(DIRS) ; do \
	if [ -s $$dir ] ; then ( cd $$dir ; make clean ) ; fi ; done

# Here is what we need to run programs in this directory

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
EXECS  = dcmplx isendtst structtest dbltest
OBJS   = dcmplx.o isendtst.o structtest.o

dcmplx: dcmplx.o $(MPILIB)
	$(FLINKER) $(OPTFLAGS) -o dcmplx dcmplx.o $(LIBS)

dbltest: dbltest.o $(MPILIB)
	$(FLINKER) $(OPTFLAGS) -o dbltest dbltest.o $(LIBS)

isendtst: isendtst.o $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o isendtst isendtst.o $(LIBS)

structtest: structtest.o $(MPILIB)
	$(CLINKER) $(OPTFLAGS) -o structtest structtest.o $(LIBS)

