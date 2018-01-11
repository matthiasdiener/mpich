/*
 *  $Id: t3ddebug.h,v 1.2 1995/06/07 06:18:43 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DDEBUG_INCLUDED
#define _T3DDEBUG_INCLUDED


/***************************************************************************
  Cray T3D device error codes
 ***************************************************************************/
#define T3D_SUCCESS            0       /* Successful operation */
#define T3D_ERR_OTHER          1       /* Other error not in this list */
#define T3D_ERR_UNKNOWN        2       /* Unknown error */
#define T3D_ERR_MEMORY         3       /* Memory allocation or free error */
#define T3D_ERR_INTERN         4       /* Internal MPI error */
#define T3D_ERR_NOTIMPLEMENTED 5       /* feature not implemented */
#define T3D_ERR_SEND           6       /* error in send code */
#define T3D_ERR_RECV           7       /* error in recv code */
#define T3D_ERR_LASTCODE       8       /* Last error code, placeholder */


/***************************************************************************
  MPID_DEBUG_ALL
    If this macro is defined, all debugging statements are included.
 ***************************************************************************/
/*#define MPID_DEBUG_ALL*/


/***************************************************************************
  These macros cause debugging code to be defined when they are defined.  If
  MPID_DEBUG_ALL is defined, these have no effect since all debugging 
  statements are included.
 ***************************************************************************/
/*#define MPID_DEBUG_SEND*/
/*#define MPID_DEBUG_RECV*/
/*#define MPID_DEBUG_SYNC*/
/*#define MPID_DEBUG_INIT*/
/*#define MPID_DEBUG_COLL*/
/*#define MPID_DEBUG_EVENT*/
/*#define MPID_DEBUG_PROBE*/
/*#define MPID_DEBUG_DEVICE*/

#endif
