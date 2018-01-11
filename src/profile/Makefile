# Generated automatically from Makefile.in by configure.
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#

##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
F77         = f77
AR          = ar clr
RANLIB      = ranlib
INCLUDE_DIR =  -I../../mpid/ch_p4 -I../../include
DEVICE      = ch_p4
OPTFLAGS    = -g

### End User configurable options ###

SHELL = /bin/sh

DEVICE_INCLUDE = ../../mpid/$(DEVICE)/dmch.h

MPI_INCLUDE    = ../../include/mpi.h ../../include/mpi_bc.h \
                 ../../include/mpir.h ../../include/mpi_ad.h \
		 ../../include/mpisys.h 
DEVICE_LIB = 
CFLAGS =   -DHAVE_STDLIB_H=1 $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(MPIPROFILE)
FFLAGS = $(OPTFLAGS)
LIBS = ../../lib/$(ARCH)/$(COMM)/libmpi.a $(DEVICE_LIB)
MPI_OBJECTS = pcontrol.o
MPI_SOURCES = pcontrol.c
MPI_CSOURCES = pcontrol.c
MPI_WSOURCES = pcontrolf.c

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

profile: ../../lib/$(ARCH)/$(COMM)/libpmpi.a

$(MPI_OBJECTS) :  $(MPI_INCLUDE)

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libpmpi.a $?
	
wrappers:
	-/bin/rm -f $(MPI_WSOURCES)
	../../util/bfort -ferr -mpi -mnative -mapptr -ptrprefix MPIR_ \
		-anyname -I pubinc $(MPI_CSOURCES)	
manpages:
	../../util/doctext -mpath ../../man/man3 -ext 3 -I pubinc \
	   -index ../../www.index -jumpfile ../../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPI \
		$(MPI_CSOURCES)
	../../util/doc2lt $(MPI_CSOURCES) >> ../../rsum.tmp

wwwpages:
	../../util/doctext -html -mpath ../../www/www3 -ext 3 -I pubinc \
	   -index ../../www.index -jumpfile ../../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPI \
		$(MPI_CSOURCES)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
.f.o:
	$(F77) $(FFLAGS) -c $*.f

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

../../lib/$(ARCH)/$(COMM)/libpmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

clean:
	/bin/rm -f *.o *~ 
