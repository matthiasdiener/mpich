# Generated automatically from Makefile.in by configure.
#
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#

##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
F77         = f77
AR          = ar clr
RANLIB      = ranlib
INCLUDE_DIR =  -I../../mpid/ch_p4 -I../../include
DEVICE      = ch_p4
OPTFLAGS    = -g

### End User configurable options ###
SHELL = /bin/sh

DEVICE_INCLUDE = ../../mpid/$(DEVICE)/dmch.h

MPI_INCLUDE    = ../../include/mpi.h ../../include/mpi_bc.h \
                 ../../include/mpir.h ../../include/mpi_ad.h \
		 ../../include/mpisys.h ./coll.h
DEVICE_LIB = 
CFLAGS =   -DHAVE_STDLIB_H=1 $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)  $(MPIPROFILE)

LIBS = ../../lib/$(ARCH)/$(COMM)/libmpi.a $(DEVICE_LIB)
MPI_CSOURCES = barrier.c bcast.c gather.c gatherv.c scatter.c scatterv.c \
               allgather.c allgatherv.c alltoall.c alltoallv.c reduce.c \
               allreduce.c red_scat.c scan.c global_ops.c \
	       opcreate.c opfree.c reduceutil.c

MPI_TSOURCES = barrier.c bcast.c gather.c gatherv.c scatter.c scatterv.c \
               allgather.c allgatherv.c alltoall.c alltoallv.c reduce.c \
               allreduce.c red_scat.c scan.c global_ops.c \
	       opfree.c

MPI_WSOURCES = barrierf.c bcastf.c gatherf.c gathervf.c scatterf.c scattervf.c\
               allgatherf.c allgathervf.c alltoallf.c alltoallvf.c reducef.c \
               allreducef.c red_scatf.c scanf.c global_opsf.c \
	       opcreatef.c opfreef.c

MPI_COBJECTS = barrier.o bcast.o gather.o gatherv.o scatter.o scatterv.o \
               allgather.o allgatherv.o alltoall.o alltoallv.o reduce.o \
               allreduce.o red_scat.o scan.o global_ops.o \
	       opcreate.o opfree.o reduceutil.o

MPI_WOBJECTS = barrierf.o bcastf.o gatherf.o gathervf.o scatterf.o scattervf.o\
               allgatherf.o allgathervf.o alltoallf.o alltoallvf.o reducef.o \
               allreducef.o red_scatf.o scanf.o \
	       opcreatef.o opfreef.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

MPI_SOURCES = $(MPI_CSOURCES) $(MPI_WSOURCES)
MPI_OBJECTS = $(MPI_COBJECTS) $(MPI_WOBJECTS)

profile: ../../lib/$(ARCH)/$(COMM)/libpmpi.a

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libpmpi.a $?
	
wrappers:
	../../util/bfort -ferr -mpi -mnative -mapptr -ptrprefix MPIR_ \
		-anyname -I pubinc $(MPI_TSOURCES)	

manpages:
	../../util/doctext -mpath ../../man/man3 -ext 3 -I pubinc \
	   -index ../../www.index -jumpfile ../../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPI \
		$(MPI_CSOURCES)
	../../util/doc2lt $(MPI_CSOURCES) >> ../../rsum.tmp

wwwpages:
	../../util/doctext -html -mpath ../../www/www3 -ext 3 -I pubinc \
	   -index ../../www.index -jumpfile ../../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPI \
		$(MPI_CSOURCES)

$(MPI_OBJECTS) :  $(MPI_INCLUDE)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

../../lib/$(ARCH)/$(COMM)/libmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

../../lib/$(ARCH)/$(COMM)/libpmpi.a: $(MPI_OBJECTS) 
	$(AR) $@ $?
	$(RANLIB) $@

clean:
	/bin/rm -f *.o *~ 

