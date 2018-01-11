# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = IRIX
COMM        = ch_shmem
DEVICE      = ch_shmem
BOPT        = 
MPIR_HOME   = /home/gropp/mpich
INCLUDE_DIR =  -Impid/ch_shmem -Iinclude
CC          = cc
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = -O
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -woff 728 -DFORTRANUNDERSCORE  -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPI_$(ARCH) -DMPID_DEVICE_CODE  -DHAVE_GETDOMAINNAME=1 -DHAVE_GETHOSTNAME=1 -DHAVE_SYSINFO=1 -DHAVE_UNAME=1  \
	         -DHAVE_MMAP=1
SOURCE   = shmemevent.c shmemrecv.c shmemsend.c shmeminit.c shmemprobe.c \
	   shmemsync.c p2p.c shmempriv.c shops2.c shmemcoll.c shmemget.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = shmemevent.o shmemrecv.o shmemsend.o shmeminit.o shmemprobe.o \
	      shmemsync.o p2p.o shmempriv.o shops2.o shmemcoll.o shmemget.o \
              $(ASMFILES_O)

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

shops2: shops2.c
	cc -o shops2 $(CFLAGS) -DBUILD_MAIN shops2.c p2p.o

