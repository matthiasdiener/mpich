/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

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
/* NOTE: we use <> for include files instead of "" because CPP includes
   the local copy if you use "" and this messes up our development work.
   Even thought this include file is in <> it is not a system file */
#include <stdeig.h>

/* GLOBAL TYPES */

typedef
  int 
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
typedef
  enum {wrap, block, wrap1d, block1d}
  E_LAYOUT
  ;
/* temporarily added so can compile code which reference old utility
   routines like fan_in */
typedef
  enum {north, east, south, west, horz, vert, clkwise, cclkwise} 
  E_TYPES 
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
#if PRISM_NX
  /* you only need the global id in nx.  It is used in routines, such as
     prism_v_bc where it identifies nodes and is used in the tag to make
     them unique.  MPI does not need or can get this information elsewhere.
     In nx it is static so ok to globally set; in MPI need to look at the
     communicator you are dealing with */
  prism_i_nx_id,       /* id of node */
#endif
  i_one,	       /* initialized to 1 to use in FORTRAN calls */
  i_zero	       /* initialized to 0 to use in FORTRAN calls */
  ;
double
#if PRISM_TIME
  prism_d_b0_time, prism_d_b1_time, prism_d_b_time, 
  prism_d_r0_time, prism_d_r1_time, prism_d_r_time,
  prism_d_m0_time, prism_d_m1_time, prism_d_m_time,
#endif
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

/* GLOBAL DEFINES */

#define MAX(x,y) ( x > y ? x : y)
#define MIN(x,y) ( x < y ? x : y)
#define ABS(x) ((x) < 0 ? -(x) : (x))

#ifdef PRISM_MPI_COLL
#define PRISM_BCAST MPI_Bcast
#else
#define PRISM_BCAST prism_v_bc
#endif

/* GLOBAL CONSTANTS */
#if PRISM_NX
#define ERRMSG_TYPE 1234567891   /* msg_type for hrecv */
#endif

/* FUNCTION PROTOTYPE DEFINITIONS */ 

extern double ** prism_m_d_alloc_matrix(int l_rows, int l_cols);
extern void prism_v_layout(int i_mdim, int i_dim, int i_v_dim,
			   int i_sbmsh_nw_x, int i_sbmsh_nw_y,
			   int i_sbmsh_rows, int i_sbmsh_cols,
			   int i_r_panelwidth, int i_c_panelwidth,
			   int i_panel_spc,
			   double (* func)(int i, int j),
			   double *p_dst, int i_lda, E_LAYOUT e_layout,
			   int i_2dcomm_rows, int i_2dcomm_cols,
			   int i_2d_row, int i_2d_col);

#if PRISM_NX
extern void prism_v_bc(void *p_v_send, int i_elements,
		       PRISM_E_DATATYPE e_datatype, int i_root,
		       PRISM_S_PROCGRP s_grp_use);
#else
extern void prism_v_bc(void *p_v_send, int i_elements,
		       MPI_Datatype x_datatype, int i_root, MPI_Comm comm_bc);
#endif

#if !PRISM_NX
extern void prism_v_bca_rb(int i_v_dim, int i_msh_rows, int i_msh_cols,
			   int i_node_row, int i_node_col, int i_rblk_nw,
			   int i_cblk_nw, int i_blk_rows,
			   int i_blk_cols, int i_g_m, int i_g_n, int i_g_k,
			   int i_m_panelwidth, int i_n_panelwidth,
			   int i_k_panelwidth, int i_m_offset, int i_n_offset,
			   int i_k_offset, int i_panel_spc, double r_alpha,
			   P_R_MATRIX m_r_a, int i_lda, 
			   P_R_MATRIX m_r_b, int i_ldb, double r_beta,
			   P_R_MATRIX m_r_c,
			   int i_ldc, P_R_MATRIX m_r_bcbuf, int i_bcbuf_sz,
			   P_R_MATRIX m_r_rollbuf, int i_rollbuf_sz,
			   MPI_Comm comm_row, MPI_Comm comm_col,
			   int (*i_lin_dim)(int i_v_dim, int i_g_dim,
					    int i_blk_nw,
					    int i_first_blk, int i_last_blk,
					    int i_panelwidth, int i_offset,
					    int i_panel_spc));

extern void prism_v_bca_rc(int i_v_dim, int i_msh_rows, int i_msh_cols,
			   int i_node_row, int i_node_col, int i_rblk_nw,
			   int i_cblk_nw, int i_blk_rows,
			   int i_blk_cols, int i_g_m, int i_g_n, int i_g_k,
			   int i_m_panelwidth, int i_n_panelwidth,
			   int i_k_panelwidth, int i_m_offset, int i_n_offset,
			   int i_k_offset, int i_panel_spc, double r_alpha,
			   P_R_MATRIX m_r_a, int i_lda,
			   P_R_MATRIX m_r_b, int i_ldb, double r_beta,
			   P_R_MATRIX m_r_c,
			   int i_ldc, P_R_MATRIX m_r_bcbuf, int i_bcbuf_sz,
			   P_R_MATRIX m_r_rollbuf, int i_rollbuf_sz,
			   MPI_Comm comm_row,
			   MPI_Comm comm_col,
			   int (*i_lin_dim)(int i_v_dim, int i_g_dim,
					    int i_blk_nw,
					    int i_first_blk, int i_last_blk,
					    int i_panelwidth, int i_offset,
					    int i_panel_spc));
