# Generated automatically from Makefile.in by configure.
#
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
CCC         = 
CFLAGS	    =   -DHAVE_STDLIB_H=1
OPTFLAGS    = -g
AR          = ar clr
RANLIB      = ranlib
INCLUDE_DIR =  -I../../mpid/ch_p4 -I../../include

### End User configurable options ###

MPI_INCLUDE    = ../../include/mpi.h     ../../include/mpi_bc.h \
                 ../../include/mpir.h    ../../include/mpi_ad.h \
		 ../../include/mpisys.h  ../../include/mpi++.h \
		 ../../include/binding.h ../../include/mpi++P.h

CCFLAGS = $(CFLAGS) $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: global.o
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi++.a $?
profile_all: 

default: ../../lib/$(ARCH)/$(COMM)/libmpi++.a
profile:

global.o: global.C $(MPI_INCLUDE)
	$(CCC) $(CCFLAGS) -c global.C

../../lib/$(ARCH)/$(COMM)/libmpi++.a: global.o
	$(AR) $@ $?
	$(RANLIB) $@

clean:
	/bin/rm -f *.o *~
