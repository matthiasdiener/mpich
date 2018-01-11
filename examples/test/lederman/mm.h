/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   PRISM routine (version 2.0)
   15 April 1994

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

/* PURPOSE
   =======
   contains type definitions and function prototype macro definitions for 
   matrix-matrix multiplication routines */

/* avoid multiple inclusion of this header file */
#ifndef MM_H  
#define MM_H  
#include <stdio.h>

/* GLOBAL TYPES */

typedef
  long 
  fint
  ;
typedef  
  double **        /* pointer to pointer to entry [0][0] */
  P_D_MATRIX       /* matrix of double */ 
  ;
typedef		   /* for now, real is equivalent to double */
  P_D_MATRIX
  P_R_MATRIX
  ;
typedef		   
  enum {false, true}
  E_BOOLEAN
  ;
typedef
  enum {brief, verbose} E_ERRTYPES
  ;

/* enumerated list of datatypes we support */
typedef
  enum {prism_byte, prism_char, prism_double, prism_float, prism_int,
	  prism_long, prism_long_double, prism_short, prism_unsigned,
	  prism_unsigned_char, prism_unsigned_long, prism_unsigned_short
        }
  PRISM_E_DATATYPE
  ;

#if PRISM_NX
/* structure to hold a list of nodes for use in our collective operations */
/* procgrp stands for process group */
typedef struct prism_s_procgrp {
  int
    i_num,                 /* number of entries in process group (nodes) */
    *p_i_grp              /* pointer to array with entries */
      ;
} PRISM_S_PROCGRP
;
#endif

/* GLOBAL VARIABLES */

int
  i_node_row,          /* your x-coordinate location in full mesh */
  i_node_col,          /* your y-coordinate location in full mesh */
  i_msh_rows,          /* number of rows in full mesh */
  i_msh_cols,          /* number of columns in full mesh */
  i_node_id,           /* your logical node number in full mesh */
  i_one,	       /* initialized to 1 to use in FORTRAN calls */
  i_zero	       /* initialized to 0 to use in FORTRAN calls */
  ;
double
  r_zero,	       /* initialized to 1.0 to use in FORTRAN calls */
  r_one		       /* initialized to 0.0 to use in FORTRAN calls */
  ;
char
  errmsg_buf[80]      /* buffer for print out run-time error msg */
  ;

#if PRISM_NX
PRISM_S_PROCGRP
  s_grp_col,
  s_grp_row
  ;
#endif

/* GLOBAL CONSTANTS */
#define ERRMSG_TYPE 1234567891   /* msg_type for hrecv */

/* FUNCTION PROTOTYPE DEFINITIONS */ 

#define PRISM_M_D_ALLOC_MATRIX double ** prism_m_d_alloc_matrix(int l_rows, \
								int l_cols)
#if PRISM_NX
#define PRISM_V_BC void prism_v_bc(void *p_v_send, int i_elements, \
				   PRISM_E_DATATYPE e_datatype, int i_root, \
				   PRISM_S_PROCGRP s_grp_use)
#else
#define PRISM_V_BC void prism_v_bc(void *p_v_send, int i_elements, \
				   MPI_Datatype x_datatype, int i_root, \
				   MPI_Comm x_comm_bc)
#endif

#if PRISM_NX
#define PRISM_V_SKEW void prism_v_skew (void * p_v_send, void * p_v_recv, \
           int i_s_elements, int i_r_elements,  PRISM_E_DATATYPE e_datatype, \
	   int i_skewstep, PRISM_S_PROCGRP s_grp_use)
#else
#define PRISM_V_SKEW void prism_v_skew (void * p_v_send, void * p_v_recv, \
           int i_s_elements, int i_r_elements,  MPI_Datatype x_datatype, \
	   int i_skewstep, MPI_Comm x_comm_skew)
#endif

#define PRISM_V_BIMMER void prism_v_bimmer(int i_sbmsh_nw_x, \
                                     int i_sbmsh_nw_y, \
				     int i_sbmsh_rows, \
				     int i_sbmsh_cols, int i_v_dim, \
				     int i_blk_row_nw, int i_blk_col_nw, \
				     int i_g_m, int i_g_k, int i_g_n, \
				     int i_panelwidth, int i_offset, \
				     int i_panel_spc, E_BOOLEAN e_transa, \
				     E_BOOLEAN e_transb, \
				     double r_alpha, P_R_MATRIX m_r_a, \
				     int i_lda, P_R_MATRIX m_r_b, \
				     int i_ldb, double r_beta, P_R_MATRIX \
				     m_r_c, int i_ldc, \
				     P_R_MATRIX m_r_bcbuf, P_R_MATRIX \
				     m_r_rollbuf, \
				     int (*i_lin_dim)(int i_v_dim, \
						       int i_g_dim, \
						       int i_blk_nw, \
						       int i_first_blk, \
						       int i_last_blk, \
						       int i_panelwidth, \
						       int i_offset, \
						       int i_panel_spc))
#define DCOPY void dcopy_(fint *n, double *dx, fint *incx, double *dy,\
			  fint *incy) 
#define DGEMM void dgemm_(char *TRANSA, char *TRANSB, fint *M, fint *N, fint *K,\
			  double *ALPHA, double *A, fint *LDA, double *B,\
			  fint *LDB, double *BETA, double *C, fint *LDC) 
#define DLACPY void dlacpy_(char *UPLO, fint *M, fint *N, double *A, fint *LDA,\
			    double *B, fint *LDB)
#define PRISM_V_FREE_MATRIX void prism_v_free_matrix(void ** p_name)
#define PRISM_V_GENERROR void prism_v_generror(char c_error_text[], \
					       E_ERRTYPES e_errtype)
#if PRISM_NX
#define PRISM_V_GENERRAUX void  prism_v_generraux(long type, long count, \
						  long node, long pid)
#endif
#define PRISM_V_INIT_VAR void prism_v_init_var()
#define PRISM_I_LDIM int prism_i_ldim(int i_v_dim, int i_g_dim, int i_blk_nw, \
				int i_first_blk,int i_last_blk, \
				int i_panelwidth, int i_offset, \
				int i_panel_spc)
#define PRISM_I_LDWK int i_ldwk(int i_v_dim, int i_g_dim, int i_blk_nw,\
			   int i_panelwidth, int i_offset, int i_panel_spc,\
			   int i_row_blks, int i_sbmsh_rows)
#define PRISM_V_FINISH void prism_v_finish()

#define PRISM_I_DATATYPE_SIZE int prism_i_datatype_size(PRISM_E_DATATYPE e_datatype)
#if !PRISM_NX
#define PRISM_X_DATATYPE MPI_Datatype prism_x_datatype(PRISM_E_DATATYPE e_datatype)
#endif

#if PRISM_NX
#define PRISM_V_COPY void prism_v_copy(void * p_v_input, void * p_v_output, \
				       int i_elements, \
				       PRISM_E_DATATYPE e_datatype)
#else
#define PRISM_V_COPY void prism_v_copy(void * p_v_input, void * p_v_output, \
				       int i_elements, MPI_Datatype x_datatype)
#endif  

#endif
