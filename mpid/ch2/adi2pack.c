/*
 *  $Id: adi2pack.c,v 1.3 1996/07/17 18:04:59 gropp Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include "mpid.h"
#include "mpidmpi.h"

/* 
 * This file contains the interface to the Pack/Unpack routines.
 * It includes not only the ADI Pack/Unpack routines, but the
 * service routines used to implement noncontiguous operations
 *
 * For now, these use the "old" routines in src/dmpi.
 */

/*
 * Compute the msgrep and msgform for a message to OR FROM a particular
 * partner.  The partner is the GLOBAL RANK in COMM_WORLD, not the relative
 * rank in the communicator.
 */
void MPID_Msg_rep( comm, partner, datatype, msgrep, msgact )
MPI_Comm        comm;
int             partner;
MPI_Datatype    datatype;
MPID_Msgrep_t   *msgrep;
MPID_Msg_pack_t *msgact;
{
#ifndef MPID_HAS_HETERO
	*msgrep = MPID_MSGREP_RECEIVER;
	*msgact = MPID_MSG_OK;
#else
    /* Check for homogeneous communicator */
    if (comm->msgform == MPID_MSG_OK) {
	*msgrep = MPID_MSGREP_RECEIVER;
	*msgact = MPID_MSG_OK;
	return;
    }

    /* Packed data is a special case (data already in correct form) */
    if (datatype->dte_type == MPIR_PACKED) {
	switch (comm->msgform) {
	case MPID_MSG_OK: *msgrep = MPID_MSGREP_RECEIVER; break;
	case MPID_MSG_SWAP: *msgrep = MPID_MSGREP_SENDER; break;
	case MPID_MSG_XDR: *msgrep = MPID_MSGREP_XDR; break;
	}
	*msgact = MPID_MSG_OK;
	return;
    }

    /* Check for collective or point-to-point */
    if (partner != MPI_ANY_SOURCE) {
	if (MPID_procinfo[MPID_MyWorldRank].byte_order == MPID_H_XDR ||
	    MPID_procinfo[partner].byte_order == MPID_H_XDR) {
	    *msgrep = MPID_MSGREP_XDR;
	    *msgact = MPID_MSG_XDR;
	}
	else if (MPID_procinfo[MPID_MyWorldRank].byte_order != 
		 MPID_procinfo[partner].byte_order) {
	    *msgrep = MPID_MSGREP_RECEIVER;
	    *msgact = MPID_MSG_SWAP;
	}
	else {
	    *msgrep = MPID_MSGREP_RECEIVER;
	    *msgact = MPID_MSG_OK;
	}
    }
    else {
	switch (comm->msgform) {
	case MPID_MSG_OK: *msgrep = MPID_MSGREP_RECEIVER; break;
	case MPID_MSG_SWAP: *msgrep = MPID_MSGREP_SENDER; break;
	case MPID_MSG_XDR: *msgrep = MPID_MSGREP_XDR; break;
	}
	*msgact = (MPID_Msg_pack_t)((comm->msgform != MPID_MSG_OK) ? 
	    MPID_MSG_XDR : MPID_MSG_OK);
    }
#endif
}

void MPID_Msg_act( comm, partner, datatype, msgrep, msgact )
MPI_Comm        comm;
int             partner;
MPI_Datatype    datatype;
MPID_Msgrep_t   msgrep;
MPID_Msg_pack_t *msgact;
{
    switch (msgrep) {
    case MPID_MSGREP_RECEIVER:
	*msgact = MPID_MSG_OK;
	break;
    case MPID_MSGREP_XDR:
	*msgact = MPID_MSG_XDR;
       break;
    case MPID_MSGREP_SENDER:
	/* Could check here for byte swap */
	(void) MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_MSGREP_SENDER,
			  "Error in packing data" );
	fprintf( stderr, "WARNING - sender format not ready!\n" );
	*msgact = MPID_MSG_OK;
	break;	
    default:
	MPIR_ERROR_PUSH_ARG(&msgrep);
	(void) MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_MSGREP_UNKNOWN,
			  "Error in packing data" );
    }
}

void MPID_Pack_size( count, datatype, msgact, size )
int             count, *size;
MPI_Datatype    datatype;
MPID_Msg_pack_t msgact;
{
    int contig_size;
#if defined(MPID_HAS_HETERO) && defined(HAS_XDR)
    /* Only XDR has a different length */
    if (msgact == MPID_MSG_XDR) {
	MPIR_GET_REAL_DATATYPE(datatype);
	*size = MPID_Mem_XDR_Len( datatype, count );
    }
    else
#endif
    {
	MPIR_DATATYPE_GET_SIZE(datatype,contig_size);
	if (contig_size > 0) 
	    *size = contig_size * count;
	else {
	    /* Our pack routine is "tight" */
	    MPIR_GET_REAL_DATATYPE(datatype);
	    *size = datatype->size * count;
	}
    }
}

void MPID_Pack( src, count, datatype, dest, maxcount, position, 
           comm, partner, msgrep, msgact, error_code )
void            *src, *dest;
int             count, maxcount, *position, partner, *error_code;
MPID_Msgrep_t   msgrep;
MPI_Datatype    datatype;
MPI_Comm        comm;
MPID_Msg_pack_t msgact;
{
int (*packcontig) ANSI_ARGS((unsigned char *, unsigned char *, MPI_Datatype, 
			     int, void * )) = 0;
void *packctx = 0;
int  outlen;
#ifdef HAS_XDR
XDR xdr_ctx;    
#endif

MPIR_GET_REAL_DATATYPE(datatype);

#ifdef MPID_HAS_HETERO
    switch (msgact) {
#ifdef HAS_XDR
    case MPID_MSG_XDR:
	MPID_Mem_XDR_Init( dest, maxcount, XDR_ENCODE, &xdr_ctx );
	packctx = (void *)&xdr_ctx;
	packcontig = MPID_Type_XDR_encode;
	break;
#endif
    case MPID_MSG_SWAP:
	packcontig = MPID_Type_swap_copy;
	break;
    case MPID_MSG_OK:
	/* use default routines */
	break;
    }
#endif
    outlen = 0;
    *error_code = MPIR_Pack2( src, count, datatype, packcontig, packctx, dest, 
			      &outlen, position );
#if HAS_XDR
    if (packcontig == MPID_Type_XDR_encode)
	MPID_Mem_XDR_Free( &xdr_ctx );
#endif    
}

void MPID_Unpack( src, maxcount, msgrep, in_position, 
		  dest, count, datatype, out_position,
		  comm, partner, error_code )
void          *src, *dest;
int           maxcount, *in_position, count, *out_position, partner, 
              *error_code;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPID_Msgrep_t msgrep;
{
    int act_len = 0;
    int dest_len = 0;

    MPIR_GET_REAL_DATATYPE(datatype);
    *error_code = MPIR_Unpack( comm, (char *)src + *in_position, 
			       maxcount - *in_position, count,  datatype, 
			       msgrep, dest, &act_len, &dest_len );
    *in_position += act_len;
    *out_position += dest_len;
}

