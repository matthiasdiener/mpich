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
   Function to initialize the global variables 
*/
/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"
#include <math.h>
#include <stdio.h>
#ifndef PRISM_MPI_BCAST
#include <stddef.h>
#endif

#if !PRISM_NX
#define NUM_DIMS 2
#endif

void prism_v_init_var()

{
  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
#if !PRISM_NX
    i_max_num,
    *p_i_ranks,
    i_topo_status,
    p_i_dims[NUM_DIMS],
    i_topo_dims,
    p_i_periods[NUM_DIMS],
    p_i_coords[NUM_DIMS],
    p_i_remain[NUM_DIMS],
    i_comm_size,
    i_comm_rank,
#endif
    i_l1
      ;

#if PRISM_NX
  long
    l_msh_rows,
    l_msh_cols
      ;
#else
  E_BOOLEAN
    e_remap
      ;
#endif

  static E_BOOLEAN e_init_var = false;

  /* ------------------ */
  /* External functions */
  /* ------------------ */
#if PRISM_NX
  extern PRISM_V_GENERRAUX;
#endif

  switch (e_init_var) {
  case true:  /* if called before then quick return */
    break;
  case false:
    e_init_var = true;

#if PRISM_NX
    i_node_id = (int)mynode();

    /* number of rows & cols in physical mesh */
    mypart(&l_msh_rows, &l_msh_cols); 
    i_msh_rows = (int)l_msh_rows;
    i_msh_cols = (int)l_msh_cols;

    /* coordinates of node */
    i_node_row = i_node_id / i_msh_cols;
    i_node_col = i_node_id % i_msh_cols;
#else
    /* see if need to initialize MPI */
    MPI_Initialized(&i_initialized);
    if (!i_initialized) {
      MPI_Init(&my_argc, &my_argv);
    }

  /* *** make communicator to start with be world - this will be replace by
     value passed by caller in future *** */
    x_comm_use0 = MPI_COMM_WORLD;

    /* get rank of your node in the comm given to use */
    MPI_Comm_rank(x_comm_use0, &i_comm_rank);
    i_node_id = (long)i_comm_rank;

    /* need a 2D cartesian topology where the columns
       are wrapped.  If given this, then simply dup, else create new
       topology */
    MPI_Topo_test(x_comm_use0, &i_topo_status);
    e_remap = true;
    if (i_topo_status == MPI_CART) {
      /* make sure it is a 2D topology with wrap in columns */
      MPI_Cartdim_get(x_comm_use0, &i_topo_dims);
      if (i_topo_dims == NUM_DIMS) {
	MPI_Cart_get(x_comm_use, (int)NUM_DIMS, p_i_dims, p_i_periods,
		     p_i_coords);
	if (p_i_periods[1] == 1) {
	  MPI_Comm_dup(x_comm_use0, &x_comm_use);
	  e_remap = false;
	}
      }
    }
    if (e_remap == true) {
      /* make into 2D grid/torus.
	 The first dimension is logically the rows and the second
	 dimension is the columns.  Want to get a wrap (torus) when
	 going down a column which means marking the first dimension
	 as periodic.  This is because we roll in the columns of the
	 2D topology. */
      MPI_Comm_size(x_comm_use0, &i_comm_size);
      p_i_dims[0] = 0;
      p_i_dims[1] = 0;
      MPI_Dims_create(i_comm_size, (int)NUM_DIMS, p_i_dims);
      p_i_periods[0] = 1;
      p_i_periods[1] = 0;
      MPI_Cart_create(x_comm_use0, (int)NUM_DIMS, p_i_dims, p_i_periods,
		    (int)1, &x_comm_use);
    }

    i_msh_rows = (long)p_i_dims[0];
    i_msh_cols = (long)p_i_dims[1];

    MPI_Cart_coords(x_comm_use, i_comm_rank, (int)NUM_DIMS, p_i_coords);
    i_node_row = (long)p_i_coords[0];
    i_node_col = (long)p_i_coords[1];

    /* make comm of rows of 2D topology */
    p_i_remain[0] = 0;
    p_i_remain[1] = 1;
    MPI_Cart_sub(x_comm_use, p_i_remain, &x_comm_row);

    /* make comm of columns of 2D topology */
    p_i_remain[0] = 1;
    p_i_remain[1] = 0;

    MPI_Cart_sub(x_comm_use, p_i_remain, &x_comm_col);
#endif

#if PRISM_NX
    /* create structure for PRISM broadcast to use */
    s_grp_row.i_num = i_msh_cols;
    s_grp_row.p_i_grp = (int *)malloc((size_t)(s_grp_row.i_num * sizeof(int)));

    s_grp_col.i_num = i_msh_rows;
    s_grp_col.p_i_grp = (int *)malloc((size_t)(s_grp_col.i_num * sizeof(int)));

    for (i_l1 = 0; i_l1 < s_grp_row.i_num; i_l1++) {
      s_grp_row.p_i_grp[i_l1] = i_node_row * i_msh_cols + i_l1;
    }
    for (i_l1 = 0; i_l1 < s_grp_col.i_num; i_l1++) {
      s_grp_col.p_i_grp[i_l1] = i_l1 * i_msh_cols + i_node_col;
    }
#endif

    /* constant initializations */

    i_zero = 0;
    i_one = 1;
    r_zero = 0.0;
    r_one = 1.0;

#if PRISM_NX
    /* initialize a hrecv for run-time error handling */
    if (i_node_id == 0) {
      hrecv(ERRMSG_TYPE, errmsg_buf, sizeof errmsg_buf, prism_v_generraux);
    }
    gsync();
    break;
#endif
  }
  return;
}
