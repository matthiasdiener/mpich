# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH	      = sun4
P4_ARCH	      = SUN
COMM	      = ch_p4
DEVICE	      = ch_p4
BOPT	      = 
MPIR_HOME     = /home/gropp/mpich
INCLUDE_DIR   = -Ip4-1.4/include  -Impid/ch_p4 -Iinclude
CC	      = cc
AR	      = ar clr
RANLIB	      = ranlib
OPTFLAGS      = -g
MAKE          = make

### End User configurable options ###

SHELL = /bin/sh

ALL: default

P4_HOME  = ./p4-1.4
LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =   -DMPE_USE_EXTENSIONS=1 -DHAS_XDR=1 -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
	   -DMPID_DEVICE_CODE  -DHAVE_GETDOMAINNAME=1 -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_PKT_VAR_SIZE
SOURCE   = p4event.c p4recv.c p4send.c p4init.c p4probe.c p4sync.c p4coll.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = p4event.o p4recv.o p4send.o p4init.o p4probe.o p4sync.o p4coll.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: p4inmpi default_all_files
default_all_files: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: 

#
# We make sure to remove the old defs.MD to ensure that p4 gets the correct
# definitions for the architecture.  We also need to remove the library,
# in case there is one.
clean:
	/bin/rm -f *~ $(MPI_OBJECTS)
	(cd $(P4_HOME); if [ -s Makefile ] ; then $(MAKE) clean; \
	/bin/rm -f lib/libp4.a ; fi ;)
#	/bin/rm -f Makefile lib/p4_config.h lib/libp4.a ; fi ;)
.c.o:
	$(CC) $(CFLAGS) $(OPTFLAGS) $(INCLUDE_DIR) -c $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: p4inmpi $(MPI_OBJECTS) 
	$(AR) $@ $(MPI_OBJECTS)
	$(RANLIB) $@

chlib: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $(MPI_OBJECTS)
	$(RANLIB) ../../lib/$(ARCH)/$(COMM)/libmpi.a

p4inmpi:
	(cd $(P4_HOME); \
	$(MAKE) p4inmpi MPIARCH=$(ARCH); cd ..)




