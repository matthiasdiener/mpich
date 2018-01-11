/*
 * $Id: cpi++.C,v 1.1 1994/06/16 16:17:15 bridges Exp $
 */

#include <mpi++.h>
#include <math.h>

double f(double a)
{
    return (4.0 / (1.0 + a*a));
}

int main(int argc, char *argv[])
{
   int done = 0, n, myid, numprocs, i, rc;
   double PI25DT = 3.141592653589793238462643;
   double mypi, pi, h, sum, x, a;
   double startwtime, endwtime;

   MPI_COMM_WORLD.Init (argc, argv);
   MPI_COMM_WORLD.Rank (myid) ;
   MPI_COMM_WORLD.Size (numprocs) ;
 
   while (!done)
   {
	 if (myid == 0)
     {
	   printf("Enter the number of intervals: (0 quits) ");
	   scanf("%d",&n);
	   startwtime = MPI_Wtime();
	 }
	 MPI_COMM_WORLD.Bcast(&n, 1, MPI_INT, 0);
	 if (n == 0)
	   done = 1;
	 else
     {
	   h   = 1.0 / (double) n;
	   sum = 0.0;
	   for (i = myid + 1; i <= n; i += numprocs)
	   {
		 x = h * ((double)i - 0.5);
		 sum += f(x);
	   }
	   mypi = h * sum;
	   
	   MPI_COMM_WORLD.Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0);
 
	   if (myid == 0)
       {
		 printf("pi is approximately %.16f, Error is %.16f\n",
				pi, fabs(pi - PI25DT));
		 endwtime = MPI_Wtime();
		 printf("wall clock time = %f\n",
				endwtime-startwtime);
	   }
	 }
   }

  MPI_COMM_WORLD.Finalize();
  return (MPI_SUCCESS);
}

