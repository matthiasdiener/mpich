#include "nexuspriv.h"

#define MPID_MyWorldRank _MPID_NEXUS_my_id()
#define MPID_WorldSize   _MPID_NEXUS_num_gps()

#define MPID_HAS_HETERO
#define MPID_PKT_INCLUDE_SRC
#define PI_NO_NRECV
#define PI_NO_NSEND

#ifdef MPID_HAS_HETERO
#define MPID_RecvAnyControl(pkt, size, from) \
{ \
    *from = ANY_NODE; \
    MPID_TRACE_CODE("BRecvAny", *(from)); \
    MPID_NEXUS_receive(MPI_CTRL_TAG, from, pkt, size); \
    MPID_TRACE_CODE("ERecvAny", *(from)); \
}
#else /* MPID_HAS_HETERO */
#define MPID_RecvAnyControl(pkt, size, from) \
{ \
    *from = ANY_NODE; \
    MPID_TRACE_CODE("BRecvAny", *(from)); \
    MPID_NEXUS_receive(MPI_CTRL_TAG, from, (char *)pkt, size); \
    MPID_TRACE_CODE("ERecvAny", *(from)); \
}
#endif /* MPID_HAS_HETERO */

#ifdef MPID_HAS_HETERO
#define MPID_RecvFromChannel(buf, maxsize, from) \
{ \
    MPID_TRACE_CODE("BRecvFrom", from); \
    MPID_NEXUS_receive(MPI_DATA_TAG, &(from), (MPID_PKT_T *)buf, maxsize); \
    MPID_TRACE_CODE("ERecvFrom", from); \
}
#else /* MPID_HAS_HETERO */
#define MPID_RecvFromChannel(buf, maxsize, from) \
{ \
    MPID_TRACE_CODE("BRecvFrom", from); \
    MPID_NEXUS_receive(MPI_DATA_TAG, &(from), (char *)buf, maxsize); \
    MPID_TRACE_CODE("ERecvFrom", from); \
}
#endif /* MPID_HAS_HETERO */

#define MPID_ControlMsgAvail() \
    MPID_NEXUS_control()

#define MPID_SENDCONTROL(handle, pkt, size, dest) \
{ \
    if (handle->is_non_blocking) \
    { \
	MPID_SendControl(pkt, size, dest); \
    } \
    else \
    { \
	MPID_SendControlBlock(pkt, size, dest); \
    } \
}

#define MPID_SendControlBlock(pkt, size, dest) \
    MPID_SendControl(pkt, size, dest)

#define MPID_SendControl(pkt, size, dest) \
{ \
    MPI_dest_t temp; \
    temp.value = dest; \
    MPID_TRACE_CODE("BSendControl", dest); \
    MPID_NEXUS_send_control(temp, (MPID_PKT_T *)pkt, size); \
    MPID_TRACE_CODE("ESendControl", dest); \
}

/*
 * This is a guess.  I am not sure what exactly should be here, so if
 * something doesn't work, it may be related to this function.
 *
 * I wish that the macros from ch_p4 would only access their parameters
 * instead of any old variable.  This can have serious side effects that
 * do become obvious because the programmer cannot tell the variables
 * are being changed without looking at the macros to find out.  This
 * is an unneccessary burden.  I am only doing it for compatibility
 * purposes with all the other devices already in operation.  I formally
 * object to the structure of them, however.
 */
#define MPID_SendData(buf, size, channel, mpid_send_handle) \
{ \
    mpid_send_handle->sid = 0; \
    MPID_SendChannel(buf, size, channel); \
    DMPI_mark_send_completed(dmpi_send_handle); \
}

#define MPID_SendChannel(buf, size, dest) \
{ \
    MPI_dest_t temp; \
    temp.value = dest; \
    MPID_TRACE_CODE("BSendChannel", dest); \
    MPID_NEXUS_send_data(temp, buf, size); \
    MPID_TRACE_CODE("ESendChannel", dest); \
}

#define MPID_IRecvFromChannel(buf, size, partner, id)
#define MPID_WRecvFromChannel(buf, size, partner, id)
#define MPID_RecvStatus(id)

#define MPID_ISendChannel(buf, size, partner, id)
#define MPID_WSendChannel(buf, size, partner, id)
#define MPID_TSendChannel(id)

#define MALLOC(a) \
    nexus_malloc((size_t)(a))
#define FREE(a) \
    nexus_free((void *)(a))
