/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

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

void prism_v_init_var()

{
  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
    i_initialized
      ;

  static E_BOOLEAN e_init_var = false;

  /* ------------------ */
  /* External functions */
  /* ------------------ */
#if PRISM_NX
#endif

  switch (e_init_var) {
  case true:  /* if called before then quick return */
    break;
  case false:
    e_init_var = true;

#if PRISM_NX 
    prism_i_nx_id = (int)mynode();
#if !PRISM_MEIKO
    /* initialize a hrecv for run-time error handling */
    if (prism_i_nx_id == 0) {
      hrecv(ERRMSG_TYPE, errmsg_buf, sizeof errmsg_buf, prism_v_generraux);
    }
    gsync();
#endif
#else
    /* see if MPI initialized */
    MPI_Initialized(&i_initialized);
    if (!i_initialized) {
      prism_v_generror("(prism_v_init_var) MPI is not initialized", brief);
    }
#endif

    /* constant initializations */

    i_zero = 0;
    i_one = 1;
    r_zero = 0.0;
    r_one = 1.0;
    break;
  }
  return;
}
