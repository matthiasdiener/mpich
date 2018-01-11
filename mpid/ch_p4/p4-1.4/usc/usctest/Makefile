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

P4_HOME_DIR = ../..
LIBDIR = $(P4_HOME_DIR)/lib
LIBS =  $(MDEP_LIBS)
INCLUDEDIR = /usr/include

USER_CFLAGS = -I$(INCLUDEDIR) -D$(P4ARCH)

default:

usctest: usctest.o
	$(CLINKER) $(CFLAGS) -o usctest usctest.o ../usc.o $(LIBS)

usctest2: usctest2.o
	$(CLINKER) $(CFLAGS) -o usctest2 usctest2.o ../usc.o $(LIBS)

capcell: capcell.o
	$(CLINKER) $(CFLAGS) -o capcell capcell.o ../usc.o $(LIBS)

realclean:	clean
	$(RM) -f *~ *.o usctest usctest2 capcell Makefile

