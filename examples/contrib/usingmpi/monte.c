/* compute pi using Monte Carlo method */

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"
#include "mpe.h"

#define LOG               1
#include "monte.h"

#define DEBUG             0
#define CHUNKSIZE      1000
#define WINDOW_SIZE     500
#define QUARTER_CIRCLE    0
#define RANDOM_SEED       0

/* message tags */
#define NEED_NUMBERS      1
#define RANDOM_NUMBERS    2

/* log events */
#define START_CREATE_RAND         1
#define END_CREATE_RAND           2
#define START_COMPUTE             3
#define END_COMPUTE               4
#define START_GRAPHICS_UPDATE     5
#define END_GRAPHICS_UPDATE       6
#define START_REDUCE              7
#define END_REDUCE                8


DrawCircle( graph, centerx, centery, radius )
MPE_XGraph graph;
int centerx, centery, radius;
{
  int lasty, x, y, radius2;

  radius2 = radius*radius;
  lasty = 0;
  for (x=1-radius; x<=radius; x++) {
    y = sqrt( (double)(radius2-x*x) );
    MPE_Draw_line( graph, x-1+centerx,  lasty+centery, x+centerx,  y+centery,
		 MPE_BLACK );
    MPE_Draw_line( graph, x-1+centerx, -lasty+centery, x+centerx, -y+centery,
		 MPE_BLACK );
    lasty = y;
  }
  MPE_Update( graph );
}
  


DrawArc( graph, size )
MPE_XGraph graph;
int size;
{
  int lasty, x, y, size2;

  size2 = size*size;
  lasty = 0;
  for (x=1; x<=size; x++) {
    y = sqrt( (double)(size2-x*x) );
    MPE_Draw_line( graph, x-1, size-lasty, x, size-y,
		 MPE_BLACK );
    lasty = y;
  }
  MPE_Update( graph );
}
  


