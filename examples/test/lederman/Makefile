# Generated automatically from Makefile.in by configure.
ALL: default
##### User configurable options #####

ARCH        = sun4
COMM        = ch_p4
BOPT        = 
P4_DIR      = 
TOOLS_DIR   = 
MPIR_HOME   = /tmp_mnt/Net/antireo/antireo6/lusk/mpich
CC          = cc
CLINKER     = $(CC)
CCC         = 
CCLINKER    = $(CCC)
F77         = f77
FLINKER     = $(F77)
AR          = ar clr
RANLIB      = ranlib
PROFILING   = $(PMPILIB)
OPTFLAGS    = -g
LIB_PATH    = -L/tmp_mnt/Net/antireo/antireo6/lusk/mpich/lib/sun4/ch_p4 
LIB_LIST    = -lmpi 
#
INCLUDE_DIR =  -I$(MPIR_HOME)/include 
DEVICE      = ch_p4
PRISM_DEFINES = -DPRISM_MPI_ANL
### End User configurable options ###


CFLAGS  =   -DHAVE_STDLIB_H=1  $(OPTFLAGS) $(INCLUDE_DIR) -DMPI_$(ARCH) $(PRISM_DEFINES)
CCFLAGS = $(CFLAGS)
#FFLAGS = '-qdpc=e' 
FFLAGS =  $(OPTFLAGS)
MPILIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi.a 
MPIPPLIB = $(MPIR_HOME)/lib/$(ARCH)/$(COMM)/libmpi++.a
LIBS = $(LIB_PATH) $(LIB_LIST)
LIBSPP = $(MPIPPLIB) $(LIBS)

EXECS = prism_test_bc_mc_ge prism_test_bc_mc_gn prism_test_bc_pc_ge_rd \
        prism_test_bc_pc_ge_re prism_test_bc_pc_gn_rd \
        prism_test_bc_pc_gn_re prism_test_sk_mc_gn  prism_test_sk_mc_ge \
        prism_test_sk_pc_ge_rd prism_test_sk_pc_ge_re prism_test_sk_pc_gn_rd \
        prism_test_sk_pc_gn_re

PRISM_OBJS = prism_test.o prism_v_init_msg.o prism_v_chk_msg.o \
	     prism_v_init_var.o prism_v_generror.o prism_rand.o \
	     prism_v_finish.o prism_i_datatype_size.o prism_x_datatype.o

PRISM_TS_OBJS = prism_test.o_ts prism_v_init_msg.o_ts prism_v_chk_msg.o_ts \
	      prism_v_init_var.o_ts prism_v_generror.o_ts prism_rand.o_ts \
	      prism_v_finish.o_ts prism_i_datatype_size.o_ts \
	      prism_x_datatype.o_ts prism_v_skew.o_ts prism_v_copy.o_ts

PRISM_TS_SR_OBJS = prism_test.o_ts_sr prism_v_init_msg.o_ts_sr \
		   prism_v_chk_msg.o_ts_sr prism_v_init_var.o_ts_sr \
		   prism_v_generror.o_ts_sr prism_rand.o_ts_sr \
	           prism_v_finish.o_ts_sr prism_i_datatype_size.o_ts_sr \
	           prism_x_datatype.o_ts_sr prism_v_skew.o_ts_sr \
		   prism_v_copy.o_ts_sr

PRISM_TS_GE_OBJS = prism_test.o_ts_ge prism_v_init_msg.o_ts_ge \
		   prism_v_chk_msg.o_ts_ge prism_v_init_var.o_ts_ge \
		   prism_v_generror.o_ts_ge prism_rand.o_ts_ge \
	           prism_v_finish.o_ts_ge prism_i_datatype_size.o_ts_ge \
	           prism_x_datatype.o_ts_ge prism_v_skew.o_ts_ge \
		   prism_v_copy.o_ts_ge

PRISM_TS_GE_SR_OBJS = prism_test.o_ts_ge_sr prism_v_init_msg.o_ts_ge_sr \
		   prism_v_chk_msg.o_ts_ge_sr prism_v_init_var.o_ts_ge_sr \
		   prism_v_generror.o_ts_ge_sr prism_rand.o_ts_ge_sr \
	           prism_v_finish.o_ts_ge_sr prism_i_datatype_size.o_ts_ge_sr \
	           prism_x_datatype.o_ts_ge_sr prism_v_skew.o_ts_ge_sr \
		   prism_v_copy.o_ts_ge_sr

PRISM_TS_MC_OBJS = prism_test.o_ts_mc prism_v_init_msg.o_ts_mc \
		   prism_v_chk_msg.o_ts_mc prism_v_init_var.o_ts_mc \
		   prism_v_generror.o_ts_mc prism_rand.o_ts_mc \
	           prism_v_finish.o_ts_mc prism_i_datatype_size.o_ts_mc \
	           prism_x_datatype.o_ts_mc

PRISM_TS_GE_MC_OBJS = prism_test.o_ts_ge_mc prism_v_init_msg.o_ts_ge_mc \
		   prism_v_chk_msg.o_ts_ge_mc prism_v_init_var.o_ts_ge_mc \
		   prism_v_generror.o_ts_ge_mc prism_rand.o_ts_ge_mc \
	           prism_v_finish.o_ts_ge_mc prism_i_datatype_size.o_ts_ge_mc \
	           prism_x_datatype.o_ts_ge_mc

