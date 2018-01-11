/*
 *  $Id: debugutil.c,v 1.1 1996/07/05 16:03:09 gropp Exp $
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

@*/
void MPIR_Breakpoint()
{
    /* This routine is only here to have a breakpoint set in it,
     * it doesn't need any contents itself, but we don't want
     * it inlined and removed despite that.
     */
}




