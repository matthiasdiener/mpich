
#include <mpi++.h>
#include <iostream.h>
#include <stdio.h>

extern void   srand48(long);
extern double drand48(void);

#define BORN 1
#define DIES 0

/* The Life function */
double life(int matrix_size, int ntimes, MPI_Comm& comm)
{
  int      rank, size ;
  int      next, prev ;
  int      i, j, k;
  int      mysize, sum ;
  int    **matrix, **temp, **addr ;
  double   totaltime, starttime, maxtime ;

  /* Determine size and my rank in communicator */
  comm.Size(size);
  comm.Rank(rank);

  /* Set neighbors */
  if (rank == 0) 
    prev = MPI_PROC_NULL;
  else
    prev = rank-1;
  if (rank == size - 1)
    next = MPI_PROC_NULL;
  else
    next = rank+1;

  /* Determine my part of the matrix */
  mysize = matrix_size/size + ((rank < (matrix_size % size)) ? 1 : 0 ) ;

  /* allocate the memory dynamically for the matrix */
  matrix = (int **) new int[mysize+2];
  temp = (int **) new int[mysize+2];
  for (i = 0; i < mysize+2; i++) {
    matrix[i] = new int[matrix_size+2];
    temp[i] = new int[matrix_size+2];
  }

  /* Initialize the boundaries of the life matrix */
  for (j = 0; j < matrix_size+2; j++)
    matrix[0][j] = matrix[mysize+1][j] = temp[0][j] = temp[mysize+1][j] = DIES ;
  for (i = 0; i < mysize+2; i++)
    matrix[i][0] = matrix[i][matrix_size+1] = temp[i][0] = temp[i][matrix_size+1] = DIES ;

  /* Initialize the life matrix */
  for (i = 1; i <= mysize; i++)  {
//    srand48((long)(1000^(i-1+mysize))) ;
    for (j = 1; j<= matrix_size; j++)
//    if (drand48() > 0.5)  
        matrix[i][j] = BORN ;
//      else
//        matrix[i][j] = DIES ;
  }

  /* Play the game of life for given number of iterations */
  starttime = MPI_Wtime() ;
  for (k = 0; k < ntimes; k++) {
    MPI_Request      req[4];
    MPI_Status       status[4];

	cout << "I'm here!\n";
    /* Send and receive boundary information */
    comm.Isend(&matrix[1][0],matrix_size+2,MPI_INT,prev,0,req[0]); 
    comm.Irecv(&matrix[0][0],matrix_size+2,MPI_INT,prev,0,req[1]);
    comm.Isend(&matrix[mysize][0],matrix_size+2,MPI_INT,next,0,req[2]);
    comm.Irecv(&matrix[mysize+1][0],matrix_size+2,MPI_INT,next,0,req[3]);
    MPI_Waitall(4, req, status);

    /* For each element of the matrix ... */ 
    for (i = 1; i <= mysize; i++) {
      for (j = 1; j < matrix_size+1; j++) {

        /* find out the value of the current cell */
        sum = matrix[i-1][j-1] + matrix[i-1][j] + matrix[i-1][j+1] 
          + matrix[i][j-1] + matrix[i][j+1] 
            + matrix[i+1][j-1] + matrix[i+1][j] + matrix[i+1][j+1] ;
        
        /* check if the cell dies or life is born */
        if (sum < 2 || sum > 3)
          temp[i][j] = DIES ;
        else if (sum == 3)
          temp[i][j] = BORN ;
        else
          temp[i][j] = matrix[i][j] ;
      }
    }
    
    /* Swap the matrices */
    addr = matrix ;
    matrix = temp ;
    temp = addr ;
  }

  /* free the memory dynamically for the matrix */
  delete matrix; delete temp;
  for (i = 0; i < mysize+2; i++) {
    delete matrix[i];
    delete temp[i];
  }

  /* Return the average time taken/processor */
  totaltime = MPI_Wtime() - starttime;
  comm.Reduce (&totaltime, &maxtime, 1, MPI_DOUBLE, MPI_SUM, 0);
  return (maxtime/(double)size);
}

int main(int argc, char *argv[])
{
  int rank, N, iters ;
  double time ;

  MPI_COMM_WORLD.Init (argc, argv);
  MPI_COMM_WORLD.Rank (rank) ;

  /* If I'm process 0, determine the matrix size and # of iterations */
  if ( rank == 0 ) {
    printf("Matrix Size : ") ;
    scanf("%d",&N) ;
    printf("Iterations : ") ;
    scanf("%d",&iters) ;
  }

  /* Broadcast the size and # of iterations to all processes */
  MPI_COMM_WORLD.Bcast(&N, 1, MPI_INT, 0) ;
  MPI_COMM_WORLD.Bcast(&iters, 1, MPI_INT, 0);

  /* Call the life routine */
  time = life ( N, iters, MPI_COMM_WORLD );

  /* Print the total time taken */
  if (rank == 0)
    printf("[%d] Life finished in %lf seconds\n",rank,time/100);

  MPI_COMM_WORLD.Finalize();
  return (MPI_SUCCESS);
}

