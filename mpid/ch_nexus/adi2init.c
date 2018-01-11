/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2init.c,v 1.2 1997/04/01 19:36:18 thiruvat Exp $
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

int MPID_IS_HETERO = NEXUS_FALSE;

void MPID_Init(int *argc, char ***argv, void *config, int *error_code)
{
    MPIR_shandles = MPID_SBinit(sizeof(MPIR_PSHANDLE), 100, 100);
    MPIR_rhandles = MPID_SBinit(sizeof(MPIR_PRHANDLE), 100, 100);

    MPID_Init_Cond_Variables();

    MPID_Nexus_Init(argc, argv);
    *error_code = 0;
}

void MPID_Abort(struct MPIR_COMMUNICATOR *comm,
		int code,
		char *user,
		char *str)
{
    nexus_fatal("[%d] %s%sAborting program %s\n",
    		    MPID_MyWorldRank,
		    user ? user : "",
		    user ? " " : "",
		    str ? str : "!");
}

void MPID_End(void)
{
    MPID_Nexus_End();
}

int MPID_DeviceCheck(MPID_BLOCKING_TYPE is_blocking)
{
    if (is_blocking)
    {
	nexus_poll_blocking();
    }
    else
    {
	nexus_poll();
    }

    return 1;
}

int MPID_CommFree(MPI_Comm comm)
{
    return MPI_SUCCESS;
}

int MPID_CommInit(MPI_Comm comm)
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
