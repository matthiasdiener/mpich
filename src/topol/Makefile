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
		 ../../include/mpisys.h
DEVICE_LIB = 
CFLAGS =   -DHAVE_STDLIB_H=1 $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(MPIPROFILE)

LIBS = ../../lib/$(ARCH)/$(COMM)/libmpi.a $(DEVICE_LIB)

MPI_CSOURCES = topo_test.c graphdims_get.c graph_get.c cartdim_get.c \
              cart_get.c dims_create.c cart_map.c graph_map.c cart_create.c \
              graph_create.c cart_rank.c cart_coords.c graph_nbr_cnt.c \
              graph_nbr.c cart_shift.c cart_sub.c topo_util.c

# wrappers for routines with Fortran LOGICAL type being made by hand
MPI_TSOURCES = topo_test.c graphdims_get.c graph_get.c cartdim_get.c \
              dims_create.c graph_map.c \
              cart_rank.c cart_coords.c graph_nbr_cnt.c \
              graph_nbr.c 

MPI_COBJECTS = topo_test.o graphdims_get.o graph_get.o cartdim_get.o \
              cart_get.o dims_create.o cart_map.o graph_map.o cart_create.o \
              graph_create.o cart_rank.o cart_coords.o graph_nbr_cnt.o \
              graph_nbr.o cart_shift.o cart_sub.o topo_util.o

MPI_WSOURCES = topo_testf.c graphdimsgetf.c graph_getf.c cartdim_getf.c \
              cart_getf.c dims_createf.c cart_mapf.c graph_mapf.c cart_createf.c \
              graph_createf.c cart_rankf.c cart_coordsf.c graphnbrcntf.c \
              graph_nbrf.c cart_shiftf.c cart_subf.c

MPI_WOBJECTS = topo_testf.o graphdimsgetf.o graph_getf.o cartdim_getf.o \
              cart_getf.o dims_createf.o cart_mapf.o graph_mapf.o cart_createf.o \
              graph_createf.o cart_rankf.o cart_coordsf.o graphnbrcntf.o \
              graph_nbrf.o cart_shiftf.o cart_subf.o

default: ../../lib/$(ARCH)/$(COMM)/libmpi.a

profile: ../../lib/$(ARCH)/$(COMM)/libpmpi.a

MPI_SOURCES = $(MPI_CSOURCES) $(MPI_WSOURCES)
MPI_OBJECTS = $(MPI_COBJECTS) $(MPI_WOBJECTS)

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
default_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libmpi.a $?
profile_all: $(MPI_OBJECTS)
	$(AR) ../../lib/$(ARCH)/$(COMM)/libpmpi.a $?
	
# Move longname-wrappers to shorter names
wrappers:
	../../util/bfort -ferr -mpi -mnative -mapptr -ptrprefix MPIR_ \
		-anyname -I pubinc $(MPI_TSOURCES)	
	if [ -s graphdims_getf.c ] ; then \
	    /bin/mv graphdims_getf.c graphdimsgetf.c ; fi
	if [ -s graph_nbr_cntf.c ] ; then \
	    /bin/mv graph_nbr_cntf.c graphnbrcntf.c ; fi
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
	-$(AR) $@ $?
	$(RANLIB) $@

../../lib/$(ARCH)/$(COMM)/libpmpi.a: $(MPI_OBJECTS) 
	-$(AR) $@ $?
	$(RANLIB) $@

clean:
	/bin/rm -f *.o *~ 
