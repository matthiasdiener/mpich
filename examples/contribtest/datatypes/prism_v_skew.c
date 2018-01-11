/* 
  COPYRIGHT U.S. GOVERNMENT 
  
  This software is distributed without charge and comes with
  no warranty.
  
  Please feel free to send questions, comments, and problem reports
  to prism@super.org. 
  */

/* PURPOSE ======= 	
   The following function is designed to perform a skew across the 
   supplied list of nodes.  It is done by first giving each node a 
   color such that sender and receiver in each pair will have
   different colors. The operation then is divided into two stages :
   1) nodes that have color 0 do sending, nodes that their sender has
   color 0 do receiving.
   2) nodes that have color 1,2 do sending, nodes that their sender has
   color 1,2 do receiving.
   
   Given the following input:
   prism_v_skew (* p_v_send, * p_v_recv, 1000, 800, 
   prism_double , 3, x_comm_skew)
   where x_comm_skew has a group of 8 processes.
   
   The algorithm can be view pictorially as:
   
   NODE     COLORING     STAGE 1    STAGE 2
   
   0          0            s          r
   
   1          1            r          s                     
   
   2          0            s          r
   
   3          1            r          s            
   
   4          0            s          r
   
   5          1            r          s                     
   
   6          0            s          r      
   
   7          1            r          s                    
   
   where  s means sending, r means receving.
   
   */

/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"
#include <stdio.h>
/* CONDITIONAL COMPILATION */
#if PRISM_SKEW_READY
/* Use ready type semantics.  This is useful on several machines.  On the 
   Intel Delta it leads to substantial performance improvements on long
   messages.  On other machines, such as the paragon, it can be useful
   to guarentee that the recv is posted before the send occurs.  This
   avoids buffering the message first in system space and then copying
   into user space.  The overhead of the synchronization message may
   be less on these systems than the copy for longer messages.  At the
   current time this routine does not support changing modes in real time.
   */
#endif

#if PRISM_SKEW_READY
#if PRISM_DELTA
#define Prism_READY_MSIZE 7000
#elif PRISM_PARAGON
/* not sure of correct value */
#define Prism_READY_MSIZE 10000
#else
/* by default always use if asked for on other machines */
#define Prism_READY_MSIZE 0
#endif
#endif
#if PRISM_NX
void prism_v_skew (void * p_v_send, void * p_v_recv, int i_s_elements,
		   int i_r_elements,  PRISM_E_DATATYPE e_datatype, 
		   int i_skewstep, PRISM_S_PROCGRP s_grp_use)
#else
     void prism_v_skew (void * p_v_send, void * p_v_recv, int i_s_elements,
			int i_r_elements,  MPI_Datatype x_datatype, 
			int i_skewstep, MPI_Comm x_comm_skew)
