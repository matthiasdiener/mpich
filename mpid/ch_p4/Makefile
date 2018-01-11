# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.4 1996/07/17 18:07:09 gropp Exp $
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
MPIR_HOME   = /home/gropp/mpich
INCLUDE_DIR =  -Impid/ch_p4 -Iinclude
CC          = cc
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = -g
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

#ALL: default
ALL: libdev

P4_HOME  = ./p4-1.4
LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
LIBDEV   = libmpid.a
CFLAGS   =  -DFORTRANUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -DHAS_XDR=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DMPI_ADI2 -I../../mpid/ch2 $(OPTFLAGS)  \
	-I../.. -I../../include -I./ -I../ch2 \
		-I../util -Ip4-1.4/include \
		-DMPID_DEVICE_CODE  -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_PKT_VAR_SIZE -DMPID_DEBUG_NONE -DMPID_STAT_NONE   -DMPE_USE_EXTENSIONS=1 -DHAVE_GETTIMEOFDAY=1
# See mpid/ch_adi2/Makefile.in for a full list
SOURCE   = adi2recv.c adi2send.c adi2init.c adi2probe.c adi2ssend.c \
	   adi2hsend.c adi2hrecv.c adi2pack.c adi2mpack.c \
	   adi2hssend.c bswap2.c adi2req.c adi2cancel.c \
	   chbeager.c chbrndv.c chdebug.c chchkdev.c chshort.c chflow.c \
	   chinit.c chnodename.c chhetero.c chtick.c p4priv.c \
	   queue.c cmnargs.c mprerr.c tr2.c sbcnst2.c 
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = adi2recv.o adi2send.o adi2init.o adi2probe.o adi2ssend.o \
	      adi2hsend.o adi2hrecv.o adi2pack.o adi2mpack.o \
	      adi2hssend.o bswap2.o adi2req.o adi2cancel.o \
	      chbeager.o chbrndv.o chdebug.o chchkdev.o chshort.o chflow.o \
	      chinit.o chnodename.o chhetero.o chtick.o p4priv.o \
	      $(ASMFILES_O) queue.o cmnargs.o tr2.o sbcnst2.o 

default: $(LIBNAME)
libdev: $(LIBDEV)

$(MPI_OBJECTS): ../ch2/mpid.h chdef.h

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: setlinks p4inmpi default_all_files
default_all_files: $(MPI_OBJECTS)
	$(AR) $(LIBNAME) $?
profile_all: 

clean:
	/bin/rm -f *~ $(MPI_OBJECTS) libmpid.a mprerr.o
	(cd $(P4_HOME); if [ -s Makefile ] ; then $(MAKE) clean; \
	/bin/rm -f lib/libp4.a ; fi ;)
#	/bin/rm -f Makefile lib/p4_config.h lib/libp4.a ; fi ;)

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

$(LIBNAME): $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

libmpid.a: $(MPI_OBJECTS) mprerr.o
	$(AR) $@ $?
	$(RANLIB) $@

p4inmpi:
	(cd $(P4_HOME); \
	$(MAKE) p4inmpi MPIARCH=$(ARCH) MPILIBNAME=mpi ; cd ..)

lint:
	lint -chvuz -I../.. -I../../include -I../ch2 \
		-I../ch_p4/p4-1.4/include -I./ \
	-DMPID_DEVICE_CODE $(SOURCE) 

