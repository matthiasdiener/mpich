/*
 *  $Id: errorstring.c,v 1.3 1995/07/25 02:47:58 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: errorstring.c,v 1.3 1995/07/25 02:47:58 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
   MPI_Error_string - Return a string for a given error code

Input Parameters:
. errorcode - Error code returned by an MPI routine or an MPI error class

Output Parameter:
. string - Text that corresponds to the errorcode 
. resultlen - Length of string 

Notes:  Error codes are the values return by MPI routines (in C) or in the
'ierr' argument (in Fortran).  These can be converted into error classes 
with the routine 'MPI_Error_class'.  
@*/
int MPI_Error_string( errorcode, string, resultlen )
int  errorcode, *resultlen;
char *string;
{
int error_case = errorcode & ~MPIR_ERR_CLASS_MASK;
int mpi_errno = MPI_SUCCESS;
/* 
   error_case contains any additional details on the cause of the error.
   The first such examples are for MPI_ERR_TYPE (invalid datatype) 
 */

switch (errorcode & MPIR_ERR_CLASS_MASK) {
    case MPI_SUCCESS:
        strcpy( string, "No error" );
	break;
    case MPI_ERR_EXHAUSTED: 
        strcpy( string, "Out of internal memory" );
        break;
    case MPI_ERR_TAG:
	strcpy( string, "Invalid message tag" );
	break;
    case MPI_ERR_COMM_NULL:
	strcpy( string, 
		 "NULL communicator argument passed to function" );
	break;
    case MPI_ERR_COMM_INTRA:
	strcpy( string,
		 "Intracommunicator is not allowed in function" );
	break;
    case MPI_ERR_COMM_INTER:
	strcpy( string,
		 "Intercommunicator is not allowed in function" );
	break;
    case MPI_ERR_ARG:
	strcpy( string, "Invalid argument" );
	break;
    case MPI_ERR_BUFFER:
	if (error_case == MPIR_ERR_USER_BUFFER_EXHAUSTED)
	    strcpy( string, 
		   "Insufficent space available in user-defined buffer" );
	else if (error_case == MPIR_ERR_BUFFER_ALIAS)
	    strcpy( string, 
		   "Arguments must specify different buffers (no aliasing)" );
	else 
	    strcpy( string, "Invalid buffer pointer" );
	break;
    case MPI_ERR_COUNT:
        strcpy( string, "Invalid count argument" );
	break;
    case MPI_ERR_TYPE:
        strcpy( string, "Invalid datatype argument" );
	if (error_case == MPIR_ERR_UNCOMMITTED) 
	    strcat( string, ": datatype has not been committed" );
	break;
    case MPI_ERR_ROOT:
	strcpy( string, "Invalid root" );
	break;
    case MPI_ERR_OP:
	strcpy( string, "Invalid operation" );
	if (error_case == MPIR_ERR_NOT_DEFINED) 
	    strcat( string, ": not defined for this datatype" );
	break;
    case MPI_ERR_ERRORCODE:
	strcpy( string, "Invalid error code" );
	break;
    case MPI_ERR_GROUP:
	strcpy( string, "Invalid group passed to function" );
	break;
    case MPI_ERR_RANK:
        strcpy( string, "Invalid rank" );
	break;
    case MPI_ERR_TOPOLOGY:
	strcpy( string, "Invalid topology" );
	break;
    case MPI_ERR_DIMS:
        strcpy( string, "Illegal dimension argument" );
	break;
    case MPI_ERR_NULL:
        strcpy( string, "Null parameter" );
	break;
    case MPI_ERR_REQUEST:
	strcpy( string, "Illegal mpi_request handle" );
	break;
    case MPI_ERR_UNKNOWN:
	strcpy( string, "Unknown error" );
	break;
    case MPI_ERR_INTERN:
	strcpy( string, "Internal MPI error!" );
	break;
    case MPI_ERR_BAD_ARGS:
	strcpy( string, "Invalid arguments to MPI routine" );
	break;
    case MPI_ERR_INIT:
	strcpy( string, "Can not call MPI_INIT twice!" );
	break;
    case MPI_ERR_PERM_KEY:
	strcpy( string, "Can not free permanent attribute key" );
	break;
    case MPI_ERR_PERM_TYPE:
	strcpy( string, "Can not free permanent data type" );
	break;
    case MPI_ERR_PERM_OP:
	strcpy( string, "Can not free permanent MPI_Op" );
	break;
/*	    strcpy( string, "Can not free permanent MPI_Errorhandler" );
 */
    case MPI_ERR_BUFFER_EXISTS:
	strcpy( string,
		"Can not attach buffer when a buffer already exists" );
	break;
    case MPI_ERR_TRUNCATE:
	strcpy( string, "Message truncated" );
	break;
    case MPI_ERR_LIMIT:
	strcpy( string, "System resource limit exceeded" );
	break;
    case MPI_ERR_COMM:
	strcpy( string, "Invalid communicator" );
	break;
    case MPI_ERR_PRE_INIT:
	strcpy( string, "MPI_INIT must be called before other MPI routines" );
	break;
    case MPI_ERR_OTHER:
	strcpy( string, "Unclassified error" );
	break;
    default:
	strcpy( string, "Unexpected error value!" );
	*resultlen = strlen( string );
	mpi_errno = MPI_ERR_ARG;
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_ERROR_STRING" );
        break;
    }
*resultlen = strlen( string );
return mpi_errno;
}
