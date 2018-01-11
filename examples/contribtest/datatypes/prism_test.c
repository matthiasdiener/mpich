/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   PRISM routine (version 2.1)

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

/* PURPOSE
   =======
   Routine to check accuracy and speed of collective routines
*/

/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"
#include <stdio.h>
#include <math.h>
#include <stddef.h>

/* CONDITIONAL COMPILATION */

#if PRISM_OUTTERM
/* 
  the "PRISM_OUTTERM" flag causes primary output to be sent to 
  the screen (stderr)
  instead of a file.  this is normally used for debugging when our routine
  hangs
*/
#endif

#if PRISM_GSYNCE
/* gsync before each trial */
#define Prism_WAYGSYNC "GSYNCE"
#elif PRISM_GSYNC1
/* only gsync once at start of loop */
#define Prism_WAYGSYNC "GSYNC1"
#else
/* don't gsync at all */
#define Prism_WAYGSYNC "NOGSYNC"
#endif

#if PRISM_MPI_COLL
/* use MPI groups for collective operation */
#define Prism_COLL "MPI"
#else
/* use our PRISM routine */
#define Prism_COLL "PRISM"
#endif

#if PRISM_TEST_SKEW
#define Prism_ROUTINE "SKEW"
#else
#define Prism_ROUTINE "Bcast"
#endif

#if PRISM_OUT_TIME
/* create output file with  time */
#endif
#if PRISM_OUT_MBPS
/* create output file with Mbytes/sec */
#endif

#if PRISM_NX
#define Prism_WAYCOMM "NX"
#else
#define Prism_WAYCOMM "MPI"
#endif

#if PRISM_MPI_COLL && PRISM_NX
/* you cannot use PRISM_MPI_COLL with PRISM_NX
   put garbage out to cause compile error */
***error***you cannot use -DPRISM_MPI_COLL with -DPRISM_NX***;
#endif

/* GLOBAL CONSTANTS */

#if !PRISM_NX
#define PRISM_NUM_DIMS 2
#endif

/* number of arguments code can receive */
#define Prism_NUM_ARGS 12

/* default datatype to use */
#define Prism_DFLT_DATATYPE prism_int
/* smallest message in elements to send */
#define Prism_DFLT_MSG_MIN 1
/* message size not to exceed in elements */
#define Prism_DFLT_MSG_MAX 8192
/* default factor to use.  the last message size (lms) will be
   incremented by nms = lms * (1.0 + Prism_DFLT_MSG_FACTOR / 100.0) +
   Prism_DFLT_MSG_STEP to get the next message size (nms). */
#define Prism_DFLT_MSG_FACTOR 100
#define Prism_DFLT_MSG_STEP 0
/* default number of trials to average over */
#define Prism_DFLT_TRIALS 3
/* default direction for collective operation to occur:
   in rows of 2D topology */
#define Prism_DFLT_DIRECTION 0
/* if skew then # of nodes to skew over:
     if doing vert skew then each column j skews (j + offset)
     if doing horz skew then each row i skews (i + offset)
   if bcast, then offset to use in each row/column as the root of the bcast:
      if doing a vert bcast, then each column j chooses the
        (j + offset) % # rows
      if doing a horz bcast, then each row i chooses the
        (i + offset) % # cols */
#define Prism_DFLT_OFFSET 0
/* default number of elements to add in skew */
#define Prism_DFLT_VARY 0

/* unique file name for each node for output */
#if PRISM_TEST_SKEW
#define PRISM_F_NAME_OUT "skewr%dc%d.m"
#define PRISM_F_NAME_ERR "skewr%dc%d.err"
#if PRISM_OUT_TIME
#define Prism_F_NAME_OUTT "skew%d.X"
#endif
#if PRISM_OUT_MBPS
#define Prism_F_NAME_OUTM "skewm%d.X"
#endif
#else
#define PRISM_F_NAME_OUT "bcr%dc%d.m"
#define PRISM_F_NAME_ERR "bcr%dc%d.err"
#if PRISM_OUT_TIME
#define Prism_F_NAME_OUTT "bc%d.X"
#endif
#if PRISM_OUT_MBPS
#define Prism_F_NAME_OUTM "bcm%d.X"
#endif
#endif

