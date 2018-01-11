/*
 *  $Id: mpi_ad.h,v 1.21 1995/06/30 17:34:21 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* user include file with dependencies on mpir.h */

#ifndef _MPI_INCLUDE_AD
#define _MPI_INCLUDE_AD

/* SGI change ... */
#include "mpi.h"
#include "dmpi.h"

/* MPI combination function */
#define MPIR_OP_COOKIE 0xca01beaf
struct MPIR_OP {
  MPI_User_function *op;
  MPIR_COOKIE 
  int               commute;
  int               permanent;
};

#endif





