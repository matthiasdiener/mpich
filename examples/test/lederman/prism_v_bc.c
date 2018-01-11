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
   The following function is designed to perform a broadcast across the 
   supplied list of nodes.  It is done by recursively dividing the
   line of processors in half and sending to the nearest processor in the
   other half of the processors.  If sending from processor i to processor i+k
   is contension free as long as processors i+1,...,i+k-1 are not sending
   then this broadcast is contention free.  This is true, for example, if
   the processors lie on a physical line.
   

   Given the following input:

   prism_v_bc(p_v_send, 1000, MPI_DOUBLE, 3, x_comm_bc)

   where x_comm_bc has a group of 7 nodes.

   The algorithm can be view pictorially as:

   NODE     STAGE 1     STAGE 2    STAGE 3

   0                              <--|
                                -------
   1                   <--|       >--|
                     -------
   2       >--|        >--|       >--|
              |                 -------
   3          |                   <--|
           -------
   4       <--|        >--|       >--|
                          |     -------
   5                      |       <--|
                      -------
   6                   <--|      (done)

   where the symbols means that processor 2 sends to processor 4 in
   stage 1. 
*/

/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"

/* CONDITIONAL COMPILATION */
#ifdef PRISM_BC_READY
/* Use ready type semantics.  This is useful on several machines.  On the 
   Intel Delta it leads to substantial performance improvements on long
   messages.  On other machines, such as the paragon, it can be useful
   to guarentee that the recv is posted before the send occurs.  This
   avoids buffering the message first in system space and then copying
   into user space.  The overhead of the synchronization message may
   be less on these systems than the copy for longer messages.
*/
#endif
#ifdef PRISM_BC_READY 
/* on some machines, you don't want to use the ready mode for small messages
   because the overhead of the synchronization message is greater than
   the savings from the ready send.  This parameter defines the minimum
   size message in bytes for ready semantics to be used. */
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
void prism_v_bc (void *p_v_send, int i_elements, PRISM_E_DATATYPE e_datatype,
int i_root, PRISM_S_PROCGRP s_grp_use)
#else
void prism_v_bc (void *p_v_send, int i_elements, MPI_Datatype x_datatype,
int i_root, MPI_Comm x_comm_bc)
#endif

/* PARAMETERS
   ==========
   
   p_v_send: Pointer to first value to broadcast.  The values are in
   contiguous memory.  Input on i_root node and output on all others.

   i_elements: # of elements of type e_datatype to send. (input)

#if PRISM_NX
   e_datatype: PRISM datatype of p_v_send which gives units of
               i_elements (input)
#else
   x_datatype: MPI datatype of p_v_send which gives units of i_elements (input)
#endif

   i_root: root node.  This is the index of the node in the group given (input)

#if PRISM_NX
   s_grp_use: structure with # entries and pointer to array defining
   process group (input)
#else
   x_comm_bc: communicator to do broadcast over (input)
#endif

*/

