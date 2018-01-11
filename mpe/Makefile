# Generated automatically from Makefile.in by configure.
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#

##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
BOPT        = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = cc
F77         = f77
AR          = ar clr
RANLIB      = ranlib
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4
OPTFLAGS    = -g
MPE_GRAPH   = -DMPE_GRAPHICS
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4  -L/usr/local/lib
# On the meiko, you need to add -lsocket to LIB_LIST
LIB_LIST    = -lmpi 

MPEGRAPHICS_SOURCE = mpe_graphics.c xcolor.c xframe.c xinit.c xwmap.c xmouse.c
MPEGRAPHICS_OBJS   = mpe_graphics.o xcolor.o xframe.o xinit.o xwmap.o xmouse.o
MPEGRAPHICS_FSOURCE = mpe_graphicsf.c
MPEGRAPHICS_FOBJS   = mpe_graphicsf.o
### End User configurable options ###
SHELL = /bin/sh

EXECS = dbxtest

CFLAGS =   -DHAVE_STDLIB_H=1 $(OPTFLAGS) $(INCLUDE_DIR) $(MPIPROFILE) -DMPI_$(ARCH) \
	 $(MPE_GRAPH)

MPE_CSOURCES = timers.c  $(MPEGRAPHICS_SOURCE) mpe_log.c decomp.c \
	       mpe_seq.c
MPE_TSOURCES = $(MPEGRAPHICS_SOURCE) decomp.c mpe_seq.c dbxerr.c
MPE_COBJECTS = timers.o  $(MPEGRAPHICS_OBJS) mpe_log.o decomp.o mpe_seq.o \
		dbxerr.o
MPE_WSOURCES = $(MPEGRAPHICS_FSOURCE) mpe_logf.c decompf.c mpe_seqf.c
MPE_WOBJECTS = timersf.o $(MPEGRAPHICS_FOBJS) mpe_logf.o decompf.o mpe_seqf.o

MPE_SOURCES = $(MPE_CSOURCES) $(MPE_WSOURCES)
MPE_OBJECTS = $(MPE_COBJECTS) $(MPE_WOBJECTS)

# default_all is the same as default, but without the RANLIB.  This
# can speed up the build (ranlibs can take a LONG time).  profile_all 
# is the same, but for the profile library
# **** Currently the same as default for simplicity ****
default_all: default
profile_all: 


default: $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe.a \
	 $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe_nompi.a \
	 $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/mpe_prof.o \
	 $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/mpe_proff.o
profile:
wrappers:
	-/bin/rm -f $(MPI_WSOURCES)
	$(MPIR_HOME)/util/bfort -ferr -mpi -mnative -mapptr -ptrprefix MPIR_ \
		-I pubinc $(MPE_TSOURCES)

#
# It is tempting here to use ... libmpe.a($(MPE_OBJECTS)) as the dependency,
# but this would require that all makes properly handle library dependencies.
# Since makes that DO are the exception rather than the rule, we don't
# use this form
$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe.a: $(MPE_OBJECTS)
	$(AR) $@ $?
	$(RANLIB) $@
	-/bin/rm -f $?

$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe_nompi.a:  $(MPEGRAPHICS_SOURCE)
	@if [ "$(MPEGRAPHICS_SOURCE)" != "" ] ; then \
	make -f Makefile_nompi $@ ; \
	fi
manpages:
	$(MPIR_HOME)/util/doctext -mpath $(MPIR_HOME)/man/man4 \
	 -ext 4 -I pubinc -index $(MPIR_HOME)/www.mpe.index \
	 -jumpfile $(MPIR_HOME)/jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPE \
		$(MPE_CSOURCES)
	$(MPIR_HOME)/util/doc2lt $(MPE_CSOURCES) >> $(MPIR_HOME)/rsummpe.tmp

wwwpages:
	../util/doctext -html -mpath ../www/www4 -ext 4 -I pubinc \
	   -index ../www.index -jumpfile ../jumpfile.list \
	   -indexdir "http:/www.mcs.anl.gov/mpi/man" -heading MPE \
		$(MPE_CSOURCES)


mpe_log.o: mpe_log.h mpe_log.c mpe_log_genproc.c mpe_log_genproc.h \
	  mpe_log_merge.c mpe_log_adjusttime.c

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.c.a:
	$(CC) $(CFLAGS) -c $*.c

$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/mpe_prof.o: mpe_prof.c
	$(CC) $(CFLAGS) -c mpe_prof.c
	/bin/cp mpe_prof.o $(MPIR_HOME)/lib/$(ARCH)/$(COMM)

$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/mpe_proff.o: mpe_proff.c
	$(CC) $(CFLAGS) -c mpe_proff.c
	/bin/cp mpe_proff.o $(MPIR_HOME)/lib/$(ARCH)/$(COMM)


dbxtest: dbxtest.o $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe.a
	$(CLINKER) -o dbxtest dbxtest.o \
		$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpe.a \
		$(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a \
	$(LIB_PATH) $(LIB_LIST)
clean:
	/bin/rm -f *.o *~ $(EXECS)
	(cd contrib/mandel ; make clean)

