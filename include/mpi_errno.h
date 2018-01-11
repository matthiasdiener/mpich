/*
 *  $Id: mpi_errno.h,v 1.12 1994/06/07 21:30:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* error codes for MPI programs
   MAKE SURE YOU UPDATE mpif.h if you change this file! */

#ifndef _MPI_ERRNO
#define _MPI_ERRNO

/* error return codes */
#define MPI_SUCCESS          0      /* Successful return code */
#define MPI_ERR_EXHAUSTED    1      /* Memory exhausted */
#define MPI_ERR_TAG          2      /* Invalid tag argument */
#define MPI_ERR_COMM_NULL    3      /* NULL communicator argument 
				       passed to function */
#define MPI_ERR_COMM_INTER   4      /* Intercommunicator is not allowed 
				       in function */
#define MPI_ERR_COMM_INTRA   5      /* Intracommunicator is not allowed 
				       in function */
#define MPI_ERR_ARG          6      /* Invalid argument */
#define MPI_ERR_BUFFER       7      /* Invalid buffer pointer */
#define MPI_ERR_COUNT        8      /* Invalid count argument */
#define MPI_ERR_TYPE         9      /* Invalid datatype argument */
#define MPI_ERR_ROOT        10      /* Invalid root */
#define MPI_ERR_OP          11      /* Invalid operation */
#define MPI_ERR_ERRORCODE   12      /* Invalid error code */
#define MPI_ERR_GROUP       13      /* Null group passed to function */
#define MPI_ERR_RANK        14      /* Invalid rank */
#define MPI_ERR_TOPOLOGY    15      /* Invalid topology */
#define MPI_ERR_DIMS        16      /* Illegal dimension argument */
#define MPI_ERR_NULL        17      /* Null parameter */
#define MPI_ERR_UNKNOWN     18      /* Unknown error */
#define MPI_ERR_REQUEST     19      /* illegal mpi_request handle */
#define MPI_ERR_LIMIT       20      /* limit reached */
#define MPI_ERR_INTERN      21      /* internal error code    */
#define MPI_ERR_NOMATCH     22      /* no recv posted for ready send */
#define MPI_ERR_TRUNCATE    23      /* message truncated on receive */
#define MPI_ERR_BAD_ARGS    24
#define MPI_ERR_INIT        25      /* MPI_INIT already called */
#define MPI_ERR_PERM_KEY    26      /* Can't free a perm key */
#define MPI_ERR_BUFFER_EXISTS 27
#define MPI_ERR_COMM        28      /* Invalid communicator */
#define MPI_ERR_PERM_TYPE   29      /* Can't free a perm type */
#define MPI_ERR_OTHER       30      /* Other error; use Error_string */
#define MPI_ERR_LASTCODE    31      /* Last error code -- always at end */

#endif /* _MPI_ERRNO */




