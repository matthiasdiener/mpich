# Generated automatically from Makefile.in by configure.

DEFS =  -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_SYSTEM=1 -DHAVE_LONG_DOUBLE=1

##### User configurable options #####

ARCH        = hpux
COMM        = ch_shmem
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /home/lusk/mpich
CC          = cc
F77         = f77
AR          = ar crl
RANLIB      = ranlib
INCLUDE_DIR =  -I../../mpid/ch_shmem -I../../include
DEVICE      = ch_shmem
OPTFLAGS    = -g
LIB_PATH    = -L/home/lusk/mpich/lib/hpux/ch_shmem 
FLIB_PATH   = -Wl,-L,/home/lusk/mpich/lib/hpux/ch_shmem
LIB_LIST    = -lmpi

### End User configurable options ###

SHELL = /bin/sh

LIBS	  = $(LIB_PATH) $(LIB_LIST)
FLIBS	  = $(FLIB_PATH) $(LIB_LIST)

CFLAGS = -g -DMPI_$(ARCH)

RM = /bin/rm

P2POBJ = p2p.o p2p_mon.o

all:	tst

tst:	tst.o $(P2POBJ)
	cc -o tst tst.o $(P2POBJ) $(LIBS)

addtwo.ask:	addtwo.ask.o $(P2POBJ)
	cc -o addtwo.ask addtwo.ask.o $(P2POBJ) $(LIBS)

grid.shmem:	grid.shmem.o $(P2POBJ)
	cc -o grid.shmem grid.shmem.o $(P2POBJ) $(LIBS)

barrtest:	barrtest.o $(P2POBJ)
	cc -o barrtest barrtest.o $(P2POBJ) $(LIBS)

adquad:	adquad.o $(P2POBJ)
	cc -o adquad adquad.o $(P2POBJ) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c 

clean:
	$(RM) -f  *.o addtwo grid.shmem


#  following for p2p developers (not needed by users)
#

configure:
	autoheader
	autoconf
    
cleanconfigure:
	$(RM) -f p3_configure.h.in configure 
	$(RM) -f config.cache config.status config.log

manpages:
	/home/gropp/bin/sun4/doctext -mpath man/man3 -ext 3 \
	-heading "p3 Sockets" p3_interface.c 