main(argc,argv)
int argc;
char *argv[];
{
    register int in,out,i,iters,max;
    register int ix,iy;
    register double x,y,calculatedPi;
    double error, epsilon;
    int ranks[1], done, temp;
    int numprocs, myid, randServer, totalin, totalout, workerNum;
    MPI_Comm world, workers;
    MPI_Group world_group, worker_group;
    MPI_Status mesgStatus;
    int randNums[CHUNKSIZE], request;
    MPE_XGraph graph;

    MPI_Init(&argc,&argv);
    world = MPI_COMM_WORLD;
    MPI_Comm_size(world,&numprocs);
    MPI_Comm_rank(world,&myid);
    randServer = numprocs-1;

    {
      int gotEpsilon;
      if (myid == 0) {
	if ((argc!=2) || !sscanf( argv[1], "%lf", &epsilon )) {
	  printf( "Syntax:\n    %s <epsilon>\n", argv[0] );
	  gotEpsilon = 0;
	} else {
	  gotEpsilon = 1;
	}
      }
      MPI_Bcast( &gotEpsilon, 1, MPI_INT, 0, MPI_COMM_WORLD );
      if (!gotEpsilon) {
	MPI_Finalize();
	exit(1);
      }
    }

    MPE_INIT_LOG();
    if (myid == 0) {
      MPE_DESCRIBE_STATE( START_CREATE_RAND, END_CREATE_RAND,
			  "Create random data", ":" );
      MPE_DESCRIBE_STATE( START_COMPUTE, END_COMPUTE,
			  "Compute", ":" );
      MPE_DESCRIBE_STATE( START_GRAPHICS_UPDATE, END_GRAPHICS_UPDATE,
			  "Graphics update", ":" );
      MPE_DESCRIBE_STATE( START_REDUCE, END_REDUCE, "Reduce", ":" );
    }
    MPE_Open_graphics( &graph, MPI_COMM_WORLD, 0, -1, -1, WINDOW_SIZE,
		       WINDOW_SIZE, 0 );
    /*
       DrawArc( graph, WINDOW_SIZE );
       */

    MPI_Comm_group( world, &world_group );	/* get group of everyone  */
    ranks[0] = randServer;	                /* exclude the last       */
    MPI_Group_excl( world_group, 1, ranks, &worker_group );
                                                /* get new group    */
    MPI_Comm_create( world, worker_group, &workers );  /* get new comm     */
    

    if (myid == randServer)	/* I am the rand server */
    {
#if RANDOM_SEED
        struct timeval time;
        gettimeofday( &time, 0 );
	/* initialize the random number generator */
	srandom( (int)(time.tv_usec*1000000+time.tv_sec) );
#endif
	do {
	    MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, NEED_NUMBERS,
		     world, &mesgStatus);
	    MPE_LOG_RECEIVE( mesgStatus.MPI_SOURCE, NEED_NUMBERS,
			     sizeof( int ) );
	    if (request)
	    {
#if DEBUG
	        fprintf( stderr, "[%d] request from %d for numbers\n", myid,
			mesgStatus.MPI_SOURCE);
#endif
	        MPE_LOG_EVENT( START_CREATE_RAND, 0, (char *)0 );
		for (i = 0; i < CHUNKSIZE; i++)
	            {
		        randNums[i] = random();
		    }
	        MPE_LOG_EVENT( END_CREATE_RAND, 0, (char *)0 );
		MPI_Send(randNums, CHUNKSIZE, MPI_INT, mesgStatus.MPI_SOURCE, 
			 RANDOM_NUMBERS, world);
		MPE_LOG_SEND( mesgStatus.MPI_SOURCE, RANDOM_NUMBERS,
			      sizeof( int ) * CHUNKSIZE );
	    }
	}
	while( request>0 );
    }
    else			/* I am a worker process */
    {
        request = 1;
	done = in = out = 0;
	max = (1 << 31) - 1;	/* find max integer for normalization */
#if DEBUG
	fprintf( stderr, "[%d] asking for numbers\n", myid );
#endif
        MPI_Send( &request, 1, MPI_INT, randServer, NEED_NUMBERS, world );
	  /* request a string of random numbers */
	MPE_LOG_SEND( randServer, NEED_NUMBERS, sizeof(int) );
        MPI_Comm_rank( workers, &workerNum );
	while (!done)
	{
	    request = 1;
	    MPI_Recv( randNums, CHUNKSIZE, MPI_INT, randServer, RANDOM_NUMBERS,
		     world, &mesgStatus );
	    MPE_LOG_RECEIVE( randServer, RANDOM_NUMBERS,
			     sizeof(int) * CHUNKSIZE );

#if DEBUG
	fprintf( stderr, "[%d] computing\n", myid );
#endif

	    MPE_LOG_EVENT( START_COMPUTE, 0, (char *)0 );
	    for (i=0; i<CHUNKSIZE; )
	    {
#if QUARTER_CIRCLE
	        x = ((float) randNums[i++])/max;
		y = ((float) randNums[i++])/max;
#else
	        x = (((float) randNums[i++])/max) * 2 - 1;
		y = (((float) randNums[i++])/max) * 2 - 1;
#endif
#if DEBUG>1
		fprintf( stderr, "[%d] (%lf %lf)\n", myid, x, y );
#endif
		if (x*x + y*y < 1.0)
		{
		    in++;
#if QUARTER_CIRCLE
		    MPE_Draw_point( graph,
				  (int)(x*WINDOW_SIZE), 
				  (int)(WINDOW_SIZE - y*WINDOW_SIZE),
				  MPE_BLACK );
#else
		    MPE_Draw_point( graph,
				  (int)(WINDOW_SIZE/2 + x*WINDOW_SIZE/2), 
				  (int)(WINDOW_SIZE/2 - y*WINDOW_SIZE/2),
				  MPE_BLACK );
#endif
		}
		else
		    out++;
	    }
	    MPE_LOG_EVENT( END_COMPUTE, 0, (char *)0 );

	    MPE_LOG_EVENT( START_GRAPHICS_UPDATE, 0, (char *)0 );
	    MPE_Update( graph );
	    MPE_LOG_EVENT( END_GRAPHICS_UPDATE, 0, (char *)0 );
	    temp = in;
#if DEBUG
	fprintf( stderr, "[%d] reducing\n", myid );
#endif
	    MPE_LOG_EVENT( START_REDUCE, 0, (char *)0 );
	    MPI_Reduce(&temp, &totalin, 1, MPI_INT, MPI_SUM, 0,
		       workers);
	      /* count total of ins  */ 
	    MPE_LOG_EVENT( END_REDUCE, 0, (char *)0 );
	    temp = out;
	    MPI_Reduce(&temp, &totalout, 1, MPI_INT, MPI_SUM, 0,
		       workers);
	      /* count total of outs */

	    if (myid == 0)
	    {
	        calculatedPi = (4.0*totalin)/(totalin + totalout);
		error = fabs( calculatedPi-3.141592653589793238462643);
		done = (error < epsilon) || (totalin+totalout)>1000000000;
		printf( "\rpi = %23.20lf", calculatedPi );
		request = (done) ? 0 : 1;
		MPI_Send( &request, 1, MPI_INT, randServer, NEED_NUMBERS,
			  world );
		MPE_LOG_SEND( randServer, NEED_NUMBERS, sizeof(int) );
	        MPI_Bcast( &done, 1, MPI_INT, 0, workers );
	    } else
	    {
	        MPI_Bcast( &done, 1, MPI_INT, 0, workers );
		if (!done)
		{
		    request = 1;
		    MPI_Send( &request, 1, MPI_INT, randServer, NEED_NUMBERS,
			     world );
		    MPE_LOG_SEND( randServer, NEED_NUMBERS, sizeof(int) );
		}
	    }
	}
    }
    if (myid == 0)
    {
        printf( "\npoints: %d\nin: %d, out: %d\n", totalin+totalout,
	       totalin, totalout );
	getchar();
    }
    MPE_Close_graphics( &graph );
    MPE_FINISH_LOG( "monte.log" );
    MPI_Finalize();
}
