#include "mpi.h"

/* 
From: hook@nas.nasa.gov (Edward C. Hook)
 */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#define WHY strerror(errno)

int main(int argc, char *argv[])
{
	int rank, size;
	int chunk = 4096;
	int i;
	char *sb;
	char *rb;
	int status;

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
				fprintf(stderr,"Huh ? %c ?\n",argv[i][1]);
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

	fputs("Before MPI_Alltoall\n",stdout);

	status = MPI_Alltoall(sb,chunk,MPI_INT,rb,chunk,MPI_INT,MPI_COMM_WORLD);

	printf("all_to_all returned %d\n",status);

	free(sb);
	free(rb);

	return(EXIT_SUCCESS);
}

