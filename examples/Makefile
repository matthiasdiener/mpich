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
MPE_GRAPH   = -DMPE_GRAPHICS
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4

### End User configurable options ###

CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(MPE_GRAPH)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS = $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
LIBS = $(LIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)
# Were not ready to do contrib by default yet.
SUBDIRS = test perftest
TESTDIRS = test
EXECS =

default: subdirs

subdirs:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make) ;\
		done

testing:
	@for dir in $(TESTDIRS) ;\
		do \
			echo " " ;\
			echo Making tests in directory $$dir ;\
			(cd $$dir; make testing) ;\
		done

linktest:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making linktest in directory $$dir ;\
			(cd $$dir; make linktest) ;\
		done

all: subdirs testing

clean:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making clean in directory $$dir ;\
			(cd $$dir; make clean) ;\
		done
