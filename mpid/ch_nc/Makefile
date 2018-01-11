# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = sun4
COMM        = 
DEVICE      = ch_tcp
BOPT        = 
MPIR_HOME   = /home/gropp/mpich
INCLUDE_DIR =  -Impid/ch_tcp -Iinclude
CC          = cc
AR          = ar clr
RANLIB      = ranlib
OPTFLAGS    = -g

### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =   -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPID_DEVICE_CODE 
SOURCE   = n3event.c n3recv.c n3send.c n3init.c n3probe.c n3sync.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = n3event.o n3recv.o n3send.o n3init.o n3probe.o n3sync.o

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



