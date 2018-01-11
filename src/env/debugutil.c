/*
 *  $Id: debugutil.c,v 1.5 1997/04/11 13:44:49 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

/* N.B. 
 * We want to compile this file for debugging under all circumstances.
 * That way we guarantee to pass the structure definition of MPIR_PROCDESC
 * over to the debugger in the debug information, so it doesn't have to make
 * any presumptions about the size or layout of the fields therein.
 * This way it can work on all the different targets without a problem.
 *
 * Since the only function that gets called in here simply returns (and
 * it's only called on spawning processes), the run time cost of compiling
 * this with debugging and without optimisation is negligible.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2

/* We *only* usefully support the debugger for MPI_ADI2 */

#include "sbcnst2.h"
/* Error handlers in pt2pt */
#include "mpipt2pt.h"

/* Include references to the Queues and Communicators here too, 
 * to ensure that the debugger can see their types.
 */
#include "../util/queue.h"
#include "comm.h"
#include "req.h"

typedef struct MPIR_COMMUNICATOR MPIR_Communicator;

/* Array of procdescs for debugging purposes */
MPIR_PROCDESC *MPIR_proctable = 0;
int MPIR_proctable_size = 0;

/* List of all communicators */
MPIR_Comm_list MPIR_All_communicators;

/* Two global variables which a debugger can use for 
 * 1) finding out what the state of the program is at
 *    the time the magic breakpoint is hit.
 * 2) informing the process that it has been attached to and is
 *    now free to run.
 */
VOLATILE int MPIR_debug_state = 0;
VOLATILE int MPIR_debug_gate  = 0;
char * MPIR_debug_abort_string= 0;
int MPIR_being_debugged       = 0;

#else
/* Not ADI2 */
/* List of all communicators */
MPIR_Comm_list MPIR_All_communicators;
#endif

/*
   MPIR_Breakpoint - Provide a routine that a debugger can intercept
                     at interesting times.
		     Note that before calling this you should set up
		     MPIR_debug_state, so that the debugger can see
		     what is going on.

*/
void MPIR_Breakpoint()
{
  /* This routine is only here to have a breakpoint set in it,
   * it doesn't need any contents itself, but we don't want
   * it inlined and removed despite that.
   */

  /* With some compilers and debug formats, (e.g. Digital Unix ("the
   * operating system formerly known as OSF1"), and AIX), including
   * the header files is not sufficient to cause the type definitions
   * to be included in the object file debug information.  To cause
   * this to happen you also need to instance an entity of that type.
   * This simplest way to do that (without causing static space to be
   * allocated) is to instance local variables in a function. Since we
   * have this function to hand, and it doesn't hurt any of the other
   * implementations, we just put these in always.
   *
   * This also has the useful effect of documenting the 
   * types which are used by TotalView's MPICH support, and here 
   * they are.
   *
   * Note that picky compilers may complain about "declared and not used"
   * variables.  Some compilers may provide a #pragma that can
   * turn off those warnings; others may be quiet if the variables are
   * declared static.
   */
  MPIR_SQUEUE       sq;
  MPID_QHDR         qh;
  MPID_QUEUE         q;
  MPID_QEL         qel;
  MPIR_SQEL       sqel;
  MPIR_RHANDLE      rh;
  MPIR_Comm_list    cl;
  MPIR_Communicator  c;
  MPI_Status         s;
}