PRISM_BR_OBJS = prism_test.o_br prism_v_init_msg.o_br prism_v_chk_msg.o_br \
	      prism_v_init_var.o_br prism_v_generror.o_br prism_rand.o_br \
	      prism_v_finish.o_br prism_i_datatype_size.o_br \
	      prism_x_datatype.o_br prism_v_bc.o_br

PRISM_GE_OBJS = prism_test.o_ge prism_v_init_msg.o_ge prism_v_chk_msg.o_ge \
	      prism_v_init_var.o_ge prism_v_generror.o_ge prism_rand.o_ge \
	      prism_v_finish.o_ge prism_i_datatype_size.o_ge \
	      prism_x_datatype.o_ge prism_v_bc.o_ge 

PRISM_GE_BR_OBJS = prism_test.o_ge_br prism_v_init_msg.o_ge_br \
		   prism_v_chk_msg.o_ge_br prism_v_init_var.o_ge_br \
		   prism_v_generror.o_ge_br prism_rand.o_ge_br \
	           prism_v_finish.o_ge_br prism_i_datatype_size.o_ge_br \
	           prism_x_datatype.o_ge_br prism_v_bc.o_ge_br

PRISM_MC_OBJS = prism_test.o_mc prism_v_init_msg.o_mc prism_v_chk_msg.o_mc \
	      prism_v_init_var.o_mc prism_v_generror.o_mc prism_rand.o_mc \
	      prism_v_finish.o_mc prism_i_datatype_size.o_mc \
	      prism_x_datatype.o_mc

PRISM_GE_MC_OBJS = prism_test.o_ge_mc prism_v_init_msg.o_ge_mc \
		   prism_v_chk_msg.o_ge_mc prism_v_init_var.o_ge_mc \
		   prism_v_generror.o_ge_mc prism_rand.o_ge_mc \
	           prism_v_finish.o_ge_mc prism_i_datatype_size.o_ge_mc \
	           prism_x_datatype.o_ge_mc

.SUFFIXES: .c .o .o_ts .o_ts_sr .o_ts_ge .o_ts_ge_sr .o_ts_mc \
.o_ts_ge_mc .o_br .o_ge .o_ge_br .o_mc .o_ge_mc

default: $(EXECS)

all: testing

prism_test_sk_pc_gn_re: $(PRISM_TS_OBJS) 
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_OBJS) $(LIBS)

prism_test_sk_pc_gn_rd: $(PRISM_TS_SR_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_SR_OBJS) $(LIBS)

prism_test_sk_pc_ge_re: $(PRISM_TS_GE_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_GE_OBJS)  $(LIBS)

prism_test_sk_pc_ge_rd: $(PRISM_TS_GE_SR_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_GE_SR_OBJS) $(LIBS)

prism_test_sk_mc_gn: $(PRISM_TS_MC_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_MC_OBJS) $(LIBS)

prism_test_sk_mc_ge: $(PRISM_TS_GE_MC_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_TS_GE_MC_OBJS) $(LIBS)

prism_test_bc_pc_gn_re: $(PRISM_GE_BR_OBJS) 
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_GE_BR_OBJS) $(LIBS)

prism_test_bc_pc_gn_rd: $(PRISM_BR_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_BR_OBJS) $(LIBS)

prism_test_bc_pc_ge_re: $(PRISM_GE_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_GE_OBJS)  $(LIBS)

prism_test_bc_pc_ge_rd: $(PRISM_GE_BR_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_GE_BR_OBJS) $(LIBS)

prism_test_bc_mc_gn: $(PRISM_MC_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_MC_OBJS) $(LIBS)

prism_test_bc_mc_ge: $(PRISM_GE_MC_OBJS)
	$(CLINKER) $(OPTFLAGS) -o $@ $(PRISM_GE_MC_OBJS) $(LIBS)

clean:
	/bin/rm -f *.o_* *.o *~ PI* $(EXECS)

cleanobjs: 
	/bin/rm -f *.o PI*


.c.o:
	$(CC) $(CFLAGS) -c $*.c
#
# Note: Some compilers do not accept or correctly process -c -o filename.
# We will need to fix this some day....
#
.c.o_ts:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -c -o $@ $*.c
.c.o_ts_sr:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -DPRISM_SKEW_READY -c -o $@ $*.c
.c.o_ts_ge:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -DPRISM_GSYNCE -c -o $@ $*.c
.c.o_ts_ge_sr:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -DPRISM_GSYNCE \
              -DPRISM_SKEW_READY -c -o $@ $*.c
.c.o_ts_mc:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -DPRISM_MPI_COLL -c -o $@ $*.c
.c.o_ts_ge_mc:
	$(CC) $(CFLAGS) -DPRISM_TEST_SKEW -DPRISM_GSYNCE \
	      -DPRISM_MPI_COLL -c -o $@ $*.c
.c.o_br:
	$(CC) $(CFLAGS) -DPRISM_BC_READY -c -o $@ $*.c
.c.o_ge:
	$(CC) $(CFLAGS) -DPRISM_GSYNCE -c -o $@ $*.c	
.c.o_ge_br:
	$(CC) $(CFLAGS) -DPRISM_GSYNCE -DPRISM_BC_READY -c -o $@ $*.c
.c.o_mc:
	$(CC) $(CFLAGS) -DPRISM_MPI_COLL -c -o $@ $*.c	
.c.o_ge_mc:
	$(CC) $(CFLAGS) -DPRISM_GSYNCE -DPRISM_MPI_COLL -c -o $@ $*.c	