#endif
     
     /* PARAMETERS
	==========
	p_v_send   : pointer to the sending buffer
	
	p_v_recv   : pointer to the receiving buffer (output)
	
	i_s_elements : number of elements to send
	
	i_r_elements : number of elements to receive
	
	#if PRISM_NX
	e_datatype: PRISM datatype of p_v_send which gives units of
	i_elements (input)
	#else
	x_datatype: MPI datatype of p_v_send which gives units of i_elements (input)
	#endif
	
	i_skewstep : distance to skew.  positive imples down list
	s_grp_use.p_i_grp and negative implies up list.  
	Thus if you are entry i_node in p_i_grp, then you  
	send to s_grp_use.p_i_grp[(s_grp_use.p_i_grp[i_node]+
	i_skewstep)%s_grp_use.i_num].
	
	#if PRISM_NX
	s_grp_use: structure with # entries and pointer to array defining
	process group (input)
	#else
	x_comm_bc: communicator to do broadcast over (input)
	#endif
	*/
     
     
{
#if PRISM_SKEW_READY
#if PRISM_NX
#define Prism_READY_MIN 1700000000  /* starting # for forced message type */
#define Prism_SYNC_MIN 246810   /* starting # for synchronizing message type */
#else
#define Prism_READY_MIN 1111   /* starting # for ready message type */
#define Prism_SYNC_MIN 555     /* starting # for synchronizing message type */
#endif
#endif
  
  
#define Prism_SEND_MIN 77
  
  
  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
#if PRISM_NX
    i_succ,                      /* receiver node number */
    i_pred,                      /* sender node number */
#endif
    
#if PRISM_SKEW_READY    
    i_sync,     /* dummy variable for zero length synchronization send/recv */
#endif
    i_step_cont,                 /* counter */
    i,                           /* loop counter */ 
    i_my_color,                  /* my color */
    i_temp,                      /* temp variable */
    i_skew_set,                  /* least significant bit position */
    i_pred_index,                /* sender index number */
    i_my_index,                  /* node index in the array */
    i_numproc,                   /* # of processes involved in skew */
    i_succ_index                 /* sucessor's node index */
      ;
  
#if PRISM_NX
  long
    l_msgid_1,                   /* used in msgwait */
    l_msgid_2,                   /* used in msgwait */
    l_s_msg_len,                 /* sending message length in bytes */
    l_r_msg_len                  /* receiving  message length in bytes */
      ;
#endif
  
  char
    c_error[120]  /* stores error message for generror */
      ;
  
  
#if !PRISM_NX
  MPI_Status
    x_status,
    y_status
      ;
  
  MPI_Request
    x_request,
    y_request
      ;
#endif
  
  
  /* ------------------ */
  /* External functions */
  /* ------------------ */
  
  /* ------------------ */
  /* External variables */
  /* ------------------ */
  
  /* global initializations */
  
  prism_v_init_var();
  
#if PRISM_NX
  i_numproc = s_grp_use.i_num;
  
  /* figure out what this nodes index is on the list */
  
  i_step_cont = 0;
  i_my_index = -1;
  
  while (i_my_index == -1 && i_step_cont < i_numproc) {
    if (s_grp_use.p_i_grp[i_step_cont] == prism_i_nx_id) {
      i_my_index = i_step_cont;
    }
    else {
      i_step_cont++;
    }
  }
  
  /* error if call but not participating */
  if (i_my_index == -1) {
    sprintf(c_error, "prism_v_skew: node %d not in list given", prism_i_nx_id);
    prism_v_generror(c_error, brief);
  }
#else
  MPI_Comm_rank(x_comm_skew, &i_my_index);
  MPI_Comm_size(x_comm_skew, &i_numproc);
#endif  
  
  /* check to see if got NULL send pointers */
  if (p_v_send == NULL) {
    prism_v_generror("prism_v_skew: the receiving pointer (p_v_send) is NULL", brief);
  }
  
  /* check to see if got NULL receive pointers */
  if (p_v_recv == NULL) {
    prism_v_generror("prism_v_skew: the receiving pointer (p_v_recv) is NULL", brief);
  }
  
  /* check to see if message length is < 0. */
  if (i_s_elements < 0 || i_r_elements < 0) {
    sprintf(c_error, "prism_v_skew: the send length = %d or recv lenght = %d less than 0",
	    i_s_elements, i_r_elements);
    prism_v_generror(c_error, brief);
  }
  
  /* check to see if p_v_send == p_v_recv  */
  if ( p_v_send == p_v_recv ) {
    prism_v_generror ("prism_v_skew: send and recv bufr has to be different ", brief);
  }
  
  
  /* set up */
  
  /* compute the skew step */
  
  i_skewstep = i_skewstep % i_numproc;
  if (i_skewstep < 0) {
    i_skewstep += i_numproc;
  }
  if (i_skewstep == i_numproc ) {
    i_skewstep = 0 ;
  }
  
#if PRISM_NX
  l_s_msg_len = (long) (prism_i_datatype_size(e_datatype) * i_s_elements);
  l_r_msg_len = (long) (prism_i_datatype_size(e_datatype) * i_r_elements);
#endif
  
  /* should do a copy when skew step =0 */
  if (i_skewstep == 0) {
    if (i_s_elements != i_r_elements) {
      sprintf(c_error, "prism_v_skew: skewstep is 0 but the send elements = %d != recv elements = %d",
	      i_s_elements, i_r_elements);
      prism_v_generror(c_error, brief);
    }
#if PRISM_NX
    prism_v_copy (p_v_send, p_v_recv, i_s_elements, e_datatype);
#else
    prism_v_copy (p_v_send, p_v_recv, i_s_elements, x_datatype);
#endif  
    return;
  }
  
  
  i_succ_index =  (i_my_index+i_skewstep+i_numproc)%i_numproc; 
  i_pred_index =  (i_my_index-i_skewstep+i_numproc)%i_numproc;
#if PRISM_NX
  i_succ = s_grp_use.p_i_grp [ i_succ_index ];
  i_pred = s_grp_use.p_i_grp [ i_pred_index ];
#endif  
  
  /* check whether we should use the simple method */
  /* if i_skewstep = 1 (roll) and number of process nodes is odd,or skew step is 
     half of the number of nodes then use simple method */
  
  if ( ( (i_skewstep >= (i_numproc/2)) 
	&& (i_skewstep <= ((i_numproc+1)/2)) ) ||
      ( ( (i_skewstep == 1)||(i_skewstep == (i_numproc-1)))
        && (i_numproc % 2 == 1) ) ||
      (  i_numproc <= 5) ) {
    
#if PRISM_SKEW_READY

#if PRISM_NX 
    if ( l_s_msg_len> Prism_READY_MSIZE) {
      if ( l_r_msg_len> Prism_READY_MSIZE) {
#else
    if ( i_s_elements> Prism_READY_MSIZE) {
      if ( i_r_elements> Prism_READY_MSIZE) {
#endif

	/* first you wait until you receive a 
	   then synchronization message from the node you expect to send to 
	   and you send a ready type message to that node since you know it
	   is now ready to receive */
	/* get a zero length message */
	
#if PRISM_NX 
	l_msgid_1 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
	csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
	msgwait(l_msgid_1);
	
#else

/*     This part of the code will cause the system to hang when
       the message length is too large ( > 2.6 M bytes for delta)

        MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
                  Prism_READY_MIN, x_comm_skew, &x_request);
        
        MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
                 Prism_SYNC_MIN, x_comm_skew);
        
        MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
                 Prism_SYNC_MIN, x_comm_skew, &x_status);
        
        MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
                  Prism_READY_MIN, x_comm_skew);
        
        MPI_Wait(&x_request, &x_status);
*/        

	
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
	        Prism_SEND_MIN, x_comm_skew, &x_request);
      
      MPI_Isend(p_v_send, i_s_elements, x_datatype, i_succ_index,
	       Prism_SEND_MIN, x_comm_skew, &y_request);
      
      MPI_Wait(&y_request, &y_status);
      MPI_Wait(&x_request, &x_status);

      
#endif  
	
      }
      else {
#if PRISM_NX 
	l_msgid_1 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
	msgwait(l_msgid_1);
	
#else
	MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		  Prism_SEND_MIN, x_comm_skew, &x_request);
	
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
		 Prism_SYNC_MIN, x_comm_skew, &x_status);
	
	MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
		  Prism_READY_MIN, x_comm_skew);
	
	MPI_Wait(&x_request, &x_status);
	
#endif
	
      }
    }

