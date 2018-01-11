/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

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

/* undefine prism_v_generror since this is the routine and don't need to
   add 2 args at end */
#undef prism_v_generror

void prism_v_generror(char c_error_text[], E_ERRTYPES e_errtype, char file[],
		      int lineno)

     /* PARAMETERS
	==========
	c_error_text:  error message

	e_errtype:  specifies whether to print verbose or brief error message
	before terminating processes 

	file: string with file name.  usually provided by cpp with "__FILE__".
	see how prism_v_generror is macroized in mm.h

	lineno: line number of file.  usually provided by cpp with "__LINE__".
	see how prism_v_generror is macroized in mm.h
*/
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
    buf[150]
      ;

#if PRISM_NX
  sprintf(buf, "***Run-time error on node %d in file %s on line %d: ",
	  mynode(), file, lineno);
#else
  MPI_Comm_rank(MPI_COMM_WORLD, &i_world_rank);
  sprintf(buf, "***Run-time error on node %d in file %s on line %d: ",
	  i_world_rank, file, lineno);
#endif
  strcat(buf,c_error_text);
    
#if (PRISM_SP || PRISM_MEIKO) && PRISM_NX
/* make e_errtype = verbose always since can't do hrecv stuff on SP-1 */
  e_errtype = verbose;
#endif
   
  switch(e_errtype) {
  case verbose:
    fprintf(stderr,"%s***\n", buf);
#if PRISM_NX
#if PRISM_MEIKO
/*    killcube(-1,0);*/
    /* killcube does not seem to exist on CS2.  Instead, abort this node
       and give warning message */
    fprintf(stderr, "***node %d exiting, others may be left running***\n", mynode());
    exit(99);
#else
    killproc(-1,0);
#endif
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

#if PRISM_NX && !PRISM_MEIKO
void prism_v_generraux(long l_type, long l_count, long l_node, long l_pid)

     /* PURPOSE
	=======
	This function is invoked by node 0 only when it receives a msg of 
	type ERRMSG_TYPE defined in global.h */

     /* PARAMETERS
	==========
	provided by hrecv */
{
  /* double check that the msg received is really an error msg */
  if (strncmp(errmsg_buf,"***Run-time",11) == 0 ) {
    /* errmsg_buf is defined in mm.h and is used in prism_v_init_var.c */
    fprintf(stderr,"%s***\n",errmsg_buf);
    fprintf(stderr,"... exiting ...\n");
#if PRISM_PARAGON
    killproc(-1,-1);
#else
    killproc(-1,0);
#endif
  }
  /* else, the msg with ERRMSG_TYPE is NOT a real error msg */
  else {
    fprintf(stderr, "***ERROR: msg type %d has been reserved!***\n",
	    ERRMSG_TYPE);
    fprintf
      (stderr,
       "***Please do not use this number as msg type in your program!***\n");
    fprintf(stderr,"... exiting ...\n");
#if PRISM_PARAGON
    killproc(-1,-1);
#else
    killproc(-1,0);
#endif
  }
}
#endif /* on PRISM_NX */
