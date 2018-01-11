# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = sun4
COMM        = ch_nexus
DEVICE      = ch_nexus
BOPT        = 
MPIR_HOME   = /Net/antireo/antireo9/MPI/mpich
INCLUDE_DIR =  -I/usr/local/nexus/include  -Impid/ch_nexus -Iinclude 
CC          = gcc
AR          = ar crl
RANLIB      = ranlib
# Need a way to figure if Nexus Lite or not
OPTFLAGS    = -g -DNEXUS_LITE
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -munaligned-doubles -DFORTRANUNDERSCORE  -DMPE_USE_EXTENSIONS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		$(INCLUDE_DIR) \
		-DMPI_$(ARCH) -DMPID_DEVICE_CODE  -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_DEBUG_NONE -DMPID_STAT_NONE -DHAVE_GETTIMEOFDAY  \
	        
SOURCE   = nexusevent.c nexusrecv.c nexussend.c nexusinit.c nexusprobe.c \
	   nexussync.c  nexuspriv.c  nexuscoll.c nexusdebug.c nexushetero.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = nexusevent.o nexusrecv.o nexussend.o nexusinit.o nexusprobe.o \
	      nexussync.o  nexuspriv.o  nexuscoll.o nexusdebug.o nexushetero.o \
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


	cc -o (CFLAGS) -DBUILD_MAIN  

