/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   PRISM routine (version 2.0)
   15 April 1994

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

#if PRISM_NX
/* HEADERS */
#include <stdio.h>
#include <string.h>
#include "stdeig.h"
#include "mm.h"

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
#ifdef PARAGON
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
#ifdef PARAGON
    killproc(-1,-1);
#else
    killproc(-1,0);
#endif
  }
}
#endif /* on PRISM_NX */