#endif
#if PRISM_NX
extern void prism_v_skew (void * p_v_send, void * p_v_recv,
			  int i_s_elements, int i_r_elements,
			  PRISM_E_DATATYPE e_datatype,
			  int i_skewstep, PRISM_S_PROCGRP s_grp_use);
#else
extern void prism_v_skew (void * p_v_send, void * p_v_recv,
			  int i_s_elements, int i_r_elements,
			  MPI_Datatype x_datatype,
			  int i_skewstep, MPI_Comm comm_skew);
#endif

#if !PRISM_NX
extern void prism_v_bimmer(int i_v_dim, int i_rblk_nw, int i_cblk_nw,
			   int i_g_m, int i_g_n, int i_g_k, int i_m_panelwidth,
			   int i_n_panelwidth,
			   int i_k_panelwidth, int i_m_offset, int i_n_offset,
			   int i_k_offset, int i_panel_spc, E_BOOLEAN e_transa,
			   E_BOOLEAN e_transb, double r_alpha, P_R_MATRIX m_r_a,
			   int i_lda, P_R_MATRIX m_r_b, int i_ldb,
			   double r_beta,
			   P_R_MATRIX m_r_c, int i_ldc, P_R_MATRIX m_r_bcbuf,
			   int i_bcbuf_sz, P_R_MATRIX m_r_rollbuf,
			   int i_rollbuf_sz,
			   MPI_Comm comm_row, MPI_Comm comm_col,
			   int (*i_lin_dim)(int i_v_dim, int i_g_dim,
					    int i_blk_nw,
					    int i_first_blk, int i_last_blk,
					    int i_panelwidth, int i_offset,
					    int i_panel_spc));
#endif

extern void prism_v_scl_mtrx(int i_rows, int i_cols,
			     P_R_MATRIX m_r_a, double r_beta);
#if PRISM_SP
/* IBM doesn't append _ in c so need to fix up Fortran calls*/
#define dcopy_ dcopy
#endif
extern void dcopy_(fint *n, double *dx, fint *incx, double *dy, fint *incy) ;
#if PRISM_SP
/* IBM doesn't append _ in c so need to fix up Fortran calls*/
#define dgemm_ dgemm
#endif
extern void dgemm_(char *TRANSA, char *TRANSB, fint *M, fint *N, fint *K,
		   double *ALPHA, double *A, fint *LDA, double *B,
		   fint *LDB, double *BETA, double *C, fint *LDC);
#if PRISM_SP
/* IBM doesn't append _ in c so need to fix up Fortran calls*/
#define prism_v_dlacpy_ prism_v_dlacpy
#endif
extern void prism_v_dlacpy_(char *UPLO, fint *M, fint *N,
			    double *A, fint *LDA, double *B, fint *LDB);
/* since prism_v_free_matrix takes different types of pointers
   on differnt calls,
   don't give definition in protype because then get warnings from
   compilers */
extern void prism_v_free_matrix();
/* To get better error messages, we add two args to the error routine
   so that the file and line # get printed out */
extern void prism_v_generror(char c_error_text[],
			     E_ERRTYPES e_errtype,
			     char *filename, int lineno);
#define prism_v_generror(A,B) prism_v_generror(A,B,__FILE__,__LINE__)
#if PRISM_NX
extern void  prism_v_generraux(long type, long count, long node, long pid);
#endif
extern void prism_v_init_var();
extern int prism_i_ldim(int i_v_dim, int i_g_dim, int i_blk_nw,
			int i_first_blk,int i_last_blk,
			int i_panelwidth, int i_offset,
			int i_panel_spc);
extern int i_ldwk(int i_v_dim, int i_g_dim, int i_blk_nw,
		  int i_panelwidth, int i_offset, int i_panel_spc,
		  int i_row_blks, int i_sbmsh_rows);

extern int prism_i_datatype_size(PRISM_E_DATATYPE e_datatype);
#if !PRISM_NX
extern MPI_Datatype prism_x_datatype(PRISM_E_DATATYPE e_datatype);
#endif

#if PRISM_NX
extern void prism_v_copy(void * p_v_input, void * p_v_output,
			 int i_elements, PRISM_E_DATATYPE e_datatype);
#else
extern void prism_v_copy(void * p_v_input, void * p_v_output,
			 int i_elements, MPI_Datatype x_datatype);
#endif  
#if PRISM_PRT
extern void prism_v_prt_m(P_R_MATRIX m_r_a, int i_rows, int i_cols,
			  FILE *outfile);
#endif

#endif