main(argc, argv)
     int argc;
     char **argv;
{

  /* --------------- */
  /* Local variables */
  /* --------------- */

  int
#if PRISM_TEST_SKEW
#if PRISM_MPI_COLL
    i_node_send, /* node to send to in skew */
#else
    i_dist_skew,
#endif
    i_node_recv, /* node to recv from in skew */
#else
   i_node_bc, /* node to start broadcast */
#endif
    i_vary, /* whether vary amount of data to send/recv in skew */
    i_recv, /* who you received data from in collective op */
    i_dir, /* holds value for what direction to collective op in */
    p_i_args[Prism_NUM_ARGS], /* array to hold input arguments */
    i_offset, /* value to add when determining root of broadcast
		 or amount to skew */
    i_clock_trials, /* number of times to time overhead of clock */
    i_t1, /* dummy int for 0 lenght messaage send */
    i_t2, /* dummy int for 0 lenght messaage send */
    i_msg_send_elements, /* # elements to send */
    i_msg_send_bytes, /* # bytes to send */
#if PRISM_TEST_SKEW
    i_msg_recv_elements, /* # elements to recv in skew */
    i_msg_recv_bytes, /* # bytes to recv in skew */
#endif
    i_node_id,  /* rank in 2D topology or id in NX */
#if !PRISM_NX
    p_i_dims[PRISM_NUM_DIMS],
    p_i_periods[PRISM_NUM_DIMS],
    p_i_coords[PRISM_NUM_DIMS],
    p_i_remain[PRISM_NUM_DIMS],
    i_comm_size, /* holds # nodes that do collective ops over with MPI */
#endif
    i_use_rows, /* number of rows to assign to 2D topology */
    i_use_cols, /* number of columns to assign to 2D topology */
    i_2dcomm_rows,     /* # rows in 2D comm used */
    i_2dcomm_cols,     /* # cols in 2D comm used */
    i_2d_row,          /* row coord in 2D comm used */
    i_2d_col,          /* column coord in 2D comm used */
    i_node_me, /* your location in list to skew */
    i_msg_elements, /* loop index for elements */
    i_elements_chk, /* # elements to check message for */
    i_start_send, /* what # array elements to start sending */
    i_stop_send, /* max # array elements to send */
    i_trials, /* # of trials to average results over */
    i_world_id,        /* rank in MPI_COMM_WORLD */
    i_l1 /* loop counter */
      ;

#if PRISM_NX
  long
    l_msh_rows, /* for call to mypart */
    l_msh_cols /* for call to mypart */
      ;
#endif

  double
    d_max_time, /* stores max time of all nodes in comm */
    d_min_time, /* stores min time of all nodes in comm */
    d_avg_time, /* stores average time of all nodes in comm */
    d_aux_time, /* aux variable for reduce operations */
    d_cktime, /* time overhead for one call to clock */
    d_st1, /* start of timing #1 */
    d_et1, /* end of timing #1 */
    d_mult_send, /* multiplicative factor to use between sending steps */
    d_add_send, /* addative factor to use between sending steps */
    d_tout, /* output for plot - either time or Mbytes/sec */
    d_tt1 /* total time for timing #1 */
      ;

  char
#if PRISM_TEST_SKEW
    *p_c_recv, /* pointer to buffer to recv into */
#endif
    *p_c_send, /* pointer to buffer to send */
    *p_c_got, /* holder of pointer that received data for collective op */
    c_tmpname[128], /* holds file name */
    c_error[120] /* holds error message for generror */
      ;

  FILE
    *outfile, /* file with statistical times and general output */
    *outfilet, /* file with readable output and plotting giving times */
    *outfilem /* file with readable output and plotting giving Mbytes/sec */
      ;

  PRISM_E_DATATYPE
    e_datatype
      ;

#if !PRISM_NX
  MPI_Datatype
    x_datatype /* datatype of message */
      ;
#endif

#if PRISM_MPI_COLL && PRISM_TEST_SKEW
  MPI_Status
    x_status /* status for sendrecv */
      ;
#endif

#if PRISM_NX
  PRISM_S_PROCGRP
    s_grp_use /* structure with nodes to do collective op over for PRISM */
      ;
#else
  MPI_Comm
    comm_use, /* 2D cartesian topology to use */
    comm_col, /* column of 2D topology */
    comm_row, /* row of 2D topology */
    comm_direction /* set to right communicator for row or column bcast */
      ;
#endif

  /* ------------------ */
  /* External functions */
  /* ------------------ */
#if PRISM_NX && PRISM_MEIKO
  extern double dclock();
#endif

#if PRISM_TEST_SKEW
#else
#endif
  extern void prism_v_init_msg(void *p_v_msg, int i_len_elements,
			       PRISM_E_DATATYPE e_datatype, int i_node);
  extern void prism_v_chk_msg(void *p_v_msg, int i_len_elements,
			      PRISM_E_DATATYPE e_datatype, int i_node,
			      FILE *outfile);
#if PRISM_NX
#else
#endif  

#if PRISM_NX
#if PRISM_MEIKO
  mpsc_init();
#endif
#else
  MPI_Init(&argc, &argv);
#endif

  /* initialize global variables */
  prism_v_init_var();

#if PRISM_NX
  i_world_id = prism_i_nx_id;
#else
  MPI_Comm_rank(MPI_COMM_WORLD, &i_world_id);
#endif

  if (argc > Prism_NUM_ARGS + 1) {
    sprintf(c_error, "prism_test: got %d args but max expect %d",
	    argc, Prism_NUM_ARGS + 1);
    prism_v_generror(c_error, brief);
  }    
#if PRISM_NX
  /* read arguments */
  for (i_l1 = 1; i_l1 < argc; i_l1++) {
    p_i_args[i_l1-1] = atoi(argv[i_l1]);
  }
  /* set rest so will default value */
  for (i_l1 = argc; i_l1 <= Prism_NUM_ARGS; i_l1++) {
    p_i_args[i_l1-1] = -99;
  }
  /* since p_i_args[8] can be any int, reset value to default if not
     given */
  if (argc <= 9) {
    p_i_args[8] = Prism_DFLT_OFFSET;
  }
#else
  /* do for p4, should work for others too */
  if (i_world_id == 0) {
    /* only read args on node 0 */
    for (i_l1 = 1; i_l1 < argc; i_l1++) {
      p_i_args[i_l1-1] = atoi(argv[i_l1]);
    }
    /* set rest so will default value */
    for (i_l1 = argc; i_l1 <= Prism_NUM_ARGS; i_l1++) {
      p_i_args[i_l1-1] = -99;
    }
    /* since p_i_args[8] can be any int, reset value to default if not
       given */
    if (argc <= 9) {
      p_i_args[8] = Prism_DFLT_OFFSET;
    }
  }
  /* broadcast args on node 0 to all other nodes */
  MPI_Bcast(p_i_args, Prism_NUM_ARGS, MPI_INT, 0, MPI_COMM_WORLD);
  /* also send # of args received */
  MPI_Bcast(&argc, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

  /* datatype to use */
  if (p_i_args[0] >= 0 && p_i_args[0] <= 11) {
    e_datatype = p_i_args[0];
  }
  else {
    e_datatype = Prism_DFLT_DATATYPE;
  }
#if !PRISM_NX
  x_datatype = prism_x_datatype(e_datatype);
#endif

  /* # min message size */
  if (p_i_args[1] > 0) {
    i_start_send = p_i_args[1];
  }
  else {
    i_start_send = Prism_DFLT_MSG_MIN;
  }

  /* max message size */
  if (p_i_args[2] > 0) {
    i_stop_send = p_i_args[2];
  }
  else {
    i_stop_send = Prism_DFLT_MSG_MAX;
  }

  /* for the next two, the next message size (nms) is gotten from the 
     previous message size (pms) by:
     nms = pms * (1.0 + d_mult_send / 100.0) + d_add_send */
  if (p_i_args[3] > 0) {
    d_mult_send = 1.0 + (double)p_i_args[3] / 100.0;
  }
  else {
    d_mult_send = 1.0 + (double)Prism_DFLT_MSG_FACTOR / 100.0;
  }
  /* msg step - addative value */
  if (p_i_args[4] >= 0) {
    d_add_send = (double)p_i_args[4];
  }
  else {
    d_add_send = (double)Prism_DFLT_MSG_STEP;
  }

  /* # trials to average over */
  if (p_i_args[5] > 0) {
    i_trials = p_i_args[5];
  }
  else {
    i_trials = Prism_DFLT_TRIALS;
  }

  /* # trials for clock testing */
  if (p_i_args[6] >= 0) {
    i_clock_trials = p_i_args[6];
  }
  else {
    i_clock_trials = i_trials;
  }    

  /* decide if horizontal (= 0), vertical (> 0) or default (<= 0) */
  if (p_i_args[7] >= 0) {
    i_dir = p_i_args[7];
  }
  else {
    i_dir = Prism_DFLT_DIRECTION;
  }

  /* set offset for bcast or skew */
  i_offset = p_i_args[8];

  /* If >= 0, then add (senders rank * p_i_args[8]) to number of elements to
     send in skew.  Default if < 0.  Ignored in broadcast (made 0) */
#if PRISM_TEST_SKEW
  if (p_i_args[9] >= 0) {
    i_vary = p_i_args[9];
  }
  else {
    i_vary = Prism_DFLT_VARY;
  }
#else
  i_vary = 0;
#endif

  /* if > 0, then # rows to use in 2D topology */
  if (p_i_args[10] > 0) {
    i_use_rows = p_i_args[10];
  }
  else {
    i_use_rows = 0;
  }

  /* If > 0, then # columns to use in 2D topology */
  if (p_i_args[11] > 0) {
    i_use_cols = p_i_args[11];
  }
  else {
    i_use_cols = 0;
  }

#if PRISM_NX
#if PRISM_MEIKO
  l_msh_rows = (long)numnodes();
  l_msh_cols = 1;
#else
  mypart(&l_msh_rows, &l_msh_cols); 
#endif
  /* if either i_use_rows or i_use_cols = 0, then use default */
  if (i_use_rows * i_use_cols == 0) {
    /* number of rows & cols in physical mesh */
    i_2dcomm_rows = (int)l_msh_rows;
    i_2dcomm_cols = (int)l_msh_cols;
  }
  /* make sure that total number of nodes asked for is same as
     actual number of nodes allociated */
  else if (i_use_rows * i_use_cols == (int)(l_msh_rows * l_msh_cols)) {
    i_2dcomm_rows = i_use_rows;
    i_2dcomm_cols = i_use_cols;
  }
  else {
    sprintf(c_error, "prism_test: request # rows %d * # cols %d != # nodes %d\n",
	    i_use_rows, i_use_cols, l_msh_rows * l_msh_cols);
    prism_v_generror(c_error, brief);
  }

  /* coordinates of node */
  i_2d_row = prism_i_nx_id / i_2dcomm_cols;
  i_2d_col = prism_i_nx_id % i_2dcomm_cols;

  i_node_id = prism_i_nx_id;
#else
  MPI_Comm_size(MPI_COMM_WORLD, &i_comm_size);
  /* if either i_use_rows or i_use_cols = 0, then make both zero so get
     default for both */
  if (i_use_rows * i_use_cols == 0) {
    p_i_dims[0] = 0;
    p_i_dims[1] = 0;
    MPI_Dims_create(i_comm_size, (int)PRISM_NUM_DIMS, p_i_dims);
  }
  /* make sure that total number of nodes asked for is no more than the
     actual number of nodes allociated */
  else if (i_use_rows * i_use_cols <= i_comm_size) {
    p_i_dims[0] = i_use_rows;
    p_i_dims[1] = i_use_cols;
  }
  else {
    sprintf(c_error, "prism_test: request # rows %d * # cols %d > # nodes %d\n",
	    i_use_rows, i_use_cols, i_comm_size);
    prism_v_generror(c_error, brief);
  }
  i_2dcomm_rows = p_i_dims[0];
  i_2dcomm_cols = p_i_dims[1];

  /* make into 2D grid/torus.
     The first dimension is logically the rows and the second
     dimension is the columns.  Want to get a wrap (torus) when
     going down a column which means marking the first dimension
     as periodic.  This is because we roll in the columns of the
     2D topology. */
  p_i_periods[0] = 1;
  p_i_periods[1] = 0;
  MPI_Cart_create(MPI_COMM_WORLD, (int)PRISM_NUM_DIMS, p_i_dims, p_i_periods,
		  (int)1, &comm_use);
  /* some processes might not be in topology */
  if (comm_use != MPI_COMM_NULL) {
    MPI_Comm_rank(comm_use, &i_node_id);
    MPI_Cart_coords(comm_use, i_node_id, (int)PRISM_NUM_DIMS, p_i_coords);
    i_2d_row = p_i_coords[0];
    i_2d_col = p_i_coords[1];

    /* make comm of rows of 2D topology */
    p_i_remain[0] = 0;
    p_i_remain[1] = 1;
    MPI_Cart_sub(comm_use, p_i_remain, &comm_row);

    /* make comm of columns of 2D topology */
    p_i_remain[0] = 1;
    p_i_remain[1] = 0;
    MPI_Cart_sub(comm_use, p_i_remain, &comm_col);
#endif

#if PRISM_NX
    /* create structure for PRISM collective functions to use */
    s_grp_row.i_num = i_2dcomm_cols;
    s_grp_row.p_i_grp = (int *)malloc((size_t)(s_grp_row.i_num * sizeof(int)));

    s_grp_col.i_num = i_2dcomm_rows;
    s_grp_col.p_i_grp = (int *)malloc((size_t)(s_grp_col.i_num * sizeof(int)));

    for (i_l1 = 0; i_l1 < s_grp_row.i_num; i_l1++) {
      s_grp_row.p_i_grp[i_l1] = i_2d_row * i_2dcomm_cols + i_l1;
    }
    for (i_l1 = 0; i_l1 < s_grp_col.i_num; i_l1++) {
      s_grp_col.p_i_grp[i_l1] = i_l1 * i_2dcomm_cols + i_2d_col;
    }
#endif

    if (i_dir > 0) { /* vert collective op */
      i_node_me = i_2d_row;
#if PRISM_NX
      s_grp_use = s_grp_col;
#else
      comm_direction = comm_col;
#endif
#if PRISM_TEST_SKEW
#if PRISM_MPI_COLL
      i_node_send = (i_2d_row + i_2d_col + i_offset) % i_2dcomm_rows;
      if (i_node_send < 0) {
	i_node_send += i_2dcomm_rows;
      }
#else
      i_dist_skew = i_2d_col + i_offset;
#endif
      i_node_recv = (i_2d_row - i_2d_col - i_offset) % i_2dcomm_rows;
      if (i_node_recv < 0) {
	i_node_recv += i_2dcomm_rows;
      }
#else
      i_node_bc = (i_2d_col + i_offset) % i_2dcomm_rows; 
      if (i_node_bc < 0) {
	i_node_bc += i_2dcomm_rows;
      }
#endif
    }
    else { /* horz bcast */
      i_node_me = i_2d_col;
#if PRISM_NX
      s_grp_use = s_grp_row;
#else
      comm_direction = comm_row;
#endif

#if PRISM_TEST_SKEW
#if PRISM_MPI_COLL
      i_node_send = (i_2d_col + i_2d_row + i_offset) % i_2dcomm_cols;
      if (i_node_send < 0) {
	i_node_send += i_2dcomm_cols;
      }
#else
      i_dist_skew = i_2d_row + i_offset;
#endif
      i_node_recv = (i_2d_col - i_2d_row - i_offset) % i_2dcomm_cols;
      if (i_node_recv < 0) {
	i_node_recv += i_2dcomm_cols;
      }
#else
      i_node_bc = (i_2d_row + i_offset) % i_2dcomm_cols; 
      if (i_node_bc < 0) {
	i_node_bc += i_2dcomm_cols;
      }
#endif
    }

    /* output file for each node */
    /* note that the files are compatible with xgraph */

    /* open output file for summary.  Only do for node 0 in each row/column
       communicator that using */
#if PRISM_OUTTERM
    outfile = stderr;
#else
    if (i_node_me == 0) {
      sprintf(c_tmpname, PRISM_F_NAME_OUT, i_2d_row, i_2d_col);
      outfile = fopen(c_tmpname, "w");
      if (outfile == NULL ) {
	sprintf(c_error, "prism_test: cannot open file %s", PRISM_F_NAME_OUT);
	prism_v_generror(c_error, brief);
      }
    }
    else {
      sprintf(c_tmpname, PRISM_F_NAME_ERR, i_2d_row, i_2d_col);
      outfile = fopen(c_tmpname, "w");
      if (outfile == NULL ) {
	sprintf(c_error, "prism_test: cannot open file %s", PRISM_F_NAME_ERR);
	prism_v_generror(c_error, brief);
      }
    }
#endif
#if PRISM_OUT_TIME
    sprintf(c_tmpname, Prism_F_NAME_OUTT, i_node_id);
    outfilet = fopen(c_tmpname, "w");
    if (outfilet == NULL ) {
      sprintf(c_error, "prism_test: cannot open file %s", Prism_F_NAME_OUTT);
      prism_v_generror(c_error, brief);
    }
#endif
#if PRISM_OUT_MBPS
    sprintf(c_tmpname, Prism_F_NAME_OUTM, i_node_id);
    outfilem = fopen(c_tmpname, "w");
    if (outfilem == NULL ) {
      sprintf(c_error, "prism_test: cannot open file %s", Prism_F_NAME_OUTM);
      prism_v_generror(c_error, brief);
    }
#endif

    if (i_node_me == 0) {
      fprintf(outfile, "%% SUMMARY FILE FOR NODE %d (world node %d) , row %d, col %d\n\n",
	      i_node_id, i_world_id, i_2d_row, i_2d_col);
    }

#if PRISM_OUT_TIME
    fprintf(outfilet, "# TIME FILE FOR NODE %d (world node %d)\n\n",
	    i_node_id, i_world_id);
#endif
#if PRISM_OUT_MBPS
    fprintf(outfilem, "# MBPS FILE FOR NODE %d (world node %d)\n\n",
	    i_node_id, i_world_id);
#endif

    /* output values used from paramenters or defaults */
    if (i_node_me == 0) {
      fprintf(outfile, "%% parameters:\n");
      fprintf(outfile, "%%\t\tdatatype\tmin msg\tmax msg\tmult factor\tadd step\n");
      fprintf(outfile, "%%\t\t--------\t-------\t-------\t-----------\t--------\n");
      fprintf(outfile, "%% value used:\t%d\t\t%d\t%d\t%f\t%f\n",
	      e_datatype, i_start_send, i_stop_send, d_mult_send, d_add_send);
      fprintf(outfile, "%%\n");
      fprintf(outfile, "%%\t\t# trials\t# clock trials\tdirection\toffset\tvary\n");
      fprintf(outfile, "%%\t\t--------\t---------------\t--------\t------\t----\n");
      fprintf(outfile, "%% value used:\t%d\t\t%d\t\t%d\t\t%d\t%d\n",
	      i_trials, i_clock_trials, i_dir, i_offset, i_vary);
      fprintf(outfile, "%%\n");
      fprintf(outfile, "%%\t\t# rows\t\t# cols\n");
      fprintf(outfile, "%%\t\t------\t\t------\n");
      fprintf(outfile, "%% value used:\t%d\t\t%d\n",
	      i_use_rows, i_use_cols);
      fprintf(outfile, "%%\n");

      if ((argc - 1) < Prism_NUM_ARGS) {
	/* not all values input, output extra info */
	fprintf(outfile, "%% DFLT values:\t%d\t\t%d\t%d\t%f\t%f\n",
		Prism_DFLT_DATATYPE, Prism_DFLT_MSG_MIN, Prism_DFLT_MSG_MAX,
		1.0 + (double)Prism_DFLT_MSG_FACTOR / 100.0,
		(double)Prism_DFLT_MSG_STEP);
	fprintf(outfile, "%%\t\t%d\t\t# trials\t%d\t\t%d\t%d\n", Prism_DFLT_TRIALS,
		Prism_DFLT_DIRECTION, Prism_DFLT_OFFSET, Prism_DFLT_VARY);
	fprintf(outfile, "%%\t\tsee below\n");
	fprintf(outfile, "\n%% default values used if no input or < (<=) 0\n\n");

	fprintf(outfile, "%% datatype to use:\n");
	fprintf(outfile, "%%\tprism_byte = %d\n%%\tprism_char = %d\n%%\tprism_double = %d\n%%\tprism_float = %d\n%%\tprism_int = %d\n%%\tprism_long = %d\n%%\tprism_long_double = %d\n%%\tprism_short = %d\n%%\tprism_unsigned = %d\n%%\tprism_unsigned_char = %d\n%%\tprism_unsigned_long = %d\n%%\tprism_unsigned_short = %d\n",
		prism_byte, prism_char, prism_double, prism_float, prism_int,
		prism_long, prism_long_double, prism_short, prism_unsigned,
		prism_unsigned_char, prism_unsigned_long, prism_unsigned_short);
	fprintf(outfile, "%% min msg: message size in elements to start looping at\n");
	fprintf(outfile, "%% max msg: message size in elements not to exceed\n");
	fprintf(outfile,
		"%% factor, step: take previous message size * (1.0 + factor / 100.0)\n");
	fprintf(outfile,
		"%%               + step to get next value\n");
	fprintf(outfile,
		"%% number of trials: how many collective ops to average time over\n");
	fprintf(outfile,
		"%% number of clock trials: number of times to average to determine clock overhead\n");
	fprintf(outfile,
		"%% direction: = 0 -> horz, ie, within rows, otherwise vertical\n");
	fprintf(outfile, "%% offset: value to shift by\n");
#if PRISM_TEST_SKEW
	fprintf(outfile,
		"%%    if doing a vert skew, then each column j skews (j + offset)\n");
	fprintf(outfile,
		"%%    if doing a horz skew, then each row i skews (i + offset)\n");
#else
	fprintf(outfile,
		"%%    if doing a vert bcast, then each column j chooses the (j + offset) % # rows\n");
	fprintf(outfile,
		"%%    if doing a horz bcast, then each row i chooses the \n%%       (i + offset) %% no. cols\n");
#endif
	fprintf(outfile, "%% if doing skew them multiply this value (if >= 0)\n%%  by senders rank to add to # of bytes sent.\n%%  Ignored (made 0) in broadcast\n");
	fprintf(outfile, "%% # of rows: number of rows to have in 2D topology\n");
#if PRISM_NX
	fprintf(outfile, "%%     defaults to physical partition in nx if either row or col 0\n");
#else
	fprintf(outfile, "%%     defaults to value returned by MPI_Dims_create in MPI \n%%     if either row or col 0\n");
#endif
	fprintf(outfile, "%% # of cols: like rows above but for columns\n");
      }
      fprintf(outfile, "mach = \'%s\';\n", Prism_STR_MACHINE);
      fprintf(outfile, "params = [%d %d %d %e %e %d %d %d %d %d %d %d];\n",
	      e_datatype, i_start_send, i_stop_send, d_mult_send, d_add_send,
	      i_trials, i_clock_trials, i_dir, i_offset, i_vary,
	      i_2dcomm_rows, i_2dcomm_cols);
    }

    /* estimate time for calling clock */

    d_cktime = 0.0;
    for (i_l1 = 0; i_l1 < i_clock_trials; i_l1++) {
      i_msg_send_elements = 0;
#if PRISM_TEST_SKEW
      /* perform skew */
#if PRISM_MPI_COLL
      MPI_Sendrecv(&i_t1, i_msg_send_elements, MPI_INT, i_node_send, (int)13,
		   &i_t2, i_msg_send_elements, MPI_INT, i_node_recv, (int)13,
		   comm_direction, &x_status);
#else
#if PRISM_NX
      prism_v_skew(&i_t1, &i_t2, i_msg_send_elements, i_msg_send_elements,
		   prism_int, i_dist_skew, s_grp_use);
#else
      prism_v_skew(&i_t1, &i_t2, i_msg_send_elements, i_msg_send_elements,
		   MPI_INT, i_dist_skew, comm_direction);
#endif
#endif
#else
      /* perform broadcast */
#if PRISM_MPI_COLL
      MPI_Bcast(&i_t1, i_msg_send_elements, MPI_INT, i_node_bc,
		comm_direction);
#else
#if PRISM_NX
      prism_v_bc(&i_t1, i_msg_send_elements, prism_int, i_node_bc, s_grp_use);
#else
      prism_v_bc(&i_t1, i_msg_send_elements, MPI_INT, i_node_bc,
		 comm_direction);
#endif
#endif
#endif
      /* start and stop clock */
#if PRISM_NX
      d_st1 = dclock();
      d_et1 = dclock();
#else
      d_st1 = MPI_Wtime();
      d_et1 = MPI_Wtime();
#endif
      d_cktime += d_et1 - d_st1;
    }
    if (i_clock_trials > 0) {
      d_cktime = d_cktime / i_clock_trials;
    }

    if (i_node_me == 0) {
      fprintf(outfile, "\n%% estimated time for clock = %e\n", d_cktime);
#if !PRISM_NX
      fprintf(outfile, "%% MPI clock resolution = %e\n\n", MPI_Wtick());
#endif
    }

    if (i_node_me == 0) {
      /*    fprintf(outfile, "TitleText: %s (%d,%d) %s/%s/%s/%s/dir %d/datatype %d/%d trials\nMarkers: false\nXUnitText: message length (bytes)\nYUnitText: %s\n",
	    Prism_STR_MACHINE, i_2dcomm_rows, i_2dcomm_cols, Prism_ROUTINE,
	    Prism_COLL, Prism_WAYCOMM, Prism_WAYGSYNC, i_dir, e_datatype,
	    i_trials, "AVE TIME (sec)");*/
      fprintf(outfile, "routine = \'%s\';\n", Prism_ROUTINE);
      fprintf(outfile, "coll = \'%s\';\n", Prism_COLL);
      fprintf(outfile, "waycomm = \'%s\';\n", Prism_WAYCOMM);
      fprintf(outfile, "waygsync = \'%s\';\n", Prism_WAYGSYNC);
      fprintf(outfile, "\n");
      fprintf(outfile, "times = [\n");
      fprintf(outfile, "%% bytes     max          min       avg\n");
    }
#if PRISM_OUT_TIME
    fprintf(outfilet, "TitleText: %s (%d,%d) %s/%s/%s/%s/dir %d/datatype %d/%d trials\nMarkers: false\nXUnitText: message length (bytes)\nYUnitText: %s\n\"node %d\"\n",
	    Prism_STR_MACHINE, i_2dcomm_rows, i_2dcomm_cols, Prism_ROUTINE,
	    Prism_COLL, Prism_WAYCOMM, Prism_WAYGSYNC, i_dir, e_datatype,
	    i_trials, "TIME (sec)", i_node_id);
#endif
#if PRISM_OUT_MBPS
    fprintf(outfilem, "TitleText: %s (%d,%d) %s/%s/%s/%s/dir %d/datatype %d/%d trials\nMarkers: false\nXUnitText: message length (bytes)\nYUnitText: %s\n\"node %d\"\n",
	    Prism_STR_MACHINE, i_2dcomm_rows, i_2dcomm_cols, Prism_ROUTINE,
	    Prism_COLL, Prism_WAYCOMM, Prism_WAYGSYNC, i_dir, e_datatype,
	    i_trials, "Mbytes/sec", i_node_id);
#endif

    /* loop over different message lengths */
    /* check that won't enter infinite loop */
    if ((int)(i_start_send * d_mult_send + d_add_send) <= i_start_send) {
      sprintf(c_error, "prism_test: i_start_send = %d times d_mult_send = %f plus step = %f will cause infinite loop", i_start_send, d_mult_send, d_add_send);
      prism_v_generror(c_error, brief);
    }
    for (i_msg_elements = i_start_send; i_msg_elements <= i_stop_send;
	 i_msg_elements = i_msg_elements * d_mult_send + d_add_send) {

#if PRISM_TEST_SKEW
      i_msg_send_elements = i_msg_elements + i_vary * i_node_me;
#else
      i_msg_send_elements = i_msg_elements;
#endif
      
      i_msg_send_bytes = prism_i_datatype_size(e_datatype)
	* i_msg_send_elements;
    
      p_c_send = (char *)malloc((size_t)i_msg_send_bytes);
      if (p_c_send == NULL) { 
	/*      fprintf (outfile, "***error***prism_test: failed to get memory for msg_send of size %d\n", i_msg_send_bytes);*/
	sprintf(c_error, "prism_test: failed to get memory for msg_send of size %d", i_msg_send_bytes);
	prism_v_generror(c_error, brief);
      }

#if PRISM_TEST_SKEW
      i_msg_recv_elements = i_msg_elements + i_vary * i_node_recv;
      i_msg_recv_bytes = prism_i_datatype_size(e_datatype)
	* i_msg_recv_elements;
      p_c_recv = (char *)malloc((size_t)i_msg_recv_bytes);
      if (p_c_recv == NULL) { 
	sprintf(c_error, "prism_test: failed to get memory for msg_recv of size %d", i_msg_recv_bytes);
	prism_v_generror(c_error, brief);
      }
#endif

      d_tt1 = 0.0;
#if PRISM_GSYNC1
#if PRISM_NX
      gsync(); 
#else
      MPI_Barrier(comm_use);
#endif
#endif
      for (i_l1 = 0; i_l1 < i_trials; i_l1++) {
	/* initialize data */
	prism_v_init_msg (p_c_send, i_msg_send_elements, e_datatype,
			i_node_id + i_trials);
#if PRISM_GSYNCE
#if PRISM_NX
	gsync(); 
#else
	MPI_Barrier(comm_use);
#endif
#endif
#if PRISM_NX
	d_st1 = dclock();
#else
	d_st1 = MPI_Wtime();
#endif

#if PRISM_TEST_SKEW
	/* perform skew */
#if PRISM_MPI_COLL
	MPI_Sendrecv(p_c_send, i_msg_send_elements, x_datatype, i_node_send,
		     (int)13, p_c_recv, i_msg_recv_elements, x_datatype,
		     i_node_recv, (int)13, comm_direction, &x_status);
  
#else
#if PRISM_NX
	prism_v_skew(p_c_send, p_c_recv, i_msg_send_elements, 
		     i_msg_recv_elements, e_datatype, i_dist_skew, s_grp_use);
#else
	prism_v_skew(p_c_send, p_c_recv, i_msg_send_elements, 
		     i_msg_recv_elements, x_datatype, i_dist_skew,
		     comm_direction);
#endif
#endif
#else
	/* perform broadcast */
#if PRISM_MPI_COLL
	MPI_Bcast(p_c_send, i_msg_send_elements, x_datatype, i_node_bc,
		  comm_direction);
#else
#if PRISM_NX
	prism_v_bc(p_c_send, i_msg_send_elements, e_datatype, i_node_bc,
		   s_grp_use);
#else
	prism_v_bc(p_c_send, i_msg_send_elements, x_datatype, i_node_bc,
		   comm_direction);
#endif
#endif
#endif

#if PRISM_NX
	d_et1 = dclock();
#else
	d_et1 = MPI_Wtime();
#endif
	d_tt1 += d_et1 - d_st1;
	/* check result of collective op */
#if PRISM_TEST_SKEW
	p_c_got = p_c_recv;
	i_recv = i_node_recv;
	i_elements_chk = i_msg_recv_elements;
#else
	p_c_got = p_c_send;
	i_recv = i_node_bc;
	i_elements_chk = i_msg_send_elements;
#endif
	if (i_dir > 0) { /* vertical */
	  i_recv = i_2d_col + i_2dcomm_cols * i_recv;
	}
	else { /* horz */
	  i_recv = i_recv + i_2dcomm_cols * i_2d_row;
	}
	prism_v_chk_msg (p_c_got, i_elements_chk, e_datatype, i_recv + i_trials,
			 outfile);
      } /* end for of # trials */

      /* free memory for message */
      free(p_c_send);
#if PRISM_TEST_SKEW
      free(p_c_recv);
#endif
      d_tt1 /= i_trials - d_cktime;

      /* output results */
      /* barrier on you comm then all node to try and minimize the effect
	 of outputting to files on times of nodes that might not be done yet */
#if PRISM_NX
      gsync();
#else
      MPI_Barrier(comm_direction);
      MPI_Barrier(comm_use);
#endif
      /* determine max, min and average time for all nodes in you comm */
      d_max_time = d_min_time = d_avg_time = d_tt1;
#if PRISM_NX
      gdhigh(&d_max_time, 1, &d_aux_time);
      gdlow(&d_min_time, 1, &d_aux_time);
      gdsum(&d_avg_time, 1, &d_aux_time);
      d_avg_time /= s_grp_use.i_num;
#else
      MPI_Reduce(&d_max_time, &d_aux_time, (int)1, MPI_DOUBLE, MPI_MAX,
		 (int)0, comm_direction);
      d_max_time = d_aux_time;
      MPI_Reduce(&d_min_time, &d_aux_time, (int)1, MPI_DOUBLE, MPI_MIN,
		 (int)0, comm_direction);
      d_min_time = d_aux_time;
      MPI_Reduce(&d_avg_time, &d_aux_time, (int)1, MPI_DOUBLE, MPI_SUM,
		 (int)0, comm_direction);
      d_avg_time = d_aux_time;
      MPI_Comm_size(comm_direction, &i_comm_size);
      d_avg_time /= i_comm_size;
#endif
      if (i_node_me == 0) {
#if PRISM_TEST_SKEW
	fprintf(outfile, "  %d  %e %e %e\n",
		(i_msg_send_bytes + i_msg_recv_bytes), d_max_time, d_min_time,
		d_avg_time);
#else
	fprintf(outfile, "  %d %e %e %e\n", i_msg_send_bytes,
		d_max_time, d_min_time, d_avg_time);
#endif
      }

#if PRISM_OUT_MBPS
#if PRISM_TEST_SKEW
      /* for skew, count total bits on and off node to determine number
	 of bytes sent */
      d_tout = ((double)(i_msg_send_bytes + i_msg_recv_bytes)) /
	(d_tt1 * pow((double)2, (double)20));
#else
      /* for broadcast, assume number of bytes is the number braodcast times
	 number of stages in a log_2 tree. */
#if PRISM_NX
      d_tout = ((i_msg_send_bytes 
		 * ceil((log10((double)(s_grp_use.i_num))
			 / log10((double)2.0))))
		/ (d_tt1 * pow((double)2, (double)20)));
#else
      d_tout = ((i_msg_send_bytes 
		 * ceil((log10((double)i_comm_size)
			 / log10((double)2.0))))
		/ (d_tt1 * pow((double)2, (double)20)));
#endif /* on prism_nx */
#endif /* on prism_test_skew */
#if PRISM_TEST_SKEW
      fprintf(outfilem, "%d  %e\n", (i_msg_send_bytes + i_msg_recv_bytes),
	      d_tout);
#else
      fprintf(outfilem, "%d  %e\n", i_msg_send_bytes, d_tout);
#endif
#endif /* on prism_out_mbps */
 
#if PRISM_OUT_TIME
      d_tout = d_tt1;
#if PRISM_TEST_SKEW
      fprintf(outfilet, "%d  %e\n", (i_msg_send_bytes + i_msg_recv_bytes),
	      d_tout);
#else
      fprintf(outfilet, "%d  %e\n", i_msg_send_bytes, d_tout);
#endif
#endif
    
    } /* end for over message sizes */

    if (i_node_me == 0) {
      fprintf(outfile, "];\n");
      fprintf(outfile, "comm_stats\n");
      fprintf(outfile, "case=case+1;\n");
      fprintf(outfile, "\n%% normal termination\n");
  }
#ifndef PRISM_OUTTERM
    if (i_node_me == 0) {
      fclose(outfile);
    }
#endif
#if PRISM_OUT_TIME
    fclose(outfilet);
#endif
#if PRISM_OUT_MBPS
    fclose(outfilem);
#endif

#if !PRISM_NX
  } /* end if on comm_use != MPI_COMM_NULL */
#endif

/* getting too much screen output, supress for now */
/*  printf("node = %d is done\n", i_node_id);*/

#if !PRISM_NX
  MPI_Finalize();
#endif

}
