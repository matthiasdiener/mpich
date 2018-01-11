#include <stdio.h>
#include <nx.h>

#define ITER_DFLT 50
#define SIZE_DFLT 125000

main(argc,argv)
    int argc;
    char *argv[];
{
  int i, self, iter=ITER_DFLT, size=SIZE_DFLT, bytes;
  double start, end, *A;
  int request;

  bytes = sizeof(double)*size;

  A = (double *) malloc(bytes);

  for (i=0; i< size; ++i) A[i] = 0.0;

  self = mynode();
  
  printf( "Process %d is alive\n", self);
  if (self == 0) printf("%d iterations, %d bytes\n",iter,bytes);

  /* gsync(); */
  gsync();
  
  if (self == 0) 
    {
      start = dclock();
      for (i=0; i< iter; ++i)
        {
          request = isend(1,A,bytes,1,0); 
          /*MPI_Isend(A,size,MPI_DOUBLE,1,0,MPI_COMM_WORLD,&request); */
          crecv(2,A,bytes); 
	  msgwait(request);
          /* MPI_Recv(A,size,MPI_DOUBLE,1,1,MPI_COMM_WORLD,&status); */
        }
      end = dclock();
      
      printf("Times are: %g %g %g %g\n",
             start, end, end - start, (2*iter*bytes)/(end - start));
      
    }
  else
    {
      for (i=0; i< iter; ++i)
        {
	  if (i > 0) msgwait(request);
          crecv(1,A,bytes); 
          /* MPI_Recv(A,size,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status); */
          request = isend(2,A,bytes,0,0); 
          /* MPI_Isend(A,size,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&request); */
        }
    }
}
