# Makefile
ARCH        = sun4
COMM        = p4
P4_DIR      = /usr/local/p4
TOOLS_DIR   = /usr/local/tools
TOOLS_LIBDIR= $(TOOLS_DIR)/libs/libsg/$(ARCH)
MPIR_HOME   = /usr/local/mpich
CC          = cc
CLINKER     = $(CC)
INCLUDE_DIR =  -I$(TOOLS_DIR) -I$(MPIR_HOME)/mpid/chameleon -I$(MPIR_HOME)/include
DEVICE_LIB  = $(P4_DIR)/lib/libp4.a $(TOOLS_LIBDIR)/cmain$(COMM).o \
	      $(TOOLS_LIBDIR)/tools$(COMM).a $(TOOLS_LIBDIR)/tools.a \
	      $(TOOLS_LIBDIR)/system.a $(CLIB)
CFLAGS      = -g $(INCLUDE_DIR) -DMPI_$(ARCH)
LIBS        = $(MPIR_HOME)/mpe/lib/$(ARCH)/libmpe.a  \
              $(MPIR_HOME)/lib/$(ARCH)/libmpi.a $(DEVICE_LIB)

EXECS = mandel_mpi xpx


#.SUFFIXES: .o .c

all: mandel_mpi xpx

mandel_mpi:	mandel_mpi.o
	$(CLINKER) -g -o $@  $(DEVICE_MAIN) $< $(LIBS)

xpx:	xpx.c
	$(CC) -o xpx xpx.c -lX11 -lm

#.c.o: 	%.c 
#	$(CC) $(CFLAGS) -c $*.c

clean:
	/bin/rm -f *.o *~ PI* $(EXECS)


