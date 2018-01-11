# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1994/04/01 23:34:00 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = rs6000
COMM        = ch_eui
DEVICE      = ch_eui
BOPT        = 
MPIR_HOME   = /home/MPI/mpich
INCLUDE_DIR =  -Impid/ch_eui -Iinclude
CC          = mpCC
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = -O

### End User configurable options ###

ALL: default

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -DFORTRANNOUNDERSCORE  -D_ALL_SOURCE=1  -DHAVE_STDLIB_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPID_DEVICE_CODE -DIBMMPL  -DMPID_LIMITED_BUFFERS -DMPID_DEBUG_NONE -DMPID_STAT_NONE
SOURCE   = euievent.c euirecv.c euisend.c euiinit.c euiprobe.c euisync.c \
	   euirndv.c euicoll.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = euievent.o euirecv.o euisend.o euiinit.o euiprobe.o euisync.o \
	      euirndv.o euicoll.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
#
# gclock/read_clock may give access to the global clock; we are allowed
# only to distribute objects for these.
#
default_all: $(MPI_OBJECTS) gclock.o read_clock.o
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: 

gclock.o: gclock.sav
	cp gclock.sav gclock.o
read_clock.o: read_clock.sav
	cp read_clock.sav read_clock.o

clean:
	/bin/rm -f *~ $(MPI_OBJECTS) 

.c.o:
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@



