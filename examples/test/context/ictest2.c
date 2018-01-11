/* ictest2.c 
   This is like ictest.c, but it creates communictors that are valid only
   at the "leaders"; other members of the local communicator are NOT
   in the remote communicator.  This is done by creating two communicators:
   0, + odd rank and even rank.  Only 0 is in in both communicators.
 */
#include "mpi.h"
#include <stdio.h>

main( argc, argv )
     int argc;
     char **argv;
{
  int size, rank, key, his_key, lrank, rsize, result;
  MPI_Comm myComm, myComm2;
  MPI_Comm myFirstComm;
  MPI_Comm mySecondComm;
  MPI_Comm evenComm, oddComm, remComm;
  int errors = 0, sum_errors;
  MPI_Status status;
  
  /* Initialization */
  MPI_Init ( &argc, &argv );
  MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
  MPI_Comm_size ( MPI_COMM_WORLD, &size);

  /* Only works for 2 or more processes */
  if (size >= 2) {
    MPI_Comm merge1, merge2, merge3, merge4;

    /* Generate membership key in the range [0,1] */
    key = rank % 2;
    /* Create the even communicator */
    MPI_Comm_split ( MPI_COMM_WORLD, key, rank, &evenComm );
    if (key == 1) {
	/* Odd rank communicator discarded */
	MPI_Comm_free( &evenComm );
	}

    /* Create the odd + 0 communicator */
    if (rank == 0) key = 1;
    MPI_Comm_split( MPI_COMM_WORLD, key, rank, &oddComm );
    if (key == 0) {
	/* Even rank communicator discarded */
	MPI_Comm_free( &oddComm );
	}
    else {
	MPI_Comm_rank( oddComm, &lrank );
	printf( "[%d] lrank in oddComm is %d (color = %d, key=%d)\n", 
	        rank, lrank, key, rank );
	}
    /* Now, choose the local and remote communicators */
    if (rank % 2) {
	/* Odd */
	myComm  = oddComm;
	remComm = evenComm;
	}
    else {
	myComm  = evenComm;
	remComm = oddComm;
	}

    /* Check that the leader is who we think he is */
    MPI_Comm_rank( myComm, &lrank );
    printf( "[%d] local rank is %d\n", rank, lrank );
    if (rank == 0) {
	int trank;
	MPI_Comm_rank( myComm, &trank );
	if (trank != 0) {
	    printf( "[%d] Comm split improperly ordered group (myComm)\n",
		    rank );
	    }
	MPI_Comm_rank( remComm, &trank );
	if (trank != 0) {
	    printf( "[%d] Comm split improperly ordered group (remComm)\n",
		    rank );
	    }
	}
    fflush(stdout);
    /* Perform the intercomm create and test it */
    MPI_Intercomm_create (myComm, 0, remComm, 0, 1, &myFirstComm );
/* temp */
    printf( "[%d] through intercom create\n", rank );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    printf( "[%d] through barrier at end of intercom create\n", rank );
/* temp */

    /* Try to dup this communicator */
    MPI_Comm_dup ( myFirstComm, &mySecondComm );

/* temp */
    printf( "[%d] through comm dup\n", rank );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    printf( "[%d] through barrier at end of comm dup\n", rank );
/* temp */

    /* Each member shares data with his "partner".  Note that process 0 in
       MPI_COMM_WORLD is sending to itself, since it is process 0 in both
       remote groups */
    MPI_Comm_rank( mySecondComm, &lrank );
    MPI_Comm_remote_size( mySecondComm, &rsize );

    printf( "[%d] lrank in secondcomm is %d and remote size is %d\n", 
	   rank, lrank, rsize );
    fflush( stdout );

    /* Send key * size + rank in communicator */
    if (lrank < rsize) {
      int myval, hisval;
      myval   = key * size + lrank;
      hisval  = -1;
      printf( "[%d] exchanging %d with %d in intercomm\n", 
	     rank, myval, lrank );
      fflush( stdout );
      MPI_Sendrecv (&myval,  1, MPI_INT, lrank, 0,
                    &hisval, 1, MPI_INT, lrank, 0, mySecondComm, &status);
      if (hisval != (lrank + (!key)*size)) {
	  printf( "[%d] expected %d but got %d\n", rank, lrank + (!key)*size,
		  hisval );
	  errors++;
	  }
      }
    
    if (errors)
      printf("[%d] Failed!\n",rank);

    /* Key is 1 for oddComm, 0 for evenComm (note both contain 0 in WORLD) */
    MPI_Intercomm_merge ( mySecondComm, key, &merge1 );
    MPI_Intercomm_merge ( mySecondComm, (key+1)%2, &merge2 );
    MPI_Intercomm_merge ( mySecondComm, 0, &merge3 );
    MPI_Intercomm_merge ( mySecondComm, 1, &merge4 );

    MPI_Comm_compare( merge1, MPI_COMM_WORLD, &result );
    if (result != MPI_SIMILAR && size > 2) {
	printf( "[%d] comparision with merge1 failed\n", rank );
	errors++;
	}

    /* Free communicators */
    MPI_Comm_free( &myComm );
    MPI_Comm_free( &myFirstComm );
    MPI_Comm_free( &mySecondComm );
    MPI_Comm_free( &merge1 );
    MPI_Comm_free( &merge2 );
    MPI_Comm_free( &merge3 );
    MPI_Comm_free( &merge4 );
  }
  else 
    printf("[%d] Failed - at least 2 nodes must be used\n",rank);

  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Allreduce( &errors, &sum_errors, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
  if (sum_errors > 0) {
      printf( "%d errors on process %d\n", errors, rank );
      }
  else if (rank == 0) {
      printf( "Completed successfully\n" );
      }
  /* Finalize and end! */

  MPI_Finalize();
}










