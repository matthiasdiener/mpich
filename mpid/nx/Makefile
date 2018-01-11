# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.1.1.2 1995/05/18 11:16:27 jim Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = paragon
COMM        = nx
DEVICE      = nx
BOPT        = 
MPIR_HOME   = /home/joel/mpich
INCLUDE_DIR =  -Impid/nx -Iinclude

CC          = icc
AR          = ar860 crl
RANLIB      = true
OPTFLAGS    = 
ICCLIB      = iCClib

### End User configurable options ###

ALL: default
	
LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   =  -nx -DICCLIB -DFORTRANUNDERSCORE  -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DPOINTER_64_BITS=1 -DINT_LT_POINTER=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 $(OPTFLAGS) -I../.. -I../../include -I./ \
		-DMPID_DEVICE_CODE 
SOURCE   = nxpriv.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = nxpriv.o

default: $(ICCLIB) ../../lib/$(ARCH)/$(COMM)/libmpi.a

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS) $(ICCLIB)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: $(MPI_OBJECTS) $(ICCLIB)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libpmpi.a $?
	@ if test -d iCClib; \
	then cd iCClib ;\
		$(AR) ../../../lib/$(ARCH)/$(COMM)/libpmpi.a *.o ; \
	fi

clean:
	/bin/rm -r -f *~ $(MPI_OBJECTS) $(ICCLIB)

.c.o:
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

$(ICCLIB):
		- mkdir iCClib
		cd iCClib ;\
		ar x /usr/lib/libjmc.a ;\
		ar x /usr/lib/libkmath.a dcopy.o; \
		ar x /usr/lib/libkmath.a scopy.o; \
		$(AR) ../../../lib/$(ARCH)/$(COMM)/libmpi.a *.o
