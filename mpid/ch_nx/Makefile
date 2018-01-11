# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.2 1996/07/06 14:11:32 gropp Exp $
#
#  (C) 1993,1995 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
# Eventually, there will be two sets of routines
#
#
##### User configurable options #####

ARCH        = intelnx
COMM        = nx
DEVICE      = ch_nx
MPIR_HOME   = /Net/antireo/antireo9/MPI/mpich
INCLUDE_DIR =  -Impid/ch_nx -Iinclude
CC          = icc
AR          = ar860 crl
RANLIB      = true
OPTFLAGS    = -O
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

#ALL: default
ALL: libdev

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
LIBDEV   = libmpid.a
CFLAGS   =  -nx -DFORTRANUNDERSCORE  -DMPE_USE_EXTENSIONS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1 -DHAVE_NICE=1 -DMPI_ADI2 -I../../mpid/ch2 $(OPTFLAGS)  \
	   	-I../.. -I../../include -I./ -I../ch2 \
		-I../util \
		-DMPID_DEVICE_CODE   -DMPID_DEBUG_NONE -DMPID_STAT_NONE -Dinteldelta 
# See mpid/ch_adi2/Makefile.in for a full list
SOURCE   = adi2recv.c adi2send.c adi2init.c adi2probe.c adi2ssend.c \
	   adi2hsend.c adi2hrecv.c adi2hssend.c adi2pack.c adi2mpack.c \
	   adi2req.c adi2cancel.c \
	   chbeager.c chneager.c chnrndv.c chdebug.c chchkdev.c chshort.c \
	   chinit.c chnodename.c chtick.c nxpriv.c queue.c cmnargs.c \
	   mprerr.c tr2.c sbcnst2.c 
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = adi2recv.o adi2send.o adi2init.o adi2probe.o adi2ssend.o \
	      adi2hsend.o adi2hrecv.o adi2hssend.o adi2pack.o adi2mpack.o \
	      adi2req.o adi2cancel.o \
	      chbeager.o chneager.o chnrndv.o chdebug.o chchkdev.o chshort.o \
	      chinit.o chnodename.o chtick.o nxpriv.o queue.o cmnargs.o \
	      tr2.o sbcnst2.o \
	      $(ASMFILES_O) 

default: $(LIBNAME)
libdev: $(LIBDEV)

$(SOURCE): ../ch2/mpid.h chdef.h

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: setlinks default_files
default_files: $(MPI_OBJECTS)
	$(AR) $(LIBNAME) $?
profile_all: 

clean:
	/bin/rm -f *~ $(MPI_OBJECTS) libmpid.a mprerr.o

setlinks:
	for file in $(SOURCE) ; do  \
	    if [ ! -s $$file -a -s ../ch2/$$file ] ; then \
	 	ln -s ../ch2/$$file $$file ; fi ; \
	    if [ ! -s $$file -a -s ../util/$$file ] ; then \
		ln -s ../util/$$file $$file ; fi ; \
	done
.c.o:
	if [ ! -s $*.c ] ; then ln -s ../ch2/$*.c $*.c ; fi
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

.o.s:
	$(CC) $(CFLAGS) $(BASEOPT) -c -S $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

libmpid.a: $(MPI_OBJECTS) mprerr.o
	$(AR) $@ $?
	$(RANLIB) $@

lint:
	lint -chvuz -I../.. -I../../include -I../ch2 \
		-I./ \
	-DMPID_DEVICE_CODE $(SOURCE) ../util/queue.c ../util/cmnargs.c

