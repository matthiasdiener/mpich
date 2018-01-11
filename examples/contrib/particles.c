#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX_PARTICLES 1000
#define MAX_PROCS      128
#define EPSILON          1.0E-10

typedef struct {
   double x, y, z;
   double mass;
} Particle;

typedef struct {
   double vx, vy, vz;
} ParticleV;

int main ( argc, argv )
int argc;
char **argv;
{
   int          pool_size, my_rank, source, node_name_length;
   char         char_buffer[BUFSIZ], my_node_name[BUFSIZ];
   MPI_Status   status;

   extern       double drand48();
   extern       void srand48();

   Particle     particles[MAX_PARTICLES];  /* Particles on all nodes */
   ParticleV    vector[MAX_PARTICLES];     /* Particle velocity */
   int          counts[MAX_PROCS];         /* Number of ptcls on each proc */
   int          displacements[MAX_PROCS];  /* Offsets into particles */
   int          offsets[MAX_PROCS];        /* Offsets used by the master */
   int          particle_number, i, j, 
                my_offset;                 /* Location of local particles */
   int          total_particles;           /* Total number of particles */
   int          count;                     /* Number of times in loop */
   double       start_time, end_time;

   MPI_Datatype particle_type;

   MPI_Init ( &argc, &argv );
 
   MPI_Comm_size ( MPI_COMM_WORLD, &pool_size );
   MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
   MPI_Get_processor_name ( my_node_name, &node_name_length );
   sprintf ( char_buffer, "processor %s, rank %d", my_node_name, my_rank );
   MPI_Send ( char_buffer, strlen(char_buffer) + 1, MPI_CHAR, 0, 2002,
              MPI_COMM_WORLD );
   if ( my_rank == 0 ) {
      for ( source = 0; source < pool_size; source++ ) {
         MPI_Recv ( char_buffer, BUFSIZ, MPI_CHAR, source, 2002, 
                    MPI_COMM_WORLD, &status );
         printf ( "%s\n", char_buffer );
      }
   }

/* For simplicity we assume that every process is responsible for the same 
   number of particles. But the program can also handle the case when
   particle numbers are different for different processes.
 */

   particle_number = MAX_PARTICLES / pool_size;

   if ( my_rank == 0 )
      printf ("%d particles per processor\n", particle_number);

/* Define the new MPI data type "particle_type", which correponds to
   structure "Particle". 
 */

   MPI_Type_contiguous ( 4, MPI_DOUBLE, &particle_type );
   MPI_Type_commit ( &particle_type );

/* Distribute the number of particles each process is supposed to look after
   in such a way that every process will find that number in
   count[my_rank]. In other slots of that array each process finds 
   the number of particles other processes will work on.
 */

   MPI_Allgather ( &particle_number, 1, MPI_INT, counts, 1, MPI_INT,
		   MPI_COMM_WORLD );

/* Data about all particles is stored on a large array called "particles".
   Every process has a copy of that array. Here we evaluate, where the
   segment of the array begins, which a given process will work on.
 */

   displacements[0] = 0;
   for (i = 1; i < pool_size; i++)
      displacements[i] = displacements[i-1] + counts[i-1];
   total_particles = displacements[pool_size - 1] + counts[pool_size - 1];
       
   if ( my_rank == 0 )
      printf ("total number of particles = %d\n", total_particles);

   my_offset = displacements[my_rank];

/* Process rank 0 gathers information from all other processes about
   offsets they calculated for the array "particles". 
 */

   MPI_Gather ( &my_offset, 1, MPI_INT, offsets, 1, MPI_INT, 0,
		MPI_COMM_WORLD );

   if ( my_rank == 0 ) {
      printf ("offsets: ");
      for (i = 0; i < pool_size; i++)
         printf ("%d ", offsets[i]);
      printf("\n");
      fflush(stdout);
   }

/* Every process seeds the random number routine drand48 with its own
   process rank.
 */

   srand48 ( (long) my_rank );

/* And now every process initialises positions and mass of particles
   it is responsible for.
 */

   for (i = 0; i < particle_number; i++) {
      particles[my_offset + i].x = drand48();
      particles[my_offset + i].y = drand48();
      particles[my_offset + i].z = drand48();
      particles[my_offset + i].mass = 1.0;
   }

   start_time = MPI_Wtime();

/* Here all processes exchange information about their particles with
   one another. When this operation is finished every process will have
   an identical fully initialised array "particles".
 */

   MPI_Allgatherv ( particles + my_offset, particle_number,
		    particle_type,
		    particles, counts, displacements, particle_type,
		    MPI_COMM_WORLD );

   end_time = MPI_Wtime();

   if ( my_rank == 0 ) {
      printf ("Communicating = %8.5f seconds\n", end_time - start_time);
      printf ("particles[offsets[i]].x: ");
      for (i = 0; i < pool_size; i++)
         printf ("%8.5f ", particles[offsets[i]].x);
      printf("\n"); fflush(stdout);
   }

   start_time = MPI_Wtime();

   count = 0;
   for (i = 0; i < particle_number; i++) {

      vector[my_offset + i].vx = 0.0;
      vector[my_offset + i].vy = 0.0;
      vector[my_offset + i].vz = 0.0;

   /* Knowing about positions of all particles in the system, every process
      can evaluate gravitational accelerations acting on its own particles,
      and write them on array "vector". Once accelerations are known,
      particle velocities (not calculated here) can be updated, and
      particles can be "pushed".
    */
	  
      for (j = 0; j < total_particles; j++) {
         if (j != i) {

            double dx, dy, dz, r2, r, mimj_by_r3;

            dx = particles[my_offset + i].x - particles[j].x;
            dy = particles[my_offset + i].y - particles[j].y;
            dz = particles[my_offset + i].z - particles[j].z;

            r2 = dx * dx + dy * dy + dz * dz; r = sqrt(r2);

            if (r2 < EPSILON) mimj_by_r3 = 0.0;
            else
               mimj_by_r3 = particles[my_offset + i].mass 
                            * particles[j].mass / (r2 * r);

            vector[my_offset + i].vx = vector[my_offset + i].vx +
                                       mimj_by_r3 * dx;
            vector[my_offset + i].vy = vector[my_offset + i].vy +
                                       mimj_by_r3 * dy;
            vector[my_offset + i].vz = vector[my_offset + i].vz +
                                       mimj_by_r3 * dz;
            count = count + 1;
         }
      }
   }

   end_time = MPI_Wtime();

   if ( my_rank == 0 )
      printf ("done my job in %8.5f seconds, waiting for slow \
processes...\n", end_time - start_time);

   MPI_Barrier (MPI_COMM_WORLD);

   end_time = MPI_Wtime();

   if ( my_rank == 0 )
      printf ("evaluated %d 3D interactions in %8.5f seconds\n",
               count * pool_size, end_time - start_time);

    MPI_Finalize ();
}
