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

# Added -I for MPI includes
CFLAGS = -g -I$(INCLUDEDIR) -I../../../../include $(MDEP_CFLAGS) $(USER_CFLAGS)
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

ALOG_HDR = alog.h
ALOG_SRC = alog.c alogf.c
ALOG_OBJ = alog.o alogf.o

default:	
	make $(LIBDIR)/libp4.a mergelogs
	@echo NOT making adjlogs on $(P4ARCH)

all:	
	make $(LIBDIR)/libp4.a mergelogs
	@echo NOT making adjlogs on $(P4ARCH)


USER_CFLAGS = -D$(P4ARCH)

alog.o:$(P)
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c alog.c 

alogf.o:$(P)
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c alogf.c 

$(ALOG_OBJ):  $(ALOG_SRC) $(ALOG_HDR)

$(LIBDIR)/libp4.a:  $(ALOG_OBJ)
	$(AR) $(LIBDIR)/libp4.a $?
	$(RANLIB) $(LIBDIR)/libp4.a
#	$(AR) $@ $?
#	$(RANLIB) $@

p4inmpi:$(P) $(ALOG_OBJ)
	$(AR) ../../../../lib/$(MPIARCH)/ch_p4/libmpi.a $?
	$(RANLIB) ../../../../lib/$(MPIARCH)/ch_p4/libmpi.a

mergelogs: alog_evntdfs.h mergelogs.o
	$(CLINKER) $(CFLAGS) -o mergelogs mergelogs.o
	/bin/mv mergelogs ../bin

adjlogs:  alog_evntdfs.h adjlogs.o
	$(CLINKER) $(CFLAGS) -o adjlogs adjlogs.o -lmp
	/bin/mv adjlogs ../bin

realclean:	clean
	$(RM) -f  Makefile

