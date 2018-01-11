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
LIBS = $(MPILIB) $(DEVICE_LIB)
LIBSPP = $(MPIPPLIB) $(LIBS)
TESTDIRS = pt2pt coll env context
SUBDIRS = pt2pt coll topol env context

default: subdirs
	@echo "Type \"make testing\" to compile and run tests"

subdirs:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make) ;\
		done

linktest:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making linktest in directory $$dir ;\
			(cd $$dir; make linktest) ;\
		done

testing: $(MPIR_HOME)/util/mpirun
	@-for dir in $(TESTDIRS) ;\
		do \
			echo " " ;\
			echo Making testing in directory $$dir ;\
			(cd $$dir; make testing) ;\
		done

clean:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making clean in directory $$dir ;\
			(cd $$dir; make clean) ;\
		done

$(MPIR_HOME)/util/mpirun:
	(cd $(MPIR_HOME) ; make mpirun )
