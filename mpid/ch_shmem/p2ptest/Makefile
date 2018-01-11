# Generated automatically from Makefile.in by configure.

DEFS =  -DMPE_USE_EXTENSIONS=1 -D_ALL_SOURCE=1 -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1

##### User configurable options #####

ARCH        = rs6000
COMM        = ch_shmem
MPIR_HOME   = /Net/antireo/antireo9/MPI/mpich
CC          = xlC
F77         = xlf
AR          = ar crl
RANLIB      = ranlib
INCLUDE_DIR =  -I../../mpid/ch_shmem -I../../include
DEVICE      = ch_shmem
OPTFLAGS    = 
LIB_PATH    = -L/Net/antireo/antireo9/MPI/mpich/lib/rs6000/ch_shmem 
FLIB_PATH   = -L/Net/antireo/antireo9/MPI/mpich/lib/rs6000/ch_shmem 
LIB_LIST    = -lmpi 

### End User configurable options ###

SHELL = /bin/sh

#LIBS	  = $(LIB_PATH) $(LIB_LIST)
#FLIBS	  = $(FLIB_PATH) $(LIB_LIST)

CFLAGS = -g -DMPI_$(ARCH)  -DFORTRANNOUNDERSCORE  -DMPE_USE_EXTENSIONS=1 -D_ALL_SOURCE=1 -DSTDC_HEADERS=1  -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DMALLOC_RET_VOID=1 -DHAVE_SYSTEM=1 -DHAVE_NICE=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_LONG_LONG_INT=1 -DMPI_ADI2 -I../../mpid/ch2  -DMPID_DEBUG_NONE -DMPID_STAT_NONE  -DHAVE_MMAP=1 -DHAVE_MSEM_INIT=1 -DHAVE_SHMAT=1 -DHAVE_SEMOP=1 -DHAS_VOLATILE=1 -DHAVE_SIGACTION=1 -DHAVE_SIGNAL=1 -DHAVE_SIGSET=1 -DRETSIGTYPE=void -DHAVE_GETTIMEOFDAY=1

RM = /bin/rm

P2POBJ = ../p2p.o 
#p2p_mon.o

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
