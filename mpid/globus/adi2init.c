/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2init.c,v 1.4 1998/06/08 19:39:25 karonis Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
#include "mpimem.h"
#include <stdio.h>
#include "../util/cmnargs.h"
#include "../util/queue.h"
#include "reqalloc.h"

/* Home for these globals */
int MPID_MyWorldSize, MPID_MyWorldRank;
MPID_SBHeader MPIR_rhandles;
MPID_SBHeader MPIR_shandles;

int MPID_IS_HETERO = GLOBUS_FALSE;

void MPID_Init(int *argc, char ***argv, void *config, int *error_code)
{
    MPIR_shandles = MPID_SBinit(sizeof(MPIR_PSHANDLE), 100, 100);
    MPIR_rhandles = MPID_SBinit(sizeof(MPIR_PRHANDLE), 100, 100);

    MPID_Init_Cond_Variables();

    MPID_Globus_Init(argc, argv);
    *error_code = 0;
}

void MPID_Abort(struct MPIR_COMMUNICATOR *comm,
		int code,
		char *user,
		char *str)
{
    globus_nexus_buffer_t abort_buf;
    int i;

    for (i = 0; i < MPID_MyWorldSize; i ++)
    {
	/* send abort message to all but myself */
	if (i != MPID_MyWorldRank)
	{
	    globus_nexus_buffer_init(&abort_buf, globus_nexus_sizeof_int(1), 0);
	    globus_nexus_put_int(&abort_buf, &code, 1);
	    globus_nexus_send_rsr(&abort_buf, 
			&Nexus_nodes[i],
			ABORT_HANDLER_ID,
			GLOBUS_TRUE,   /* destroy buffer */
			GLOBUS_FALSE); /* called from non-threaded handler */
	} /* endif */
    } /* endfor */

    globus_fatal("[%d] %s%sAborting program %s\n",
    		    MPID_MyWorldRank,
		    user ? user : "",
		    user ? " " : "",
		    str ? str : "!");
}

void MPID_End(void)
{
    int i;

    /* flushing all rsr's on each sp */
    for (i = 0; i < MPID_MyWorldSize; i ++)
	globus_nexus_startpoint_flush(&Nexus_nodes[i]);

    /* START NICK DUROC */
    /* globus_nexus_shutdown(); */
    globus_module_deactivate(GLOBUS_NEXUS_MODULE);

    /* END NICK DUROC */
}

int MPID_DeviceCheck(MPID_BLOCKING_TYPE is_blocking)
{
    if (is_blocking)
    {
	globus_poll_blocking();
    }
    else
    {
	globus_poll();
    }

    return 1;
}

/* NICK */
/* int MPID_CommFree(MPI_Comm comm) */
int MPID_CommFree(struct MPIR_COMMUNICATOR *dummy)
{
    return MPI_SUCCESS;
}

int MPID_Complete_pending(void)
{
    return MPI_SUCCESS;
}

int MPID_WaitForCompleteSend(MPIR_SHANDLE *request)
{
    while (!request->is_complete)
    {
	MPID_DeviceCheck(MPID_BLOCKING);
    }
    return MPI_SUCCESS;
}

int MPID_WaitForCompleteRecv(MPIR_RHANDLE *request)
{
    while (!request->is_complete)
    {
	MPID_DeviceCheck(MPID_BLOCKING);
    }
    return MPI_SUCCESS;
}

void MPID_SetPktSize(int len)
{
    /* do nothing */
}

#define MPIDPATCHLEVEL 2.0
#define MPIDTRANSPORT "ch_nexus"
void MPID_Version_name(char *name)
{
    sprintf(name, "ADI version %4.2f - transport %s",
    		MPIDPATCHLEVEL, MPIDTRANSPORT);
}
