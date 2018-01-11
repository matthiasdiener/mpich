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
   This is the stdeig.h header file.  It defines things that are included
   in most PRISM routines
*/

#ifdef PRISM_GAMMA
#define Prism_STR_MACHINE "GAMMA"
#if PRISM_NX
#include <cube.h>
extern long mypart(long *, long *);
#endif
#endif

#ifdef PRISM_DELTA
#define Prism_STR_MACHINE "DELTA"
#if PRISM_NX
#include <mesh.h>
#endif
#endif

#ifdef PRISM_PARAGON
#define Prism_STR_MACHINE "PARAGON"
#if PRISM_NX
#include <nx.h>
extern long mypart(long *, long *);
#endif
#endif

#ifdef PRISM_SP1
#define Prism_STR_MACHINE "SP1"

#ifdef PRISM_BLAS_DCOPY
/* IBM doesn't append _ in c */
#define dcopy_ dcopy
#endif
#endif

#if defined(PRISM_SUN) || defined(MPI_sun4)
#define Prism_STR_MACHINE "SUN"
#endif

#if defined(PRISM_IRIX) || defined(MPI_IRIX)
#define Prism_STR_MACHINE "IRIX"
#endif

#if !defined(Prism_STR_MACHINE)
#define Prism_STR_MACHINE "Unknown"
#endif

/* this is a hack.  make my_argc, my_argv global so it can get to
   prism_v_init_var for call to MPI_Init */

int
   my_argc
      ;

char
   **my_argv
      ;

#if !PRISM_NX
#include "mpi.h"

MPI_Comm
   x_comm_use0, /* initial communicator passed */
   x_comm_use, /* communicator to use in library, 2D topology */
   x_comm_row, /* rows of x_comm_use */
   x_comm_col /* columns of x_comm_use */
      ;

/* this global variable stores whether MPI was initialized when call
   prism_v_init_var.  Therefore know whether to finalize when done */
int
   i_initialized
      ;

#if 0
#include "tools.h"
#endif

#endif
