# Generated automatically from Makefile.in by configure.
#
#  $Id: Makefile.in,v 1.16 1995/07/25 02:41:04 gropp Exp $
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#
##### User configurable options #####

ARCH        = sun4
COMM        = p4
DEVICE      = chameleon
BOPT        = g
P4_DIR      = /usr/local/p4-1.4
TOOLS_DIR   = /home/gropp/tools.n
MPIR_HOME   = /Net/alriga/alriga9/MPI/mpich
INCLUDE_DIR = -I/home/gropp/tools.n -Impid/chameleon -Iinclude

### End User configurable options ###

SHELL = /bin/sh

ALL: default
	
ITOOLSDIR  = $(TOOLS_DIR)
LIBNAME  = ../../lib/$(ARCH)/$(COMM)/libmpi.a
CFLAGS   = $(COPT) $(OPT) -I$(ITOOLSDIR) -I../.. -I../../include -I./ \
	-DMPID_DEVICE_CODE  -DMPID_PKT_VAR_SIZE
LDIR     = $(ITOOLSDIR)/libs/libs$(BOPT)$(PROFILE)/$(ARCH)
SOURCE   = chevent.c chrecv.c chsend.c chinit.c chprobe.c chsync.c chdebug.c
SOURCEC  = $(SOURCE)
SOURCEF  =
MPI_OBJECTS = chevent.o chrecv.o chsend.o chinit.o chprobe.o chsync.o chdebug.o

#$(LIBNAME)($(OBJS)): mpid.h dmch.h ../../include/mpir.h

clean:
	/bin/rm -f *~ $(MPI_OBJECTS) 

include $(ITOOLSDIR)/bmake/$(ARCH).$(BOPT)$(PROFILE)
include $(ITOOLSDIR)/bmake/$(ARCH)
include $(ITOOLSDIR)/bmake/$(ARCH).$(COMM)

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

profile:

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
	$(AR) cr ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: 

manpages:
	$(MPIR_HOME)/util/doctext -mpath $(MPIR_HOME)/man/man5 \
	 -ext 5 -I pubinc -index $(MPIR_HOME)/www.mpe.index \
	 -jumpfile $(MPIR_HOME)/jumpfile.list -nolocation \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading ADI readme
	$(MPIR_HOME)/util/doctext -latex -mpath $(MPIR_HOME)/ref/ref5 \
	 -ext 5 -I pubinc -nolocation -heading ADI readme
	$(MPIR_HOME)/util/doc2lt readme >> $(MPIR_HOME)/rsumadi.tmp

wwwpages:
	../../util/doctext -html -mpath ../../www/www5 -ext 5 -I pubinc \
	   -index ../../www.index -jumpfile ../../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading ADI \
		-nolocation readme

.c.o:
	$(CC) $(CFLAGS) $(BASEOPT) -c $*.c $(MPI_PROFILE)

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) cr $@ $?
	$(RANLIB) $@

chtest: chtest.c
	$(CC) -o chtest chtest.o 
