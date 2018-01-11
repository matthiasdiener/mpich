# Generated automatically from Makefile.in by configure.
#
#  $Id$
#
#  (C) 1993,1995 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
# Eventually, there will be two sets of routines
#
#
##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
DEVICE      = ch_p4
MPIR_HOME   = /Net/alriga/alriga9/MPI/mpich
INCLUDE_DIR =  -Impid/ch_p4 -Iinclude
CC          = cc
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = -g
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -DFORTRANUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -DHAS_XDR=1  -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPI_$(ARCH) -DMPID_DEVICE_CODE  -DHAVE_GETDOMAINNAME=1 -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_PKT_VAR_SIZE \
	        
# See mpid/ch_adi2/Makefile.in for a full list
SOURCE   = adi2recv.c adi2send.c adi2init.c \
	   chbeager.c chbrndv.c chdebug.c chrecv.c chshort.c \
	   
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = adi2recv.o adi2send.o adi2init.o \
	   chbeager.o chbrndv.o chdebug.o chrecv.o chshort.o \
	      $(ASMFILES_O) 

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a
local: libmpid.a

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

.o.s:
	$(CC) $(CFLAGS) $(BASEOPT) -c -S $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

libmpid.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

