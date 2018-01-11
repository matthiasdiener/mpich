#include <stdio.h>
#include <mpi.h>

#define SIZE 100

/* This is basically example 3.33 (p. 77) from the standard. */
/* This test uses "type_count", which has been removed from the standard
 */

struct Partstruct {
  int class;     /* Particle class */
  double d[6];   /* particle coordinates */
  char b[7];     /* some additional information */
};

struct Partstruct particle[SIZE];

doit()
{
  int i, dest, rank, extent, size, count, lb, ub, myrank;
  MPI_Comm comm;

  /* Build datatype describing structure */

  MPI_Datatype Particletype;
  MPI_Datatype type[3] = {MPI_INT, MPI_DOUBLE, MPI_CHAR};
  int blocklen[3] = {1, 6, 7};
  MPI_Aint disp[3];
  int base;
  
  /* Compute displacements of structure components */

  MPI_Address(particle, disp);
  MPI_Address(particle[0].d, disp+1);
  MPI_Address(particle[0].b, disp+2);
  base = disp[0];
  for (i = 0; i < 3; i++) disp[i] -= base;


  MPI_Type_struct(3, blocklen, disp, type, &Particletype);
  MPI_Type_commit( &Particletype );

  MPI_Type_extent(Particletype,&extent);
  MPI_Type_size(Particletype,&size);
  MPI_Type_count(Particletype,&count);
   -- MPI_Type_count removed from MPI --
  MPI_Type_lb(Particletype,&lb);
  MPI_Type_ub(Particletype,&ub);

  printf("Particletype has extent: %d size: %d count: %d lb: %d ub: %d\n",
         extent, size, count, lb, ub);

  /* This is necessary for the program to work since the value 63 is
   * calculated for the extent and ub.
   */
  /* Particletype->extent = Particletype->ub = 64; */


  
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  if (myrank == 0) {
    /* Initialize particle array */
  
    for (i = 0; i < SIZE; i++) {
      particle[i].class = i;
      particle[i].d[0] = (double) i;
      particle[i].d[1] = (double) i;
      particle[i].d[2] = (double) i;
      particle[i].d[3] = (double) i;
      particle[i].d[4] = (double) i;
      particle[i].d[5] = (double) i;
      strcpy(particle[i].b,"nuts");
    }
    
    /* Now send it to 1 */

    MPI_Send(particle, SIZE, Particletype, 1, 0, MPI_COMM_WORLD);
  }
  else if (myrank == 1) {
    MPI_Status status;



    MPI_Recv(particle, SIZE, Particletype, 0, 0, MPI_COMM_WORLD, &status);

    MPI_Get_count( &status, MPI_BYTE, &size );
    printf("Status source: %d tag: %d size: %d\n",status.MPI_SOURCE,
           status.MPI_TAG, size);
    
    for (i = 0; i < SIZE; i++) {
      printf("Class: %d (%f,%f,%f,%f,%f,%f) %s\n", particle[i].class,
             particle[i].d[0], particle[i].d[1], particle[i].d[2],
             particle[i].d[3], particle[i].d[4], particle[i].d[5],
             particle[i].b);
    }
  }

}

main (int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  doit();
  MPI_Finalize();
}
