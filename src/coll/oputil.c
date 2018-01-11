/*
 *  $Id: oputil.c,v 1.1 1996/04/12 20:15:07 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpiops.h"

/* 
 * This file contains the routines to setup the MPI ops
 */

/* static space for predefined combination functions */
struct MPIR_OP MPIR_I_MAX, MPIR_I_MIN, MPIR_I_SUM, MPIR_I_PROD, 
              MPIR_I_LAND, MPIR_I_BAND, MPIR_I_LOR, MPIR_I_BOR, MPIR_I_LXOR, 
              MPIR_I_BXOR, MPIR_I_MINLOC, MPIR_I_MAXLOC;

/* 
   This function is used to initialize the MPI_Op's; it can be used
   for the predefined operations as well 
 */
void MPIR_Op_setup( function, commute, is_perm, newop )
MPI_User_function *function;
int               commute, is_perm;
MPI_Op            newop;
{
  MPIR_SET_COOKIE(newop,MPIR_OP_COOKIE)
  newop->commute   = commute;
  newop->op        = function;
  newop->permanent = is_perm;
}

