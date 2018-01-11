#include <stdio.h>
#include <string.h>
#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
    int pool_size, my_rank, destination, source, node_name_length;
    char char_buffer[BUFSIZ], my_node_name[BUFSIZ], host_name[BUFSIZ]; 
    MPI_Status status;

    MPI_Init ( &argc, &argv );

/* How many processes are there in the pool? */
    MPI_Comm_size ( MPI_COMM_WORLD, &pool_size ); 
/* What is my rank number? */
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
/* What is my name? */
    MPI_Get_processor_name ( my_node_name, &node_name_length );
/* The rank 0 process sends its name to other processes. */
    if ( my_rank == 0 ) {
       strcpy (char_buffer, my_node_name);
       for (destination = 1; destination < pool_size; destination++)
             MPI_Send(char_buffer, strlen(my_node_name) + 1, MPI_CHAR, 
		      destination, 1001, MPI_COMM_WORLD);
    }
    else {
       MPI_Recv(char_buffer, BUFSIZ, MPI_CHAR, 0, 1001, 
                MPI_COMM_WORLD, &status);
       strcpy (host_name, char_buffer);
    }
/* The other processes send their names to process 0, which prints
   the messages on standard output. 
 */
    if ( my_rank != 0 ) {
       sprintf (char_buffer, "Greetings to %s from (%s, %d)",
                host_name, my_node_name, my_rank);
       MPI_Send (char_buffer, strlen(char_buffer) + 1, MPI_CHAR, 0, 
                 2002, MPI_COMM_WORLD);
    }
    else {
       for (source = 1; source < pool_size; source++) {
          MPI_Recv(char_buffer, BUFSIZ, MPI_CHAR, source, 2002,
                   MPI_COMM_WORLD, &status);
          printf ("%s\n", char_buffer);
       }
    }

    MPI_Finalize ();
}
