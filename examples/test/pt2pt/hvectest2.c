/*
    hvectest - test program that sends an array of floats from the first 
             process of a group to the last, using send and recv and the
	     vector datatype.
*/

#include "mpi.h"
#include <stdio.h>
/* #define SHOWMSG */

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size, to, from, tag, count, np, i;
    int src, dest;
    int st_source, st_tag, st_count;
    int errcnt = 0;
    MPI_Request handle;
    MPI_Status status;
    double data[100];
    MPI_Datatype rowtype;
    int blens[2];
    MPI_Datatype types[2];
    MPI_Aint displs[2];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    if (argc > 1 && argv[1] && strcmp( "-alt", argv[1] ) == 0) {
	src  = size - 1;
	dest = 0;
	}
    else {
	dest = size - 1;
	src  = 0;
	}

    blens[0]  = 1;
    blens[1]  = 1;
    displs[0] = 0;
    displs[1] = 10*sizeof(double);
    types[0]  = MPI_DOUBLE;
    types[1]  = MPI_UB;
    MPI_Type_struct( 2, blens, displs, types, &rowtype );
    MPI_Type_commit( &rowtype );
    /* First test: send a row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Send( data, count, rowtype, to, tag, MPI_COMM_WORLD );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
    }

    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Recv(data, count, MPI_DOUBLE, from, tag, MPI_COMM_WORLD,
		 &status ); 

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i] != 10*i) { 
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](rcv double) %d'th element = %f, should be %f\n",
		     rank, i, data[i], 10.0*i );
	    }
    }

    /* Second test: receive a column into row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Send( data, count, MPI_DOUBLE, to, tag, MPI_COMM_WORLD );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i]);printf("\n");
#endif
    }
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Recv(data, count, rowtype, from, tag, MPI_COMM_WORLD,
		 &status ); 

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i*10] != i) {
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](rcv row) %d'th element = %f, should be %f\n",
		     rank, i, data[i*10], 1.0*i );
	    }
    }

    /* Third test: send AND receive a row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Send( data, count, rowtype, to, tag, MPI_COMM_WORLD );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
    }
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Recv(data, count, rowtype, from, tag, MPI_COMM_WORLD,
		 &status ); 

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i*10] != i*10) {
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](rcv row-row) %d'th element = %f, should be %f\n",
		     rank, i, data[i*10], 10.0*i );
	    }
    }

    /* Second Set of Tests: Use Isend and Irecv instead of Send and Recv */
    /* First test: send a row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Isend( data, count, rowtype, to, tag, MPI_COMM_WORLD, &handle );
	MPI_Wait( &handle, &status );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
    }

    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Irecv(data, count, MPI_DOUBLE, from, tag, MPI_COMM_WORLD,
		 &handle ); 
	MPI_Wait( &handle, &status );

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i] != 10*i) {
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](ircv double) %d'th element = %f, should be %f\n",
		     rank, i, data[i], 10.0*i );
	    }
    }

    /* Second test: receive a column into row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Isend( data, count, MPI_DOUBLE, to, tag, MPI_COMM_WORLD, 
		   &handle );
	MPI_Wait( &handle, &status );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i]);printf("\n");
#endif
    }
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Irecv(data, count, rowtype, from, tag, MPI_COMM_WORLD,
		 &handle ); 
	MPI_Wait( &handle, &status );

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i*10] != i) {
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](ircv row) %d'th element = %f, should be %f\n",
		     rank, i, data[i*10], 1.0*i );
	    }
    }

    /* Third test: send AND receive a row */
    if (rank == src)
    {
	to     = dest;
	count  = 10;
	tag    = 2001;
	for (i = 0; i < 100; i++)
	    data[i] = i;
	/* Send a row */
	MPI_Isend( data, count, rowtype, to, tag, MPI_COMM_WORLD, &handle );
	MPI_Wait( &handle, &status );
#ifdef SHOWMSG
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
    }
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	count = 10;		
	from  = MPI_ANY_SOURCE;
	MPI_Irecv(data, count, rowtype, from, tag, MPI_COMM_WORLD,
		 &handle ); 
	MPI_Wait( &handle, &status );

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
#ifdef SHOWMSG
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i*10]);printf("\n");
#endif
	for (i = 0; i < 10; i++) if (data[i*10] != i*10) {
	    errcnt++;
	    fprintf( stderr, 
		    "[%d](ircv row-row) %d'th element = %f, should be %f\n",
		     rank, i, data[i*10], 10.0*i );
	    }
    }

    i = errcnt;
    MPI_Allreduce( &i, &errcnt, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if (errcnt > 0) {
	printf( "Found %d errors in the run \n", errcnt );
	}
    MPI_Finalize();
}
