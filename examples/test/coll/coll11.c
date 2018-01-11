#include "mpi.h"

void addem(invec, inoutvec, len, dtype)
int *invec, *inoutvec, *len;
MPI_Datatype *dtype;
{
  int i;
  for ( i=0; i<*len; i++ ) 
    inoutvec[i] += invec[i];
}

#define BAD_ANSWER 100000

void assoc(invec, inoutvec, len, dtype)
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
    MPI_Op           op_assoc, op_addem;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    data = rank;

    correct_result = 0;
    for (i=0;i<=rank;i++)
      correct_result += i;

    MPI_Scan ( &data, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if (result != correct_result) errors++;

    MPI_Scan ( &data, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if (result != correct_result) errors++;

    data = rank;
    result = -100;
    MPI_Op_create( (MPI_User_function *)assoc, 0, &op_assoc );
    MPI_Op_create( (MPI_User_function *)addem, 1, &op_addem );
    MPI_Scan ( &data, &result, 1, MPI_INT, op_addem, MPI_COMM_WORLD );
    if (result != correct_result) errors++;

    MPI_Scan ( &data, &result, 1, MPI_INT, op_addem, MPI_COMM_WORLD );
    if (result != correct_result) errors++;

    result = -100;
    data = rank;
    MPI_Scan ( &data, &result, 1, MPI_INT, op_assoc, MPI_COMM_WORLD );
    if (result == BAD_ANSWER) errors++;

    MPI_Op_free( &op_assoc );
    MPI_Op_free( &op_addem );

    MPI_Finalize();
    if (errors)
      printf( "[%d] done with ERRORS(%d)!\n", rank, errors );
    return errors;
}
