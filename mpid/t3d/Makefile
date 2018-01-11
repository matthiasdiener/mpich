# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.3 1995/06/07 06:09:14 bright Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = cray_t3d
COMM        = t3d
DEVICE      = t3d
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /usr/tmp/bright/mpich
INCLUDE_DIR =  -Impid/t3d -Iinclude -I.
AR          = ar crl
RANLIB      = true
LINTFLAGS   = -v -u
CC          = /mpp/bin/cc
ASM         = /mpp/bin/cam
ASMFILES_O  = shmem_stack.o

### End User configurable options ###

SHELL = /bin/sh

LIBNAME     = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS      =  -Tcray-t3d -DFORTRANCAPS   -DHAVE_STDLIB_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -g $(COPT) $(OPT) -I../.. -I../../include -I. \
              -DMPID_DEVICE_CODE 
SOURCES     = t3dcoll.c   t3dinit.c   t3drecv.c   t3dsync.c \
              t3devent.c  t3dprobe.c  t3dsend.c   t3ddebug.c
OBJECTS     = $(SOURCES:.c=.o) $(ASMFILES_O)


ALL: default

default: $(LIBNAME) 

$(LIBNAME): $(OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

default_all: $(OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?

.c.o:
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

.s.o:
	$(ASM) -I/usr/include/mpp $*.s

lint:
	lint $(LINTFLAGS) $(CFLAGS) $(SOURCES)

clean:
	/bin/rm -f *~ $(OBJECTS) 

