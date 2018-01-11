# Generated automatically from Makefile.in by configure.
#
#  $Id$
#
#  (C) 1993,1995 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4-2
DEVICE      = ch_p4-2
MPIR_HOME   = /home/gropp/mpich
INCLUDE_DIR =  -I../ch_p4-2 -I../../include
CC          = cc
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = -g
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -DFORTRANUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -DHAS_XDR=1  -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 $(OPTFLAGS) -I../.. -I../$(COMM) -I../ch2 -I../../include -I./ 
SOURCE   = queue.c mperror.c cmnargs.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = queue.o mperror.o cmnargs.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a
local: ../$(COMM)/libmpid.a

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

../$(COMM)/libmpid.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

