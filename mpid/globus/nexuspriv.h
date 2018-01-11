#ifndef _GLOBUS_NEXUS_PRIV_H_
#define _GLOBUS_NEXUS_PRIV_H_

#include "globus_gram_myjob.h"
#include "globus_nexus.h"

/* begin NICK */
/* extern globus_nexus_startpoint_t *MPID_Nexus_nodes; */
/* extern globus_nexus_endpointattr_t MPID_default_ep_attr; */
/* extern int *MPID_remote_formats; */
extern globus_nexus_startpoint_t *Nexus_nodes;
extern globus_nexus_endpointattr_t default_ep_attr;
extern int *remote_formats;

/* begin status->private macros */

/* these macros are intended to manipulate the "private_count" field (an int) */
/* in MPI_Status.  The "private_count" field is sometimes (when necessary)    */
/* used to store the dataorigin_format in the rightmost byte of the           */
/* field (formats are always at most one byte).  these macros manipulate the  */
/* second byte from the right of the "private_count" field. that byte is used */
/* to identify how the "count" field in MPI_Status should be interpreted ...  */
/* there are three possible interpretations:                                  */
/*     1. count field is local byte count of received data                    */
/*     2. count field is dataorigin (nonpacksize) byte count of received data */
/*     3. count field is number of bytes receiver has provided to receive     */
/*        not yet received data (in which case the rightmost byte of          */
/*        is meaningless).                                                    */

#define STATUSCOUNT_LOCAL       0x0100
#define STATUSCOUNT_DATAORIGIN  0x0200
#define STATUSCOUNT_RCVBUFSIZE  0x0300
#define STATUSCOUNT_ZERO        0x0400

#define SET_STATUSCOUNT_ISLOCAL(X) \
    { \
        (X) = ((X)&0x00ff) | STATUSCOUNT_LOCAL; \
    }

#define SET_STATUSCOUNT_ISDATAORIGIN(X) \
    { \
        (X) = ((X)&0x00ff) | STATUSCOUNT_DATAORIGIN; \
    }

#define SET_STATUSCOUNT_ISRCVBUFSIZE(X) \
    { \
        (X) = ((X)&0x00ff) | STATUSCOUNT_RCVBUFSIZE; \
    }
#define SET_STATUSCOUNT_ISZERO(X) \
    { \
        (X) = ((X)&0x00ff) | STATUSCOUNT_ZERO; \
    }

#define STATUSCOUNT_ISLOCAL(X)      (((X) & (0xff00)) == STATUSCOUNT_LOCAL)
#define STATUSCOUNT_ISDATAORIGIN(X) (((X) & (0xff00)) == STATUSCOUNT_DATAORIGIN)
#define STATUSCOUNT_ISRCVBUFSIZE(X) (((X) & (0xff00)) == STATUSCOUNT_RCVBUFSIZE)
#define STATUSCOUNT_ISZERO(X)       (((X) & (0xff00)) == STATUSCOUNT_ZERO)

#define STATUSCOUNT_EXTRACT_FORMAT(X) ((X) & 0xff)
/* end status->private macros   */
/* end NICK */

#define SEND_CONTIG_HANDLER_ID			0
#define SSEND_DONE_ID				1
#define SEND_DATATYPE_HANDLER_ID		2
#define SSEND_DATATYPE_HANDLER_ID		3
#define PUBLICIZE_NODES_HANDLER_ID		4
#define PUBLICIZE_NODES_REPLY_HANDLER_ID	5
#define PUBLICIZE_FORMATS_HANDLER_ID		6
#define PUBLICIZE_FORMATS_REPLY_HANDLER_ID	7
#define ABORT_HANDLER_ID	                8

#define HANDLER_TABLE_SIZE 9

#endif
