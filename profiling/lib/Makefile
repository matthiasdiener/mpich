# Generated automatically from Makefile.in by configure.
#  (C) 1993 by Argonne National Laboratory and Mississipi State University.
#      All rights reserved.  See COPYRIGHT in top-level directory.
#

##### User configurable options #####

ARCH        = sun4
BOPT        = 
COPT        = -g
MPIR_HOME   = /Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = $(CC)
F77         = f77
AR          = ar clr
RANLIB      = ranlib
INCLUDE_DIR = -I/home/gropp/tools.n -I$(MPIR_HOME)/include \
	 -DMPE_GRAPHICS -I$(MPIR_HOME)/mpe
DEVICE      = chameleon
OPTFLAGS    = -g
TOOLS_DIR   = /home/gropp/tools.n

### End User configurable options ###

CFLAGS = $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH)
LIBNAME  = $(TOOLS_DIR)/libs/libs$(BOPT)$(PROFILE)/$(ARCH)/system.a

default: log_wrappers.c time_wrappers.c visual_mess.c

log_wrappers.c: log_wrappers.w
	../wrappergen/wrappergen -w log_wrappers.w -o log_wrappers.c 

time_wrappers.c: time_wrappers.w
	../wrappergen/wrappergen -w time_wrappers.w -o time_wrappers.c 

visual_mess.c: visual_mess.w
	../wrappergen/wrappergen -w visual_mess.w -w message_prof.w \
	-o visual_mess.c 

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	/bin/rm -rf *.o *.c *~

