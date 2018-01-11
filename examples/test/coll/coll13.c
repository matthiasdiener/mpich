#include "mpi.h"

/* 
From: hook@nas.nasa.gov (Edward C. Hook)
 */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#ifdef __STDC__
#define WHY strerror(errno)
#else
extern char *sys_errlist[];
#define WHY sys_errlist[errno]
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

int main( argc, argv )
int argc;
char *argv[];
{
	int rank, size;
	int chunk = 4096;
	int i;
	char *sb;
	char *rb;
	int status, gstatus;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	for ( i=1 ; i < argc ; ++i ) {
		if ( argv[i][0] != '-' )
			continue;
		switch(argv[i][1]) {
			case 'm':
				chunk = atoi(argv[++i]);
				break;
			default:
				fprintf(stderr,"Unrecognized argument %s\n",
					argv[i]);
				exit(EXIT_FAILURE);
		}
	}

	sb = (char *)malloc(size*chunk*sizeof(int));
	if ( !sb ) {
		fprintf(stderr,"can't allocate send buffer: %s\n",WHY);
		exit(EXIT_FAILURE);
	}
	rb = (char *)malloc(size*chunk*sizeof(int));
	if ( !rb ) {
		free(sb);
		fprintf(stderr,"can't allocate recv buffer: %s\n",WHY);
		exit(EXIT_FAILURE);
	}
	for ( i=0 ; i < size*chunk ; ++i ) {
		sb[i] = rank + 1;
		rb[i] = 0;
	}

	/* fputs("Before MPI_Alltoall\n",stdout); */

	status = MPI_Alltoall(sb,chunk,MPI_INT,rb,chunk,MPI_INT,
			      MPI_COMM_WORLD);

	/* fputs("Before MPI_Allreduce\n",stdout); */
	MPI_Allreduce( &status, &gstatus, 1, MPI_INT, MPI_SUM, 
		       MPI_COMM_WORLD );

	/* fputs("After MPI_Allreduce\n",stdout); */
	if (rank == 0)
	    printf("all_to_all returned %d\n",gstatus);

	free(sb);
	free(rb);

	MPI_Finalize();

	return(EXIT_SUCCESS);
}

