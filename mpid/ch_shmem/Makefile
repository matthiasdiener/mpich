# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.6 1996/07/06 14:11:16 gropp Exp $
#
#  (C) 1993,1995 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
# Eventually, there will be two sets of routines
#
#
##### User configurable options #####

ARCH        = rs6000
COMM        = ch_shmem
DEVICE      = ch_shmem
MPIR_HOME   = /Net/antireo/antireo9/MPI/mpich
INCLUDE_DIR =  -Impid/ch_shmem -Iinclude
CC          = xlC
AR          = ar crl
RANLIB      = ranlib
OPTFLAGS    = 
SHELL       = /bin/sh
ASMFILES_O  = 
### End User configurable options ###

#ALL: default
ALL: libdev

LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
LIBDEV   = libmpid.a
CFLAGS   =  -DFORTRANNOUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -D_ALL_SOURCE=1 -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 -DMPI_ADI2 -I../../mpid/ch2 $(OPTFLAGS)  \
	   	-I../.. -I../../include -I./ -I../ch2 \
		-I../util -DMPI_rs6000 \
		-DMPID_DEVICE_CODE  -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_DEBUG_NONE -DMPID_STAT_NONE  -DHAVE_MMAP=1 -DHAVE_MSEM_INIT=1 -DHAVE_SHMAT=1 -DHAVE_SEMOP=1 -DHAS_VOLATILE=1 -DHAVE_SIGACTION=1 -DHAVE_SIGNAL=1 -DHAVE_SIGSET=1 -DRETSIGTYPE=void -DHAVE_GETTIMEOFDAY=1
CFLAGSNOOPT   =  -DFORTRANNOUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -D_ALL_SOURCE=1 -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 -DMPI_ADI2 -I../../mpid/ch2 \
	   	-I../.. -I../../include -I./ -I../ch2 \
		-I../util -DMPI_rs6000 \
		-DMPID_DEVICE_CODE  -DHAVE_GETHOSTNAME=1 -DHAVE_UNAME=1  -DMPID_DEBUG_NONE -DMPID_STAT_NONE  -DHAVE_MMAP=1 -DHAVE_MSEM_INIT=1 -DHAVE_SHMAT=1 -DHAVE_SEMOP=1 -DHAS_VOLATILE=1 -DHAVE_SIGACTION=1 -DHAVE_SIGNAL=1 -DHAVE_SIGSET=1 -DRETSIGTYPE=void -DHAVE_GETTIMEOFDAY=1
# See mpid/ch2/Makefile.in for a full list
# Many of the chxxxx files are replaced with shmemxxxx files because the
# shared memory version uses a slightly different form, in particular, it
# allocates packets from a shared area rather than on the call stack.
# chbeager.c chbrndv.c 
SOURCE   = adi2recv.c adi2send.c adi2init.c adi2probe.c adi2ssend.c \
	   adi2hsend.c adi2hrecv.c adi2hssend.c adi2pack.c adi2mpack.c \
	   adi2req.c adi2cancel.c \
	   shmemdebug.c shmemchkdev.c \
           shmemshort.c shmemneager.c shmemnrndv.c shmempriv.c p2p.c \
	   shmeminit.c chnodename.c chtick.c queue.c cmnargs.c \
	   mprerr.c tr2.c sbcnst2.c 
SOURCEC  = $(SOURCE)
SOURCEF  =
# chbeager.o chbrndv.o 
MPI_OBJECTS = adi2recv.o adi2send.o adi2init.o adi2probe.o adi2ssend.o \
	      adi2hsend.o adi2hrecv.o adi2hssend.o adi2pack.o adi2mpack.o \
	      adi2req.o adi2cancel.o \
	      shmemdebug.o shmemchkdev.o \
	      shmemshort.o shmemneager.o shmemnrndv.o shmempriv.o p2p.o \
	      shmeminit.o chnodename.o chtick.o queue.o cmnargs.o \
	      tr2.o sbcnst2.o \
	      $(ASMFILES_O) 

default: $(LIBNAME)
libdev: $(LIBDEV)

$(MPI_OBJECTS): ../ch2/mpid.h mpiddev.h
p2p.o: p2psemop.c p2p.c 

#
# shmempriv is sensitive to memory access ordering.  Despite the use
# of VOLATILE statements, some compilers may be over-aggressive when
# compiling this file.  
#
shmempriv.o: shmempriv.c
	$(CC) $(CFLAGSNOOPT) $(BASEOPT) -c shmempriv.c

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

