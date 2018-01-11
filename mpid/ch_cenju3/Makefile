# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.1 1996/04/12 20:18:44 gropp Exp $
#
#  (C) 1993,1995 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
# Eventually, there will be two sets of routines
#
#
##### User configurable options #####

ARCH        = R4000
COMM        = ch_cenju3
DEVICE      = ch_cenju3
MPIR_HOME   = /home/gmap21/mpich
INCLUDE_DIR =  -Impid/ch_cenju3 -Iinclude
CC          = /usr/necccs/bin/cc
ASM         = /usr/necccs/bin/as
AR          = ar crl
RANLIB      = :
OPTFLAGS    = -O -Kmips2
SHELL       = /bin/sh
ASMFILES_O  = get_stack.o
### End User configurable options ###

#ALL: default
ALL: setlinks libdev

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpich.a
LIBDEV   = libmpid.a
CFLAGS   =  -DFORTRANUNDERSCORE  -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DMPI_ADI2 -I../../mpid/ch2 $(OPTFLAGS)  \
	   	-I../.. -I../../include -I./ -I../ch2 \
		-I../util \
		-DMPID_DEVICE_CODE  -DHAVE_GETHOSTNAME=1 -DHAVE_SYSINFO=1 -DHAVE_UNAME=1  -DMPID_DEBUG_NONE -DMPID_STAT_NONE 
# See mpid/ch_adi2/Makefile.in for a full list
SOURCE   = adi2recv.c adi2send.c adi2init.c adi2probe.c adi2ssend.c \
           adi2hsend.c adi2hrecv.c adi2hssend.c adi2pack.c adi2mpack.c \
           adi2req.c \
           chinit.c shmemdebug.c shmemchkdev.c \
           shmemshort.c shmemneager.c shmemnrndv.c cenju3priv.c \
           chnodename.c chtick.c queue.c cmnargs.c \
           tr2.c sbcnst2.c 
SOURCEC  = $(SOURCE)
SOURCEF  =
# chbeager.o chbrndv.o
MPI_OBJECTS = adi2recv.o adi2send.o adi2init.o adi2probe.o adi2ssend.o \
              adi2hsend.o adi2hrecv.o adi2hssend.o adi2pack.o adi2mpack.o \
              adi2req.o \
              chinit.o shmemdebug.o shmemchkdev.o \
              shmemshort.o shmemneager.o shmemnrndv.o cenju3priv.o \
              chnodename.o chtick.o queue.o cmnargs.o \
              tr2.o sbcnst2.o $(ASMFILES_O) 

default: setlinks $(LIBNAME)
libdev: setlinks $(LIBDEV)

$(SOURCE): ../ch2/mpid.h shdef.h

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
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
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

.s.o:
	$(ASM) -c $*.s

../../lib/$(ARCH)/$(COMM)/libmpich.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

libmpid.a: $(MPI_OBJECTS) mprerr.o
	$(AR) $@ $?
	$(RANLIB) $@

lint:
	lint -chvuz -I../.. -I../../include -I../ch2 \
		-I./ \
	-DMPID_DEVICE_CODE $(SOURCE) ../util/queue.c ../util/cmnargs.c

