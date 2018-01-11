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

SERVER_HOME = /etc

P4_PORT = 753
P4_PORT_NAME = serv_p4

CHMOD = chmod
CHOWN = chown
INSTALL = /bin/cp -i
SED   = sed

P4_HOME_DIR = ..
INCLUDEDIR = $(P4_HOME_DIR)/include
LIBDIR = $(P4_HOME_DIR)/lib

default:
	echo not making server on $(P4ARCH)

all: serv_p4 example_configs iserv_p4 killiserv_p4

example_configs: inetd.entry services.entry

serv_p4: serv_p4.o
	$(CLINKER) $(CFLAGS) -o serv_p4 serv_p4.o
	/bin/mv serv_p4 ../bin

iserv_p4:$(P)  iserv_p4.o
	$(CLINKER) $(CFLAGS) -o iserv_p4 iserv_p4.o $(LIBDIR)/libp4.a
	/bin/mv iserv_p4 ../bin

killiserv_p4:$(P)  killiserv_p4.o
	$(CLINKER) $(CFLAGS) -o killiserv_p4 killiserv_p4.o $(LIBDIR)/libp4.a
	/bin/mv killiserv_p4 ../bin

inetd.entry: inetd.entry.base
	$(SED) -e 's,P4_PORT_NAME,$(P4_PORT_NAME),g' \
	    -e 's,P4_PORT,$(P4_PORT),g' \
	    -e 's,SERVER_HOME,$(SERVER_HOME),g' \
	    inetd.entry.base > inetd.entry

services.entry: services.entry.base
	$(SED) -e 's,P4_PORT_NAME,$(P4_PORT_NAME),g' \
	    -e 's,P4_PORT,$(P4_PORT),g' \
	    -e 's,SERVER_HOME,$(SERVER_HOME),g' \
	    services.entry.base > services.entry

install: serv_p4
	$(INSTALL) ../bin/serv_p4 $(SERVER_HOME)
	$(CHOWN) root $(SERVER_HOME)/serv_p4
	$(CHMOD) 700 $(SERVER_HOME)/serv_p4

realclean:	clean
	$(RM)  -f *~ \#* inetd.entry services.entry \
		     serv_p4 Makefile P4Server.Log.* \
		     iserv_p4 killiserv_p4