#if PRISM_NX 
else if ( l_r_msg_len> Prism_READY_MSIZE) 
#else
else if ( i_r_elements> Prism_READY_MSIZE)
#endif  
    
      {
#if PRISM_NX 
	l_msgid_1 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
	csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
	csend(Prism_SEND_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
	msgwait(l_msgid_1);

#else
	MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		  Prism_READY_MIN, x_comm_skew, &x_request);
	MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
		 Prism_SYNC_MIN, x_comm_skew);
	MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
		 Prism_SEND_MIN, x_comm_skew);
	
	MPI_Wait(&x_request, &x_status);

#endif

      }
    else {
#endif      
      /* the message size is too small, send as regular type message */
      
#if PRISM_NX
      l_msgid_1 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
      csend(Prism_SEND_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
      msgwait(l_msgid_1);

#else
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		Prism_SEND_MIN, x_comm_skew, &x_request);
      
      MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
	       Prism_SEND_MIN, x_comm_skew);
      
      MPI_Wait(&x_request, &x_status);
      

#endif


#if PRISM_SKEW_READY
  }
#endif
        return;
}  
  /* dimension is even and step is odd case */
  
  if ( (i_numproc % 2)==0 && (i_skewstep % 2)==1 ) {
    i_my_color   =   i_my_index & 1;
  } 
  else { 
    
    /* coloring of index*/
    
    i_step_cont = 0 ;
    
    if (i_skewstep > i_numproc/2)
      i_temp =  i_numproc - i_skewstep;
    else
      i_temp = i_skewstep ;
    
    i_skew_set = 1 ;
    
    while((i_temp & 1) == 0) {
      i_step_cont++;
      i_temp =i_temp >> 1;
    }
    
    i_skew_set = i_skew_set << (i_step_cont);
    
    i_my_color =  ((i_skew_set & i_my_index)>0 ) +
      (!(((i_skew_set & i_my_index)>0)^((i_skew_set & i_succ_index)>0)))
	+(((i_skew_set & i_my_index) == 0)&((i_skew_set & i_succ_index)==0));
  }    
  
  /* start sending */
