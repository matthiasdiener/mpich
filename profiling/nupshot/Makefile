# Generated automatically from Makefile.in by configure.
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#

##### User configurable options #####

ARCH        = sun4
BOPT        = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = cc
INCLUDE_DIR =   $(TCLTK_INC)
OPTFLAGS    = -g
LIB_PATH    = $(TCLTK_LIB) -L/usr/local/lib
LIB_LIST    = -lX11 -lm

# It is easier if the tcl and tcl stuff is in the same directory,
# but if your installation has them in separate directories, so be it.

# Right now, TCLLIB and TKLIB can be specified as a command line option
# for configure, so we'll make do with what we can.

TCL_DIR     = 
#TCL_DIR     = /usr/local/tcl

TK_DIR      = 
#TK_DIR      = /usr/local/tcl

TCL_LIB     = -L$(TCL_DIR)/lib -ltcl
#TCL_LIB      = -L@TCLLIB@ -ltcl
TK_LIB      = -L$(TK_DIR)/lib -ltk
#TK_LIB      = -ltk
TCLTK_LIB   = $(TCL_LIB) $(TK_LIB)

TCL_INC     = -I$(TCL_DIR)/include
TK_INC      = -I$(TK_DIR)/include

TCLTK_INC   = $(TCL_INC) $(TK_INC)


### End User configurable options ###
SHELL = /bin/sh

EXECS = nupshot

CFLAGS =   -DHAVE_STDLIB_H=1 $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(MPE_GRAPH) \
	 "-DNUPSHOT_TCL_CODE=\"$(MPIR_HOME)/profiling/nupshot/nupshot.tcl\""

CSOURCES = alog.c bitmaps.c cvt_args.c events.c log.c log_common.c \
	   log_widget.c msgs.c states.c str_dup.c tclptr.c time_lbl.c \
	   timelines.c tkAppInit.c vis.c nupshot.c

COBJECTS = alog.o bitmaps.o cvt_args.o events.o log.o log_common.o \
	   log_widget.o msgs.o states.o str_dup.o tclptr.o time_lbl.o \
	   timelines.o tkAppInit.o vis.o nupshot.o

default: nupshot

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.c.a:
	$(CC) $(CFLAGS) -c $*.c

nupshot: $(COBJECTS)
	$(CLINKER) -o nupshot $(COBJECTS) \
	$(LIB_PATH) $(LIB_LIST)
clean:
	/bin/rm -f *.o *~ $(EXECS)

