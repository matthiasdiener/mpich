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

MPI_CSOURCES = keyval_free.c keyval_create.c attr_getval.c \
		attr_delval.c attr_util.c attr_putval.c \
		group_excl.c group_diff.c group_free.c group_incl.c \
		group_inter.c group_rexcl.c group_rincl.c group_compare.c \
		group_rank.c group_size.c group_union.c group_util.c \
		comm_dup.c comm_free.c comm_group.c comm_create.c \
		comm_size.c comm_util.c comm_split.c context_util.c \
		group_tranks.c comm_test_ic.c comm_rank.c comm_compare.c \
		comm_rsize.c comm_rgroup.c ic_create.c ic_merge.c \
		null_copy_fn.c null_del_fn.c dup_fn.c

MPI_WSOURCES = keyval_freef.c \
		attr_delvalf.c \
		group_exclf.c group_difff.c group_freef.c group_inclf.c \
		group_interf.c \
		group_rankf.c group_sizef.c group_unionf.c groupcomparef.c \
		comm_dupf.c comm_freef.c comm_groupf.c comm_createf.c \
		comm_sizef.c comm_splitf.c \
		group_tranksf.c \
		comm_test_icf.c comm_rankf.c comm_comparef.c \
		comm_rsizef.c comm_rgroupf.c ic_createf.c ic_mergef.c

# still making comm_sizef.c and comm_rankf.c by hand
# MUST make attr_putvalf.c and attr_getvalf.c by hand
# MUST make group_rinclf.c and group_rexclf.c by hand
MPI_TSOURCES = keyval_free.c \
		attr_delval.c \
		group_free.c \
		group_rank.c group_size.c group_compare.c \
		comm_size.c group_tranks.c \
		comm_test_ic.c comm_rank.c comm_compare.c \
		comm_rsize.c 

MPI_COBJECTS = keyval_free.o keyval_create.o attr_getval.o \
		attr_delval.o attr_util.o attr_putval.o \
		group_excl.o group_diff.o group_free.o group_incl.o \
		group_inter.o group_rexcl.o group_rincl.o group_compare.o \
		group_rank.o group_size.o group_union.o group_util.o \
		comm_dup.o comm_free.o comm_group.o comm_create.o comm_rank.o \
		comm_size.o comm_util.o comm_split.o context_util.o \
		group_tranks.o comm_test_ic.o comm_compare.o \
		comm_rsize.o comm_rgroup.o ic_create.o ic_merge.o \
		group_rinclf.o group_rexclf.o \
		null_copy_fn.o null_del_fn.o dup_fn.o

MPI_WOBJECTS = keyval_freef.o keyvalcreatef.o attr_getvalf.o \
		attr_delvalf.o  attr_putvalf.o \
		group_exclf.o group_difff.o group_freef.o group_inclf.o \
		group_interf.o \
		group_rankf.o group_sizef.o group_unionf.o groupcomparef.o \
		comm_dupf.o comm_freef.o comm_groupf.o comm_createf.o \
		comm_sizef.o comm_splitf.o group_tranksf.o \
		comm_test_icf.o comm_rankf.o comm_comparef.o \
		comm_rsizef.o comm_rgroupf.o ic_createf.o ic_mergef.o \
		null_copy_fnf.o null_del_fnf.o dup_fnf.o

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
	-/bin/rm -f $(MPI_WSOURCES)
	../../util/bfort -ferr -mpi -mnative -mapptr -ptrprefix MPIR_ \
		-anyname -I pubinc $(MPI_TSOURCES)	
	if [ -s keyval_createf.c ] ; then \
	    /bin/mv keyval_createf.c keyvalcreatef.c ; fi
	if [ -s group_comparef.c ] ; then \
	    /bin/mv group_comparef.c groupcomparef.c ; fi
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
