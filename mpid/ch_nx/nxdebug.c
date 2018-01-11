






/*
 *  $Id: chdebug.c,v 1.3 1995/09/18 21:11:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: chdebug.c,v 1.3 1995/09/18 21:11:31 gropp Exp $";
#endif /* lint */

#include "mpid.h"

#ifdef MPID_USE_RNDV
/* Request packets are only defined if MPID_USE_RNDV is */
MPID_NX_Rndv_print_pkt( fp, pkt )
FILE *fp;
MPID_PKT_T *pkt;
{
if (pkt->head.mode != MPID_PKT_OK_TO_SEND) {
    fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tsend_id    = %d\n\
\tsend_hndl  = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->request_pkt.send_id, pkt->request_pkt.send_handle );
    }
else {
    fprintf( fp, "\
\tsend_id    = %d\n\
\trecv_hndl  = %d\n\
\tmode       = ", 
	pkt->sendok_pkt.send_id, pkt->sendok_pkt.recv_handle );
    }
}
#endif

int MPID_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
fprintf( fp, "[%d] PKT =\n", MPID_MyWorldRank );
switch (pkt->head.mode) {
    case MPID_PKT_SYNC_ACK:
    fprintf( fp, "\
\tsync_id    = %d\n", pkt->sync_ack_pkt.sync_id );
    break; 
    case MPID_PKT_SHORT:
    case MPID_PKT_LONG:
    case MPID_PKT_SHORT_SYNC:
    case MPID_PKT_LONG_SYNC:
    case MPID_PKT_SHORT_READY:
    case MPID_PKT_LONG_READY:
fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank );
    break;
#ifdef MPID_USE_RNDV
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_REQUEST_SEND_READY:
    case MPID_PKT_OK_TO_SEND:
    MPID_NX_Rndv_print_pkt( fp, pkt );
    break;
#endif
#ifdef MPID_USE_GET
    case MPID_PKT_DO_GET:
    case MPID_PKT_DO_GET_SYNC:
    case MPID_PKT_DONE_GET:
    case MPID_PKT_CONT_GET:
    MPID_NX_Get_print_pkt( fp, pkt );
    break;
#endif
    default:
    fprintf( fp, "\n" );
    }
MPID_Print_mode( fp, pkt );
fputs( "\n", fp );
return MPI_SUCCESS;
}

void MPID_NX_Get_print_pkt( fp, pkt )
FILE       *fp;
MPID_PKT_T *pkt;
{
fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tcur_offset = %d\n\
\tlen_avail  = %d\n\
\tsend_id    = %d\n\
\trecv_id    = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank,
	pkt->get_pkt.cur_offset, pkt->get_pkt.len_avail, pkt->get_pkt.send_id,
	pkt->get_pkt.recv_id );
}

int MPID_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
char *modename=0;
int  sync_id=0;
switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
    fputs( "short", fp );
    break;
    case MPID_PKT_LONG:
    fputs( "long", fp );
    break;
    case MPID_PKT_SHORT_SYNC:
    sync_id  = pkt->short_sync_pkt.sync_id;
    modename = "sync";
#ifndef MPID_USE_RNDV
    case MPID_PKT_LONG_SYNC:
    sync_id  = pkt->long_sync_pkt.sync_id;
    modename = "long sync";
    break;
#endif
    case MPID_PKT_SHORT_READY:
    fputs( "short ready", fp );
    break;
    case MPID_PKT_LONG_READY:
    fputs( "long ready", fp );
    break;
    case MPID_PKT_SYNC_ACK:
    modename = "syncack";
    sync_id = pkt->sync_ack_pkt.sync_id;
    case MPID_PKT_COMPLETE_SEND:
    fputs( "complete send", fp );
    break;
    case MPID_PKT_COMPLETE_RECV:
    fputs( "complete recv", fp );
    break;
    case MPID_PKT_REQUEST_SEND:
    fputs( "request send", fp );
    break;
    case MPID_PKT_OK_TO_SEND:
    fputs( "ok to send", fp );
    break;
    case MPID_PKT_READY_ERROR:
    fputs( "ready error", fp );
    break;
    case MPID_PKT_DO_GET:
    fputs( "do get", fp );
    break; 
    case MPID_PKT_DO_GET_SYNC:
    fputs( "do get sync", fp );
    break; 
    case MPID_PKT_DONE_GET:
    fputs( "done get", fp );
    break;
    case MPID_PKT_CONT_GET:
    fputs( "continue get", fp );
    break;
    default:
    fprintf( fp, "Mode %d is unknown!\n", pkt->short_pkt.mode );
    break;
    }
/* if (MPID_MODE_HAS_XDR(pkt)) fputs( "xdr", fp ); */

if (modename) {
    fputs( modename, fp );
    fprintf( fp, " - id = %d", sync_id );
    }
return MPI_SUCCESS;
}
    
void MPID_NX_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
int i; char *aa = (char *)address;

if (msg)
    printf( "[%d]%s\n", MPID_MyWorldRank, msg );
if (len < 78 && address) {
    for (i=0; i<len; i++) {
	printf( "%x", aa[i] );
	}
    printf( "\n" );
    }
fflush( stdout );
}
