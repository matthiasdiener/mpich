# P4 makefile for Sun
P4ARCH = SUN
AR = ar lruv
FC = f77
CLINKER = cc 
FLINKER = f77 
RANLIB = ranlib
MDEP_LIBS = 
MDEP_CFLAGS = 
MDEP_FFLAGS = 
P4_OBJ = $(P4_COMM_OBJ) $(P4_SOCK_OBJ) $(P4_SHMEM_OBJ)

RM = /bin/rm

CFLAGS = -g -I$(INCLUDEDIR) $(MDEP_CFLAGS) $(USER_CFLAGS)
#CFLAGS = -O -I$(INCLUDEDIR) $(MDEP_CFLAGS) $(USER_CFLAGS)

FFLAGS = -g -I$(FORTLIBDIR) $(MDEP_FFLAGS)

.SUFFIXES:	.o .c .f .h

.c.o:$(P)
	$(CC) $(CFLAGS) -c $*.c 

.f.o:$(P)
	$(FC) $(FFLAGS) -c $*.f 

dummy:	default

clean:
	$(RM) -f *~ *.o *.bak tags TAGS core

P4_HOME_DIR = ..
INCLUDEDIR = $(P4_HOME_DIR)/include
LIBDIR = $(P4_HOME_DIR)/lib
LIBS = $(MDEP_LIBS)

USC_HDR = usc_sys.h usc.h
USC_SRC = usc.c
USC_OBJ = usc.o

default:	
	make $(LIBDIR)/libp4.a

all:	
	make $(LIBDIR)/libp4.a

USER_CFLAGS = -D$(P4ARCH)

usc.o:$(P)
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c usc.c 

$(USC_OBJ):  $(USC_SRC) $(USC_HDR)


$(LIBDIR)/libp4.a:  $(USC_OBJ)
	$(AR) $(LIBDIR)/libp4.a $?
	$(RANLIB) $(LIBDIR)/libp4.a
#	$(AR) $@ $?
#	$(RANLIB) $@

p4inmpi:$(P) $(USC_OBJ)
	$(AR) ../../../../lib/$(MPIARCH)/ch_p4/libmpi.a $?
	$(RANLIB) ../../../../lib/$(MPIARCH)/ch_p4/libmpi.a

realclean:	clean
	$(RM) -f $(LIBDIR)/libp4.a Makefile
	@cd usctest; make clean