#if PRISM_SKEW_READY

#if PRISM_NX 
    if ( l_s_msg_len> Prism_READY_MSIZE) {
      if ( l_r_msg_len> Prism_READY_MSIZE) {
#else
    if ( i_s_elements> Prism_READY_MSIZE) {
      if ( i_r_elements> Prism_READY_MSIZE) {
#endif
      
#if PRISM_NX 
      if ( i_my_color != 0 ) { 
	l_msgid_1 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
	csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
      } 
      if ( i_my_color != 1 ) {
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
      }  
      if ( i_my_color != 0 ) { 
	msgwait(l_msgid_1);
      }  
      
      if ( i_my_color == 0 ) { 
	l_msgid_2 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
	csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
	msgwait(l_msgid_2);
      }
      if  ( i_my_color == 1 ) {
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0);
      }
#else
      if ( i_my_color != 0 ) { 
	MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		  Prism_READY_MIN, x_comm_skew, &x_request);
	
	MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
		 Prism_SYNC_MIN, x_comm_skew);
	
	
      } 
      if ( i_my_color != 1 ) {
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
		 Prism_SYNC_MIN, x_comm_skew, &x_status);
	
	MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
		  Prism_READY_MIN, x_comm_skew);
      }  
      if ( i_my_color != 0 ) { 
	MPI_Wait(&x_request, &x_status);
      } 
      
      if ( i_my_color == 0 ) { 
	MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		  Prism_READY_MIN, x_comm_skew, &x_request);
	
	MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
		 Prism_SYNC_MIN, x_comm_skew);
	MPI_Wait(&x_request, &x_status);
	
      }
      if  ( i_my_color == 1 ) {
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
		 Prism_SYNC_MIN, x_comm_skew, &x_status);
	
	MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
		  Prism_READY_MIN, x_comm_skew);
      }  
#endif  

    }
    else {
#if PRISM_NX 
      if ( i_my_color != 0 ) { 
	l_msgid_1 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
      } 
      if ( i_my_color != 1 ) {
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
      }
      if ( i_my_color != 0 ) { 
	msgwait(l_msgid_1);
      }  
      
      if ( i_my_color == 0 ) { 
	l_msgid_2 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
	msgwait(l_msgid_2);
      }
      if  ( i_my_color == 1 ) {
	crecv(Prism_SYNC_MIN+prism_i_nx_id, p_v_recv, 0);
	csend(Prism_READY_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0);
      }
#else
      if ( i_my_color != 0 ) { 
	MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		  Prism_SEND_MIN, x_comm_skew, &x_request);
      } 
      if ( i_my_color != 1 ) {
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
		 Prism_SYNC_MIN, x_comm_skew, &x_status);
	
	MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
		  Prism_READY_MIN, x_comm_skew);
      }
      if ( i_my_color != 0 ) { 
	MPI_Wait(&x_request, &x_status);
      }  
      
      if ( i_my_color == 0 ) { 
	MPI_Irecv(p_v_send, i_r_elements, x_datatype, i_pred_index,
		  Prism_SEND_MIN, x_comm_skew, &x_request);
	MPI_Wait(&x_request, &x_status);
      }
      if  ( i_my_color == 1 ) {
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_succ_index, 
		 Prism_SYNC_MIN, x_comm_skew, &x_status);
	
	MPI_Rsend(p_v_send, i_s_elements, x_datatype, i_succ_index,
		  Prism_READY_MIN, x_comm_skew);
      }
      
