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
   general error handler
   prints the error message <error_text> and terminates execution 
*/

/* INCLUDE FILES */
#include <stdio.h>
#include <string.h>
#include "stdeig.h"
#include "mm.h"

void prism_v_generror(char c_error_text[], E_ERRTYPES e_errtype)

     /* PARAMETERS
	==========
	c_error_text:  error message

	e_errtype:  specifies whether to print verbose or brief error message
	before terminating processes */
{ 
  /* ------------------ */
  /* local variables */
  /* ------------------ */
#if !PRISM_NX
  int
    i_world_rank
      ;
#endif
  long 
    l_msglen
      ;
  char
    buf[120]
      ;

#if PRISM_NX
  sprintf(buf, "***Run-time error on node %d: ", mynode());
#else
  MPI_Comm_rank(MPI_COMM_WORLD, &i_world_rank);
  sprintf(buf, "***Run-time error on node %d: ", i_world_rank);
#endif
  strcat(buf,c_error_text);
   
  switch(e_errtype) {
  case verbose:
    fprintf(stderr,"%s***\n", buf);
#if PRISM_NX
    killproc(-1,0);
#else
    MPI_Abort(MPI_COMM_WORLD, (int)99);
#endif
#if PRISM_NX
  case brief:
    l_msglen = sizeof(char) * strlen(buf);

    /* send error msg to node 0 */

    csend(ERRMSG_TYPE,buf,l_msglen,0,0);  
    exit(0);
  default:  /* do the same as the case brief at this stage */
    l_msglen = sizeof(char) * strlen(buf);

    /* send error msg to node 0 */

    csend(ERRMSG_TYPE,buf,l_msglen,0,0);  
    exit(0);
#else
/* at current time, don't know any way for MPI to send a message to one
   node and cause interrupt as done with hrecv.  Must use verbose case */
  default:
    fprintf(stderr,"%s***\n", buf);
    MPI_Abort(MPI_COMM_WORLD, (int)99);
#endif
  }
} 

