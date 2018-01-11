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

# prototype makefile for util directory of P4

RM = rm

defs.MD: install.awk defs.all makefile.proto
	@if grep $(P4ARCH) machines > /dev/null ;\
	then \
	echo making machine-dependent definitions for $(P4ARCH) ;\
	awk -f install.awk machine=$(P4ARCH) defs.all > defs.MD ;\
	else \
	echo cannot install for $(P4ARCH), valid machines are: ;\
	cat machines ;\
	exit 1 ;\
	fi

default:

all:

realclean:
	$(RM) -f Makefile defs.MD *~ *.o
