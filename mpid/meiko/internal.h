/****************************************************************************/
/*                                                                          */
/* Copyright    **    **  ********  ********  **    **   ******     Limited */
/* Copyright    ***  ***  ********  ********  **   **   ********    Limited */
/* Copyright    ********  **           **     **  **    **    **    Limited */
/* Copyright    ** ** **  ******       **     ****      **    **    Limited */
/* Copyright    **    **  **           **     ** **     **    **    Limited */
/* Copyright    **    **  **           **     **  **    **    **    Limited */
/* Copyright    **    **  ********  ********  **   **   ********    Limited */
/* Copyright    **    **  ********  ********  **    **   ******     Limited */
/*                                                                          */
/*                      **       ****     ******    ******                  */
/*                     ***      ******   ********  ********                 */
/*                    ****     **    **  **    **  **   ***                 */
/*                      **      *******   *******      ***                  */
/*                      **           **        **    ***                    */
/*                      **           **        **   ***                     */
/*                      **     ********  ********  ********                 */
/*                      **      ******    ******   ********                 */
/*                                                                          */
/* Author:  Jim Cownie - September 1992                                     */
/****************************************************************************/

/*
 * Copyright (c) 1992 by Meiko Ltd.
 */

/* $Id: internal.h,v 1.1.1.2 1995/05/18 11:16:34 jim Exp $ */
/* $Source: /home/cvs/master/cs2parts/mpi/mpid/meiko/internal.h,v $ */

#ifndef _MPSC_INTERNAL_H
#define _MPSC_INTERNAL_H

#ifdef	__cplusplus
extern "C" {
#endif
#pragma ident "$Id: internal.h,v 1.1.1.2 1995/05/18 11:16:34 jim Exp $"

#define MPSC_DFLT_TRACEFILE	"LIBMPSC_TRACE"
#define MPSC_DFLT_TRACEBUF	10000

#define MPSC_MINDESCS	64			/* min # descs to allocate */
#define MPSC_TXCHUNK     8                      /* max number of sends in csend(,,,-1) */

#define MPSC_EBASE	  1000			/* exception codes */
#define MPSC_EINIT	  MPSC_EBASE + 0
#define MPSC_ENODESCS	  MPSC_EBASE + 1
#define MPSC_EBADPID	  MPSC_EBASE + 2
#define MPSC_EBADEVENT	  MPSC_EBASE + 3
#define MPSC_ENODMAS	  MPSC_EBASE + 4
#define MPSC_EBADNODE	  MPSC_EBASE + 5
#define MPSC_EINVAL	  MPSC_EBASE + 6
#define MPSC_EBADTAG	  MPSC_EBASE + 7
#define MPSC_EBADPTYPE    MPSC_EBASE + 8
#define MPSC_EBADREQ      MPSC_EBASE + 9
#define MPSC_WLEFTOVEROP  MPSC_EBASE +10
#define MPSC_WLEFTOVERMSG MPSC_EBASE +11

struct msgInfo
{
    int info_type;
    int info_count;
    int info_node;
    int info_ptype;        /* Always zero */
};

typedef struct mpsc_desc
{
   ELAN_EVENT     *d_tx;
   ELAN_EVENT     *d_rx;
   struct msgInfo *d_msginfo;
   struct mpsc_desc *d_suc;
}               MPSC_DESC;

typedef struct
{
   int             proc;           /* Who am I ?                      */
   int		   ncube;          /* How many like me are there ?    */
   int             nproc;          /* How many TOTAL processes ?      */
   int		   hostVp;         /* Address of host (if he's there) */
   int             pid;            /* The stupid IPSC pid             */
   EW_TPORT       *tport;          /* What we actually use for moving messages */
   unsigned int    max_buffered;   /* How large a message to allow to be buffered */
   EW_EXH	   oldHandler;     
   MPSC_DESC      *freeDescs;      /* List of free descriptors        */
   int             nDescs;         /* How many are there in all       */
   struct msgInfo  info;           /* The implicit tag, sender etc    */
   int		   mclock0;        /* The epoch                       */
   EW_GROUP       *group;          /* The group with everyone in      */
}               MPSC_STATE;

#define infocount  info.info_count
#define infonode   info.info_node
#define infotype   info.info_type

#define TXFLAGS(len)  (((len) < mpsc_state.max_buffered) ? 0 : EW_TPORT_TXSYNC)

extern MPSC_STATE mpsc_state;

#define MPSC2VP(x)	(ew_groupMember(ew_base.allGroup,(x)))

/* Unsigned comparison allows a test for 0 <= x <= y to be done all in one go */
#define INVALIDNODE(node)   (((unsigned int)(node)) >= ((unsigned int)mpsc_state.nproc))

#define IAMHOST() (ew_state.vp == mpsc_state.hostVp)
#define HOSTED()  (mpsc_state.hostVp != ELAN_INVALID_PROCESS)

extern int mpsc_hostInit(ELAN_CAPABILITY, const int);
#pragma weak mpsc_resourceInit
extern void mpsc_resourceInit(void);

#ifdef	__cplusplus
}
#endif

#endif
