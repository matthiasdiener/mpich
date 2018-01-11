#include "mpi.h"

#define BAD_ANSWER 100000

int assoc(invec, inoutvec, len, dtype)
int *invec, *inoutvec, *len;
MPI_Datatype *dtype;
{
  int i;
  for ( i=0; i<*len; i++ )  {
    if (inoutvec[i] > invec[i] )
      inoutvec[i] = BAD_ANSWER;
    else 
      inoutvec[i] = invec[i];
  }
  return (1);
}

int main( argc, argv )
int argc;
char **argv;
{
    int              rank, size, i;
    MPI_Request      handle;
    MPI_Status       status;
    int              data;
    int              errors=0;
    int              result = -100;
    int              correct_result;
    MPI_Op           op;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    data = rank;

    MPI_Op_create( (MPI_User_function*)assoc, 0, &op );
    MPI_Reduce ( &data, &result, 1, MPI_INT, op, size-1, MPI_COMM_WORLD );
    MPI_Bcast  ( &result, 1, MPI_INT, size-1, MPI_COMM_WORLD );
    MPI_Op_free( &op );
    if (result == BAD_ANSWER) errors++;

    MPI_Finalize();
    if (errors)
      printf( "[%d] done with ERRORS(%d)!\n", rank, errors );
    return errors;
}
