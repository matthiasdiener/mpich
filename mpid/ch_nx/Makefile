# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = intelnx
COMM        = nx
DEVICE      = ch_nx
BOPT        = 
MPIR_HOME   = /Net/alriga/alriga9/MPI/mpich
INCLUDE_DIR =  -Impid/ch_nx -Iinclude
CC          = icc
AR          = ar860 crl
RANLIB      = ranlib
OPTFLAGS    = -O

### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -nx   -DHAVE_STDLIB_H=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPID_DEVICE_CODE  -DMPID_DEBUG_NONE -DMPID_STAT_NONE -Dinteldelta
SOURCE   = nxevent.c nxrecv.c nxsend.c nxinit.c nxprobe.c nxsync.c nxrndv.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = nxevent.o nxrecv.o nxsend.o nxinit.o nxprobe.o nxsync.o nxrndv.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: 

clean:
	/bin/rm -f *~ $(MPI_OBJECTS) 

.c.o:
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@



