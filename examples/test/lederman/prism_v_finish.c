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
   Function to finish up - call at end
*/
/* INCLUDE FILES */
#include "stdeig.h"
#include <stdio.h>

void prism_v_finish()

{
#if !PRISM_NX
  if (!i_initialized) {
    MPI_Finalize();
  }
#endif
}
