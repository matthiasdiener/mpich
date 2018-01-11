/* This is pretty much copied from p4hetero, so here goes... */

#include "mpid.h"
#include "mpiddebug.h"
#include "nexuspriv.h"

#include <netinet/in.h>

MPID_INFO *MPID_procinfo = NULL;
MPID_H_TYPE MPID_byte_order;
static char *(ByteOrderName[]) = { "None", "LSB", "MSB", "XDR" };
int MPID_IS_HETERO = NEXUS_FALSE;
nexus_bool_t use_xdr = NEXUS_FALSE;

MPID_NEXUS_init_hetero(nexus_bool_t master, int *argc, char ***argv)
{
    int arg_num;
    int order;
    int i;

    /* Apparently the -mpixdr flag forces use of XDR for debugging and
     * timing comparisons.  I will look for both it and -xdr after the
     * -mpi switch.
     */
    if ((arg_num = nexus_find_argument(argc, argv, "xdr", 1)) >= 0)
    {
	use_xdr = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    else if ((arg_num = nexus_find_argument(argc, argv, "mpixdr", 1)) >= 0)
    {
	use_xdr = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }

    MPID_procinfo = (MPID_INFO *)malloc(MPID_WorldSize * sizeof(MPID_INFO));
    if (!(MPID_procinfo))
    {
	nexus_fatal("Not enough memory!\n");
    }

    for (i = 0; i < MPID_WorldSize; i++)
    {
	MPID_procinfo[i].byte_order = MPID_H_NONE;
	MPID_procinfo[i].short_size = 0;
	MPID_procinfo[i].int_size = 0;
	MPID_procinfo[i].long_size = 0;
	MPID_procinfo[i].float_size = 0;
	MPID_procinfo[i].double_size = 0;
	MPID_procinfo[i].float_type = 0;
    }

    /* Set my entry up */
#ifdef MPID_FLOAT_CRAY
    /*
     * Set floating point type.  IEEE is 0 (default), Cray is 2, others
     * can be added as needed.
     */
    MPID_procinfo[MPID_MyWorldRank].float_type = 2;
#endif
    
    if (use_xdr)
    {
	MPID_byte_order = MPID_H_XDR;
    }
    else
    {
	order = MPID_NEXUS_GetByteOrder();
	switch (order)
	{
	  case 1:  MPID_byte_order = MPID_H_LSB; break;
	  case 2:  MPID_byte_order = MPID_H_MSB; break;
	  default: MPID_byte_order = MPID_H_XDR; break;
	}
    }
    MPID_procinfo[MPID_MyWorldRank].byte_order = MPID_byte_order;
    MPID_procinfo[MPID_MyWorldRank].short_size = sizeof(short);
    MPID_procinfo[MPID_MyWorldRank].int_size = sizeof(int);
    MPID_procinfo[MPID_MyWorldRank].long_size = sizeof(long);
    MPID_procinfo[MPID_MyWorldRank].float_size = sizeof(float);
    MPID_procinfo[MPID_MyWorldRank].double_size = sizeof(double);

    /* Get everyone else's */
    if (master)
    {
	MPID_NEXUS_broadcast_types(MPID_procinfo);
    }
    else
    {
        MPID_NEXUS_get_types(MPID_procinfo);
    }
    MPID_IS_HETERO = MPID_procinfo[0].byte_order == MPID_H_XDR;
    for (i = 1; i < MPID_WorldSize; i++)
    {
	if (   MPID_procinfo[0].byte_order != MPID_procinfo[i].byte_order
	    || MPID_procinfo[i].byte_order == MPID_H_XDR
	    || MPID_procinfo[0].short_size != MPID_procinfo[i].short_size
	    || MPID_procinfo[0].int_size != MPID_procinfo[i].int_size
	    || MPID_procinfo[0].long_size != MPID_procinfo[i].long_size
	    || MPID_procinfo[0].float_size != MPID_procinfo[i].float_size
	    || MPID_procinfo[0].double_size != MPID_procinfo[i].double_size
	    || MPID_procinfo[0].float_type != MPID_procinfo[i].float_type)
	{
	    MPID_IS_HETERO = 1;
	    break;
	}
    }

    if (MPID_IS_HETERO)
    {
	for (i = 0; i < MPID_WorldSize; i++)
	{
	    if (i == MPID_MyWorldRank)
	    {
		continue;
	    }
	    if (   MPID_procinfo[MPID_MyWorldRank].byte_order !=
		    MPID_procinfo[i].byte_order
		|| MPID_procinfo[MPID_MyWorldRank].short_size !=
		    MPID_procinfo[i].short_size
		|| MPID_procinfo[MPID_MyWorldRank].int_size !=
		    MPID_procinfo[i].int_size
		|| MPID_procinfo[MPID_MyWorldRank].long_size !=
		    MPID_procinfo[i].long_size
		|| MPID_procinfo[MPID_MyWorldRank].float_size !=
		    MPID_procinfo[i].float_size
		|| MPID_procinfo[MPID_MyWorldRank].double_size !=
		    MPID_procinfo[i].double_size
		|| MPID_procinfo[MPID_MyWorldRank].float_type !=
		    MPID_procinfo[i].float_type)
	    {
		MPID_procinfo[i].byte_order = MPID_H_XDR;
	    }
	}
    }

    return MPI_SUCCESS;
}

MPID_NEXUS_Dest_byte_order(int dest)
{
    if (MPID_IS_HETERO)
    {
	return MPID_procinfo[dest].byte_order;
    }
    else
    {
	return MPID_H_NONE;
    }
}


/*
 * This function actually came from p4init.c
 */
int MPID_NEXUS_GetByteOrder(void)
{
    int l;
    char *b = (char *)&l;

    l = 1;
    if (b[0] == 1)
    {
	return 1;
    }
    if (b[sizeof(int) - 1] == 1)
    {
	return 2;
    }
    return 0;
}

MPID_NEXUS_Pkt_pack(MPID_PKT_T *pkt, int size, int dest)
{
    /* Do nothing.  This is all done in the send routines. */
}

MPID_NEXUS_Pkt_unpack(MPID_PKT_T *pkt, int size, int from)
{
    /* Do nothing.  This is all done in the receive routhines */
}

#if !defined(MPID_TEST_SYNC)
typedef struct _MPID_SyncID {
    int id;
    MPIR_SHANDLE *dmpi_send_handle;
    MPID_SHANDLE *mpid_send_handle;
    struct _MPID_SyncId *next;
} MPID_SyncId;

static MPID_SyncId *head = NULL;
static int CurId = 1;

MPID_Aint MPID_NEXUS_Get_Sync_Id(MPIR_SHANDLE *dmpi_handle,
			         MPID_SHANDLE *mpid_handle)
{
    MPID_SyncId *temp;
    int id = CurId;

    temp = (MPID_SyncId *)malloc(sizeof(MPID_SyncId));
    if (!temp)
    {
	MPID_NEXUS_Abort(MPI_ERR_EXHAUSTED);
    }
    temp->id = CurId++;
    temp->dmpi_send_handle = dmpi_handle;
    temp->mpid_send_handle = mpid_handle;
    temp->next = head;
    head = temp;

    return (MPID_Aint)id;
}

int MPID_NEXUS_Lookup_SyncAck(MPID_Aint sync_id,
			      MPIR_SHANDLE **dmpi_send_handle,
			      MPID_SHANDLE **mpid_send_handle)
{
    MPID_SyncId *cur, *last;

    last = NULL;
    *dmpi_send_handle = NULL;
    *mpid_send_handle = NULL;

    for (cur = head; cur; cur = cur->next)
    {
	if ((MPID_Aint)cur->id == sync_id)
	{
	    *dmpi_send_handle = cur->dmpi_send_handle;
	    *mpid_send_handle = cur->mpid_send_handle;
	    if (last)
	    {
		last->next = cur->next;
	    }
	    else
	    {
		head = cur->next;
	    }
	    if ((MPID_Aint)CurId == sync_id + 1)
	    {
		CurId--;
	    }
	    free(cur);
	    return MPI_SUCCESS;
	}
	last = cur;
    }
    /* Error, did not find id! */
    if (!dmpi_send_handle)
    {
	fprintf(stderr, "Error in processing sync id %x!\n", sync_id);
    }
    return MPI_SUCCESS;
}

int MPID_SyncAck(MPID_Aint sync_id, int from)
{
    MPIR_SHANDLE *dmpi_send_handle;
    MPID_SHANDLE *mpid_send_handle;

    MPID_NEXUS_Lookup_SyncAck(sync_id,
    			      &dmpi_send_handle,
			      &mpid_send_handle);
    if (!dmpi_send_handle)
    {
	fprintf(stderr, "Error in processing sync ack!\n");
	return MPI_ERR_INTERN;
    }
    DMPI_mark_send_completed(dmpi_send_handle);
    return MPI_SUCCESS;
}

void MPID_SyncReturnAck(MPID_Aint sync_id, int from)
{
    MPID_PKT_SEND_DECL(MPID_PKT_SYNC_ACK_T, pkt);

    MPID_PKT_SEND_ALLOC(MPID_PKT_SYNC_ACK_T, pkt);
    MPID_PKT_SEND_SET(pkt, mode, MPID_PKT_SYNC_ACK);
    MPID_PKT_SEND_SET(pkt, sync_id, sync_id);

    MPID_SendControl(MPID_PKT_SEND_ADDR(pkt),
		     sizeof(MPID_PKT_SYNC_ACK_T),
		     from);
}

void MPID_Sync_discard(MPIR_SHANDLE *dmpi)
{
    MPID_SyncId *cur;

    for (cur = head; cur; cur = cur->next)
    {
	if (cur->dmpi_send_handle == dmpi)
	{
	    cur->dmpi_send_handle = NULL;
	    cur->mpid_send_handle = NULL;
	    /* We leave this here just in case a reply is received, so
	     * that we can distinquish between invalid syncAck messages
	     * and synAcks for cancelled messages. */
	    return;
	}
    }
    /* Error, did not find id!  Ignored for now */
}
#endif /* MPID_SYNC_TEST */
