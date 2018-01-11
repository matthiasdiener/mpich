#include "mpi.h"
#include "mpe.h"
#include <math.h>
#include <stdio.h>

double f(a)
double a;
{
    return (4.0 / (1.0 + a*a));
}

int main(argc,argv)
int argc;
char *argv[];
{
  int done = 0, n, myid, numprocs, i, rc, repeat;
  double PI25DT = 3.141592653589793238462643;
  double mypi, pi, h, sum, x, a;
  double startwtime, endwtime;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  MPE_Init_log();
  if (myid == 0) {
    MPE_Describe_state(1, 2, "Broadcast", "red:vlines3");
    MPE_Describe_state(3, 4, "Compute",   "blue:gray3");
    MPE_Describe_state(5, 6, "Reduce",    "green:light_gray");
    MPE_Describe_state(7, 8, "Sync",      "yellow:gray");
  }

  while (!done)
    {
      if (myid == 0) 
	{
	  printf("Enter the number of intervals: (0 quits) ");
	  scanf("%d",&n);
	  startwtime = MPI_Wtime();
	}
      MPI_Barrier(MPI_COMM_WORLD);
      MPE_Start_log();

      MPE_Log_event(1, 0, "start broadcast");
      MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPE_Log_event(2, 0, "end broadcast");
    
      if (n == 0)
	done = 1;
      else
	{
	    MPE_Log_event(7,0,"Start Sync");
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPE_Log_event(8,0,"End Sync");
	    MPE_Log_event(3, 0, "start compute");
	    h   = 1.0 / (double) n;
	    sum = 0.0;
	    for (i = myid + 1; i <= n; i += numprocs)
	      {
		x = h * ((double)i - 0.5);
		sum += f(x);
	      }
	    mypi = h * sum;
	    MPE_Log_event(4, 0, "end compute");

	    MPE_Log_event(5, 0, "start reduce");
	    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	    MPE_Log_event(6, 0, "end reduce");
	    if (myid == 0)
	      {
		printf("pi is approximately %.16f, Error is %.16f\n",
		       pi, fabs(pi - PI25DT));
		endwtime = MPE_Wtime();
		printf("wall clock time = %f\n", endwtime-startwtime);	       
	      }
	  }
      MPE_Stop_log();
    }
  MPE_Finish_log("cpilog.log");
  MPI_Finalize();
}