#endif 

    }
 }

#if PRISM_NX 
else if ( l_r_msg_len> Prism_READY_MSIZE) 
#else
else if ( i_r_elements> Prism_READY_MSIZE)
#endif  
      
      {
#if PRISM_NX 
    if ( i_my_color != 0 ) { 
      l_msgid_1 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
      csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
    } 
    if ( i_my_color != 1 ) {
      csend(Prism_SEND_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
    }
    if ( i_my_color != 0 ) { 
      msgwait(l_msgid_1);
    }  
    
    if ( i_my_color == 0 ) { 
      l_msgid_2 = irecv(Prism_READY_MIN+i_pred, p_v_recv, l_r_msg_len);
      csend(Prism_SYNC_MIN+i_pred, p_v_send, 0, i_pred, 0);
      msgwait(l_msgid_2);
    }
    if  ( i_my_color == 1 ) {
      csend(Prism_SEND_MIN +prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0);
    }
#else
    if ( i_my_color != 0 ) { 
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
                Prism_READY_MIN, x_comm_skew, &x_request);
      
      MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
               Prism_SYNC_MIN, x_comm_skew);
      
    } 
    if ( i_my_color != 1 ) {
      MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
               Prism_SEND_MIN, x_comm_skew);
    }
    if ( i_my_color != 0 ) { 
      MPI_Wait(&x_request, &x_status);
    }  
    
    if ( i_my_color == 0 ) { 
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
                Prism_READY_MIN, x_comm_skew, &x_request);
      
      MPI_Send(&i_sync, (int)0, MPI_INT, i_pred_index,
               Prism_SYNC_MIN, x_comm_skew);
      MPI_Wait(&x_request, &x_status);
    }
    if  ( i_my_color == 1 ) {
      MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
               Prism_SEND_MIN, x_comm_skew);
    }
#endif 

  }
else {



#endif 
    /* the message size is too small, send as regular type message */
    
#if PRISM_NX 
    if ( i_my_color != 0 ) { 
      l_msgid_1 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
    } 
    if ( i_my_color != 1 ) {
      csend(Prism_SEND_MIN+prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0); 
    }  
    if ( i_my_color != 0 ) { 
      msgwait(l_msgid_1);
    }  
    
    if ( i_my_color == 0 ) { 
      l_msgid_2 = irecv(Prism_SEND_MIN+i_pred, p_v_recv, l_r_msg_len);
      msgwait(l_msgid_2);
    }
    if  ( i_my_color == 1 ) {
      csend(Prism_SEND_MIN +prism_i_nx_id, p_v_send, l_s_msg_len, i_succ, 0);
    }
#else
    if ( i_my_color != 0 ) { 
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		Prism_SEND_MIN, x_comm_skew, &x_request);
    } 
    if ( i_my_color != 1 ) {
      MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
	       Prism_SEND_MIN, x_comm_skew);
    }  
    if ( i_my_color != 0 ) { 
      MPI_Wait(&x_request, &x_status);
    } 
    if ( i_my_color == 0 ) { 
      MPI_Irecv(p_v_recv, i_r_elements, x_datatype, i_pred_index,
		Prism_SEND_MIN, x_comm_skew, &x_request);
      MPI_Wait(&x_request, &x_status);
    }
    if  ( i_my_color == 1 ) {
      MPI_Send(p_v_send, i_s_elements, x_datatype, i_succ_index,
	       Prism_SEND_MIN, x_comm_skew);
    }
#endif

#if PRISM_SKEW_READY

  }	
#endif
  
	return;   
}




