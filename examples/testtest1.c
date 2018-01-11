/* 
   This is a test of MPI_Test to receive a message of known length (used as a
   server)
 */
#include "mpi.h"

main(argc, argv) 
int  argc;
char **argv;
{
int data, to, from, tag, len, maxlen, np, myid, flag, dest, src;
MPI_Status status, status1;
MPI_Request request;

MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &myid );
MPI_Comm_size( MPI_COMM_WORLD, &np );

if (argc > 1 && argv[1] && strcmp( "-alt", argv[1] ) == 0) {
    src  = np - 1;
    dest = 0;
    }
else {
    dest = np - 1;
    src  = 0;
    }

if (myid == src) {
    to   = dest;
    len  = 1;
    tag = 2000;
    data = 100;
    printf( "About to send\n" );
    MPI_Send( &data, 1, MPI_INT, to, tag, MPI_COMM_WORLD );
    tag = 2001;
    data = 0;
    printf( "About to send 'done'\n" );
    MPI_Send( &data, 1, MPI_INT, to, tag, MPI_COMM_WORLD );
    }
else {
    /* Server loop */
    while (1) {
	tag    = MPI_ANY_TAG;
	from   = MPI_ANY_SOURCE;
	MPI_Irecv( &data, 1, MPI_INT, from, tag, MPI_COMM_WORLD,
		   &request );
	/* Should really use MPI_Wait, but functionally this will work
	   (it is less efficient, however) */
	do {		
	    MPI_Test( &request, &flag, &status );
	    } while (!flag);
	if (status.MPI_TAG == 2001) {
	    printf( "Received terminate message\n" );
	    break;
	    }
	if (status.MPI_TAG == 2000) {
	    MPI_Get_count( &status, MPI_INT, &maxlen );
	    if (maxlen != 1) {
		fprintf( stderr, "Should have received one integer; got %d\n",
			maxlen );
		}
	    /* Check data: */
	    if (data != 100) {
		fprintf( stderr, 
		   "Did not receive correct data: %d instead of %d\n", 
			data, 100 );
		}
	    }
	}
    }
MPI_Barrier( MPI_COMM_WORLD );
printf( "%d done\n", myid );
MPI_Finalize();
}