{

#if PRISM_NX

#ifdef PRISM_BC_READY
#define Prism_READY_MIN 1600000000  /* starting # for forced message type */
#define Prism_SYNC_MIN 1357911      /* starting # for synchronizing message type */
#endif
#define Prism_SEND_MIN 13

#else

#ifdef PRISM_BC_READY
#define Prism_READY_MIN 111     /* starting # for ready message type */
#define Prism_SYNC_MIN 13	  /* starting # for synchronizing message type */
#endif
#define Prism_SEND_MIN 13

#endif

  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
#ifdef PRISM_BC_READY
    i_sync,	/* dummy variable for zero length synchronization send/recv */
#endif
#if PRISM_BC_READY || PRISM_NX
    i_bytes,    /* size in bytes of message */
#endif
#if PRISM_NX
    i_send,	/* physical address of what node to send to in */
		/* the group of processes */
#endif
    i_max,	/* the maximum (last) node in your part of */
		/* the group of processes */
    i_mid,	/* the middle node in your part of the group of processes */
    i_min,	/* the minimum (first) node in your part of */
		/* the group of processes */
    i_wr,	/* what node to receive in your part of group of processes */
    i_ws,	/* what node to send in your part of group of processes */
    i_index,    /* index of this node in s_grp_use.p_i_grp */
    i_numproc,  /* # of processes involved in broadcast */
    i_l1        /* loop index */
    ;

#ifdef PRISM_NX
  long
    l_msg_num	/* id for irecv */
      ;
#else
  MPI_Status
    x_status
      ;

  MPI_Request
    x_request
      ;

#if PRISM_BC_READY
  MPI_Aint
    i_extent
      ;
#endif
#endif

  char
    c_error[120]  /* stores error message for generror */
      ;

  /* ------------------ */
  /* External functions */
  /* ------------------ */
  extern PRISM_V_GENERROR;
  extern PRISM_V_INIT_VAR;
  extern PRISM_I_DATATYPE_SIZE;
  
  /* global initializations */
  prism_v_init_var();

#if PRISM_NX
  i_numproc = s_grp_use.i_num;

  /* figure out what this nodes index is on the list */
  i_l1 = 0;
  i_index = -1;
  while (i_index == -1 && i_l1 < i_numproc) {
    if (s_grp_use.p_i_grp[i_l1] == i_node_id) {
      i_index = i_l1;
    }
    else {
      i_l1++;
    }
  }

  /* error if call but not participating */
  if (i_index == -1) {
    sprintf(c_error, "prism_v_bc: node %d not in list given", i_node_id);
    prism_v_generror(c_error, brief);
  }
#else
  MPI_Comm_rank(x_comm_bc, &i_index);
  /* error if call but not participating */
  if (i_index == -1) {
    sprintf(c_error, "prism_v_bc: node %d not in communicator given",
	    i_node_id);
    prism_v_generror(c_error, brief);
  }
  MPI_Comm_size(x_comm_bc, &i_numproc);
#endif

  /* check to see if got NULL receive pointers */
  if (p_v_send == NULL) {
    prism_v_generror("prism_v_bc: the receiving pointer (p_v_send) is NULL", brief);
  }

  /* check to see if message length is < 0. */
  if (i_elements < 0) {
    prism_v_generror("prism_v_bc: the message length less than 0", brief);
  }

  /* error if broadcasting node not between 0 and # processes - 1 */

  if (i_root < 0 || i_root >= i_numproc) {
    sprintf(c_error, "prism_v_bc: i_root = %d is < 0 or >= %d (# nodes participating)", i_root, i_numproc);
    prism_v_generror(c_error, brief);
  }

#if PRISM_NX
  i_bytes = prism_i_datatype_size(e_datatype) * i_elements;
#elif PRISM_BC_READY
  MPI_Type_extent(x_datatype, &i_extent);
  i_bytes = (int)i_extent * i_elements;
#endif

  i_ws = i_root; /* initial node to send is node to broadcast */
  i_min = 0; /*initial min mode is first node in group */
  i_max = i_numproc - 1; /* initial max mode is last node in group */

  /* while loop until you are done sending and receiving.  You know you
     are done when your max == min because this means that your group within 
     the row or column only includes you so you have recursively finished */

  while (i_min != i_max) {
    i_mid = (i_max + i_min) / 2;

    /* reset who receives in your group.  if the sending node is at or below
       the middle then you send one past the middle node.  if the sending
        node is above the middle then you send to the middle node */

    if (i_ws <= i_mid) {
      i_wr = i_mid + 1;
    }
    else {
      i_wr = i_mid;
    }

    /* do sending and receiving for node that should */
    if (i_index == i_ws) {
      /* you are a sending node. */

#if PRISM_NX
      /* physical address of who to send to */
      i_send = s_grp_use.p_i_grp[i_wr];
#endif

#ifdef PRISM_BC_READY
      if (i_bytes >= Prism_READY_MSIZE) {
	/* first you wait until you receive a 
	   then synchronization message from the node you expect to send to 
	   and you send a ready type message to that node since you know it
	   is now ready to receive */
	/* get a zero length message */
#if PRISM_NX
	crecv ((long)(Prism_SYNC_MIN + i_send), &i_sync, (long)0);
#else
	MPI_Recv(&i_sync, (int)0, MPI_INT, i_wr, 
		 Prism_SYNC_MIN, x_comm_bc, &x_status);
#endif
	       
	/* send data by ready type */
#if PRISM_NX
	csend ((long)(Prism_READY_MIN + i_node_id), p_v_send, (long)i_bytes,
	       (long)i_send, (long)0);
#else
	MPI_Rsend(p_v_send, i_elements, x_datatype, i_wr,
		  Prism_READY_MIN, x_comm_bc);
#endif
      }
      else {
	/* the message size is too small, send as regular type message */

#endif /* on PRISM_BC_READY */
      /* send regular type message */
#if PRISM_NX
      csend((long)(Prism_SEND_MIN + i_node_id), p_v_send, (long)i_bytes,
	    (long)i_send, (long)0);
#else
      MPI_Send(p_v_send, i_elements, x_datatype, i_wr,
		Prism_SEND_MIN, x_comm_bc);
#endif

#if PRISM_BC_READY
    }
#endif
    }
    else if (i_index == i_wr) {
      /* you are a receiving node.  */ 

#if PRISM_NX
      /* physical address of node to send */
      i_send = s_grp_use.p_i_grp[i_ws];
#endif

#ifdef PRISM_BC_READY 
      if (i_bytes >= Prism_READY_MSIZE) {
	/* first you post a receive for the ready 
	   type message and then send a synchronizing message to the node you 
	   expect to receive receive from so it knows to send */
	/* post receive for broadcast data */
#if PRISM_NX
	l_msg_num = irecv((long)(Prism_READY_MIN + i_send), p_v_send,
			  (long)i_bytes);
#else
	MPI_Irecv(p_v_send, i_elements, x_datatype, i_ws,
		  Prism_READY_MIN, x_comm_bc, &x_request);
#endif

	/* send sychronization message */
#if PRISM_NX
	csend((long)(Prism_SYNC_MIN + i_node_id), &i_sync, (long)0,
	      (long)i_send, (long)0);
#else
	MPI_Send(&i_sync, (int)0, MPI_INT, i_ws,
		 Prism_SYNC_MIN, x_comm_bc);
#endif

	/* wait until data received before proceeding */
#if PRISM_NX
	msgwait(l_msg_num);
#else
	MPI_Wait(&x_request, &x_status);
#endif
      }
      else {
	/* the message size is too small, send as regular type message */

#endif /* on PRISM_BC_READY */
      /* receive regular type message */
#if PRISM_NX
      crecv((long)(Prism_SEND_MIN + i_send), p_v_send, (long)i_bytes);
#else
      MPI_Recv(p_v_send, i_elements, x_datatype, i_ws,
		Prism_SEND_MIN, x_comm_bc, &x_status);
#endif

#if PRISM_BC_READY
    }
#endif
    }

    /* recalculate the min, max and send nodes as needed */
    if (i_index > i_mid) { /* your min changes if you are above middle */
      i_min = i_mid + 1;
      /* to get here you are above the middle.  If the sending node was below 
	 the middle then your new group received the value and that node 
	 becomes the sending node next time */
      if (i_ws <= i_mid) {
	i_ws = i_mid + 1;
      }
    }
    else { /* your max changes if you are below the middle */
      i_max = i_mid;
      /* to get here you are at or below the middle.  If the sending node was 
	 above the middle then your new group received the value and that node 
	 becomes the sending node next time */
      if (i_ws > i_mid) {
	i_ws = i_mid;
      }
    }
  } /* end while */
  return;
}
