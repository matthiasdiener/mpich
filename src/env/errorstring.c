/*
 *  $Id: errorstring.c,v 1.8 1996/01/29 21:22:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: errorstring.c,v 1.8 1996/01/29 21:22:12 gropp Exp $";
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

.N fortran
@*/
int MPI_Error_string( errorcode, string, resultlen )
int  errorcode, *resultlen;
char *string;
{
int error_case = errorcode & ~MPIR_ERR_CLASS_MASK;
int mpi_errno = MPI_SUCCESS;
/* 
   error_case contains any additional details on the cause of the error.
 */

string[0] = 0;
switch (errorcode & MPIR_ERR_CLASS_MASK) {
    case MPI_SUCCESS:
        strcpy( string, "No error" );
	break;
    case MPI_ERR_BUFFER:
	strcpy( string, "Invalid buffer pointer" );
	if (error_case == MPIR_ERR_BUFFER_EXISTS) {
	    strcat( string,
		": Can not attach buffer when a buffer already exists" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_USER_BUFFER_EXHAUSTED) {
	    strcat( string, 
		   ": Insufficent space available in user-defined buffer" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_BUFFER_ALIAS) {
	    strcat( string, 
                 ": Arguments must specify different buffers (no aliasing)" );
	    error_case = 0;
	    }
	break;
    case MPI_ERR_COUNT:
        strcpy( string, "Invalid count argument" );
	break;
    case MPI_ERR_TYPE:
        strcpy( string, "Invalid datatype argument" );
	if (error_case == MPIR_ERR_UNCOMMITTED) {
	    strcat( string, ": datatype has not been committed" );
	    error_case = 0;
	    }
	break;
    case MPI_ERR_TAG:
	strcpy( string, "Invalid message tag" );
	break;
    case MPI_ERR_COMM:
	strcpy( string, "Invalid communicator" );
        if (error_case == MPIR_ERR_COMM_NULL) {
	    strcat( string, ": Null communicator" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_COMM_INTER) {
	    strcat( string, ": Intercommunicator is not allowed" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_COMM_INTRA) {
	    strcat( string, ": Intracommunicator is not allowed" );
	    error_case = 0;
	    }
	break;
    case MPI_ERR_RANK:
        strcpy( string, "Invalid rank" );
	break;
    case MPI_ERR_ROOT:
	strcpy( string, "Invalid root" );
	break;
    case MPI_ERR_GROUP:
	strcpy( string, "Invalid group passed to function" );
	break;
    case MPI_ERR_OP:
	strcpy( string, "Invalid operation" );
	if (error_case == MPIR_ERR_NOT_DEFINED) {
	    strcat( string, ": not defined for this datatype" );
	    error_case = 0;
	    }
	break;
    case MPI_ERR_TOPOLOGY:
	strcpy( string, "Invalid topology" );
	break;
    case MPI_ERR_DIMS:
        strcpy( string, "Illegal dimension argument" );
	break;
    case MPI_ERR_ARG:
	strcpy( string, "Invalid argument" );
	if (error_case == MPIR_ERR_ERRORCODE) {
	    strcat( string, ": Invalid error code" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_NULL) {
	    strcat( string, ": Null parameter" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_PERM_KEY) {
	    strcat( string, ": Can not free permanent attribute key" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_PERM_TYPE) {
	    strcat( string, ": Can not free permanent data type" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_PERM_OP) {
	    strcat( string, ": Can not free permanent MPI_Op" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_FORTRAN_ADDRESS_RANGE) {
	    strcat( string, 
": Address of location given to MPI_ADDRESS does not fit in Fortran integer" );
	    error_case = 0;
	    }
	break;

/* 
    case MPI_ERR_BAD_ARGS:
	strcpy( string, "Invalid arguments to MPI routine" );
	break;
*/

    case MPI_ERR_UNKNOWN:
	strcpy( string, "Unknown error" );
	break;
    case MPI_ERR_TRUNCATE:
	strcpy( string, "Message truncated" );
	break;
    case MPI_ERR_OTHER:
	/* This is slightly different from the other error codes/classes, 
	   in that there is no default message */
	if (error_case == MPIR_ERR_LIMIT) {
	    strcpy( string, "System resource limit exceeded" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_NOMATCH) {
	    strcpy( string, "Ready send had no matching receive" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_INIT) {
	    strcpy( string, "Can not call MPI_INIT twice!" );
	    error_case = 0;
	    }
	else if (error_case == MPIR_ERR_PRE_INIT) {
	    strcpy( string, 
		   "MPI_INIT must be called before other MPI routines" );
	    error_case = 0;
	    }
	else {
	    strcpy( string, "Unclassified error" );
	    }
	break;
    case MPI_ERR_INTERN:
	strcpy( string, "Internal MPI error!" );
	if (error_case == MPIR_ERR_EXHAUSTED) {
	    strcat( string, ": Out of internal memory" );
	    error_case = 0;
	    }
	break;
    case MPI_ERR_IN_STATUS:
	strcpy( string, "Error code is in status" );
	break;
    case MPI_ERR_PENDING:
	strcpy( string, "Pending request (no error)" );
        break;

    case MPI_ERR_REQUEST:
	strcpy( string, "Illegal mpi_request handle" );
	break;

    default:
	strcpy( string, "Unexpected error value!" );
	*resultlen = strlen( string );
	mpi_errno = MPI_ERR_ARG;
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_ERROR_STRING" );
    }

if (error_case != 0) {
    /* Unrecognized error case */
    strcat( string, ": unrecognized error code in error class" );
    }

*resultlen = strlen( string );
return mpi_errno;
}
