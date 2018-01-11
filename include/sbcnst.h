/*
 *  $Id: sbcnst.h,v 1.7 1994/09/29 21:51:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* %W% %G% */
#ifndef _SBCNST
#define _SBCNST
#ifndef _SBCNSTDEF
#if defined __STDC__
extern void *MPIR_SBinit( int, int, int ), 
             MPIR_SBfree( /* MPIR_SBHeader *, void * */ ), 
             MPIR_SBiAllocate( /* MPIR_SBHeader *, int, int */), 
            *MPIR_SBalloc( /* MPIR_SBalloc * */ ), 
             MPIR_SBPrealloc( /* MPIR_SBHeader *, int */ );
#else
extern void *MPIR_SBinit(), MPIR_SBfree(), MPIR_SBiAllocate(), 
            *MPIR_SBalloc(), MPIR_SBPrealloc();
#endif
#endif

#define SBinit BUGGG%
#define SBfree BUGGG%
#define SBiAllocate BUGGG%
#define SBalloc BUGGG%
#define SBPrealloc BUGGG%

/* Chameleon/PETSc includes memory tracing functions that can be used
   to track storage leaks.  */
#ifndef MALLOC
#if defined(DEVICE_CHAMELEON) && defined(MPIR_DEBUG_MEM)
#define MALLOC(a)    trmalloc((unsigned)(a),__LINE__,__FILE__)
#define CALLOC(a,b)  trcalloc((unsigned)(a),(unsigned)(b),__LINE__,__FILE__)
#define FREE(a)      trfree((char *)(a),__LINE__,__FILE__)
#define NEW(a)    (a *)MALLOC(sizeof(a))
/* Also replace the SB allocators so that we can get the trmalloc line/file
   tracing. */
#define MPIR_SBinit(a,b,c) ((void *)(a))
#define MPIR_SBalloc(a)    trmalloc((unsigned)(a),__LINE__,__FILE__)
#define MPIR_SBfree(a,b)   trfree((char *)(b),__LINE__,__FILE__)
#define MPIR_SBdestroy(a)
#else
/* We also need to DECLARE malloc etc here ... */
#if HAVE_STDLIB_H || STDC_HEADERS
#include <stdlib.h>
#else
#ifdef __STDC__
extern void 	*calloc(/*size_t, size_t*/);
extern void	free(/*void * */);
extern void	*malloc(/*size_t*/);
#else
extern char *malloc();
extern char *calloc();
extern int free();
#endif
#endif

#define MALLOC(a)    malloc((unsigned)(a))
#define CALLOC(a,b)  calloc((unsigned)(a),(unsigned)(b))
#define FREE(a)      free((char *)(a))
#define NEW(a)    (a *)MALLOC(sizeof(a))
#endif /*MPIR_DEBUG_MEM*/
#endif /*MALLOC*/

#endif
