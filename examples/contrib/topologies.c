#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define UPDOWN              0
#define SIDEWAYS            1
#define RIGHT               1
#define UP                  1

#define PROCESS_DIMENSIONS  2

#define PROCESS_ROWS        4
#define ROWS                4
#define DISPLAY_ROWS       16      /* must be PROCESS_ROWS * ROWS */

#define PROCESS_COLUMNS     3
#define COLUMNS             4
#define DISPLAY_COLUMNS    12      /* must be PROCESS_COLUMNS * COLUMNS */

int world_rank_array[PROCESS_ROWS * PROCESS_COLUMNS];
int process_matrix[PROCESS_ROWS][PROCESS_COLUMNS];

int main ( argc, argv )
int argc;
char **argv;
{
   int pool_size, my_rank, source, node_name_length;
   char char_buffer[BUFSIZ], my_node_name[BUFSIZ];
   MPI_Status status;
   int divisions[PROCESS_DIMENSIONS];
   int periods[PROCESS_DIMENSIONS];
   int reorder = 1;
   MPI_Comm cartesian_communicator;
   int my_cartesian_rank, my_coordinates[PROCESS_DIMENSIONS];
   int left_neighbour, right_neighbour, bottom_neighbour, top_neighbour;
   int matrix [ROWS][COLUMNS];
   int i, j;
   MPI_Datatype column_type;


   MPI_Init ( &argc, &argv );

   MPI_Comm_size ( MPI_COMM_WORLD, &pool_size );
   MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );

/* If we don't have a sufficient number of processes for this job,
   we must abort. 
 */

   if ( pool_size < PROCESS_ROWS * PROCESS_COLUMNS ) {
      if ( my_rank == 0 ) {
	 fprintf ( stderr, 
		   "Error: not enough processes for this job, exiting... \n" );
	 fflush ( stderr );
      }
      MPI_Finalize ();
      exit (1);
   }

/* Every process finds about its processor name and sends a message to
   process number 0, which, in turn, displays it on standard output.
   Process number 0 sends the message to itself.
 */
      
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

/* Now we create a new communicator with Cartesian topology. Some processes
   may be thrown away from the new pool. For those processes the value of 
   cartesian_communicator will be MPI_COMM_NULL.
 */

   divisions[0]	= PROCESS_ROWS;
   divisions[1]	= PROCESS_COLUMNS;
   periods[0]	= 0;
   periods[1]	= 1;
   MPI_Cart_create ( MPI_COMM_WORLD, PROCESS_DIMENSIONS, divisions, 
                     periods, reorder, &cartesian_communicator );

   if (cartesian_communicator != MPI_COMM_NULL) {


   /* Within cartesian_communicator processes find about their new
      rank numbers, their cartesian coordinates, and their neighbours.
      Rank numbers within MPI_COMM_WORLD are remembered in my_rank
      variables.
    */

      MPI_Comm_rank ( cartesian_communicator, &my_cartesian_rank );
      MPI_Cart_coords ( cartesian_communicator, my_cartesian_rank,
                        PROCESS_DIMENSIONS, my_coordinates );
      MPI_Cart_shift ( cartesian_communicator, SIDEWAYS, RIGHT, 
                       &left_neighbour, &right_neighbour );
      MPI_Cart_shift ( cartesian_communicator, UPDOWN, UP,
                       &bottom_neighbour, &top_neighbour );

   /* All processes in cartesian_communicator send messages
      about themselves to process rank 0. Because we do not have
      the guarantee that process rank 0 belongs to cartesian_communicator,
      this communication must take place within MPI_COMM_WORLD.
      If process rank 0 belongs to cartesian_communicator,
      it sends the message to itself. 
    */

      sprintf ( char_buffer, "process %2d, cartesian %2d, \
coords (%2d,%2d), left %2d, right %2d, top %2d, bottom %2d",
		my_rank, my_cartesian_rank, my_coordinates[0],
		my_coordinates[1], left_neighbour, right_neighbour,
		top_neighbour, bottom_neighbour );
      MPI_Send ( char_buffer, strlen(char_buffer) + 1, MPI_CHAR,
		 0, 3003, MPI_COMM_WORLD );
   }

   if ( my_rank == 0 ) {

      int number_of_c_procs, count, world_rank, cartesian_rank,
	  row, column;
      char auxiliary_buffer[BUFSIZ];

      number_of_c_procs = divisions[0] * divisions[1];
      for ( count = 0; count < number_of_c_procs; count++ ) {
         MPI_Recv ( char_buffer, BUFSIZ, MPI_CHAR, MPI_ANY_SOURCE, 3003,
		    MPI_COMM_WORLD, &status );

	 /* Process 0 in MPI_COMM_WORLD extracts world_rank, 
	    cartesian_rank, and cartesian coordinates from the received 
	    message and writes it on the global array world_rank_array,
	    and on the global matrix process_matrix. This array and this
	    matrix will be used within the function collect_matrices.
	  */

	 sscanf ( char_buffer, 
		  "process %2d, cartesian %2d, coords (%2d,%2d), %s",
		  &world_rank, &cartesian_rank, &row, &column, 
		  auxiliary_buffer );
	 world_rank_array[cartesian_rank] = world_rank;
	 process_matrix[row][column] = cartesian_rank;

         printf ( "%s\n", char_buffer );
	 /*
	 printf ( "world_rank_array[%d] = %d\n", cartesian_rank, world_rank );
	  */
      }
   }

   /* Every process in cartesian_communicator initialises its own matrix
      to its own rank in cartesian_communicator, and sends the matrix
      to process 0 in MPI_COMM_WORLD.
    */

   if ( cartesian_communicator != MPI_COMM_NULL ) {

      for ( i = 0; i < ROWS; i++ ) {
         for ( j = 0; j < COLUMNS; j++ ) {
            matrix [i][j] = my_cartesian_rank;
         }
      }

      MPI_Send ( matrix, COLUMNS * ROWS, MPI_INT, 0, 4004,
                 MPI_COMM_WORLD );
   }
   if ( my_rank == 0 ) collect_matrices ( matrix, 4004 );

   /* Now processes in cartesian_communicator exchange the top and
      bottom rows of their matrices with their top and bottom neighbours.
      The updated matrices are sent, as before, to process 0, which displays
      them on standard output. We can transfer the rows in one go in C,
      because C stores matrices by rows. When exchanging columns,
      we will have to define a new MPI data type.
    */

   if ( cartesian_communicator != MPI_COMM_NULL ) {

      MPI_Sendrecv ( &matrix[ROWS - 2][0], COLUMNS, MPI_INT, 
                     top_neighbour, 4004,
                     &matrix[0][0], COLUMNS, MPI_INT, 
		     bottom_neighbour, 4004, 
                     cartesian_communicator, &status );

      MPI_Sendrecv ( &matrix[1][0], COLUMNS, MPI_INT, 
		     bottom_neighbour, 5005,
                     &matrix[ROWS - 1][0], COLUMNS, MPI_INT, 
                     top_neighbour, 5005,
                     cartesian_communicator, &status );

      MPI_Send ( matrix, COLUMNS * ROWS, MPI_INT, 0, 6006,
		 MPI_COMM_WORLD );
   }
   if ( my_rank == 0 ) collect_matrices ( matrix, 6006 );

   /* Here the processes in cartesian_communicator exchange the leftmost
      and rightmost columns of their matrices with their right and left
      neighbours. Because matrices in C are stored by rows, not by columns,
      we create a new stridden MPI data type, which extracts a column from 
      the matrix. Observe that this time, we only send one item across.
      That item corresponds to the whole column. 
    */

   if ( cartesian_communicator != MPI_COMM_NULL ) {

      MPI_Type_vector (ROWS, 1, COLUMNS, MPI_INT, &column_type);
      MPI_Type_commit (&column_type);

      MPI_Sendrecv ( &matrix[0][1], 1, column_type, 
		     left_neighbour, 7007,
                     &matrix[0][COLUMNS - 1], 1, column_type, 
                     right_neighbour, 7007,
                     cartesian_communicator, &status );
 
      MPI_Sendrecv ( &matrix[0][COLUMNS - 2], 1, column_type, 
                     right_neighbour, 8008,
                     &matrix[0][0], 1, column_type, 
		     left_neighbour, 8008,
                     cartesian_communicator, &status );

      MPI_Send ( matrix, COLUMNS * ROWS, MPI_INT, 0, 9009,
                 MPI_COMM_WORLD );

      MPI_Type_free( &column_type );
   }
   if ( my_rank == 0 ) collect_matrices ( matrix, 9009 );


   MPI_Comm_free( &cartesian_communicator );
   MPI_Finalize ();
}

int print_array ( array, vertical_break, horizontal_break )
int array[DISPLAY_ROWS][DISPLAY_COLUMNS];
int vertical_break, horizontal_break;
{
   int k, l;

   printf ("\n");
   for (k = DISPLAY_ROWS - 1; k >= 0; k -- ) {
      for (l = 0; l < DISPLAY_COLUMNS; l ++ ) {
	 if (l % horizontal_break == 0) printf (" ");
	 printf ( "%2d ", array [k][l] );
      }
      printf ( "\n" );
      if (k % vertical_break == 0) printf ( "\n" );
   }
}

int collect_matrices ( matrix, tag )
int matrix[ROWS][COLUMNS];
int tag;
{
   int client_matrix[ROWS][COLUMNS];
   int display[DISPLAY_ROWS][DISPLAY_COLUMNS];
   int i, j, k, l, source;
   MPI_Status status;
 
   for ( i = PROCESS_ROWS - 1; i >= 0; i -- ) {
      for ( j = 0; j < PROCESS_COLUMNS; j ++ ) {
	 source = process_matrix[i][j];
         MPI_Recv ( client_matrix, BUFSIZ, MPI_INT, world_rank_array[source], 
		    tag, MPI_COMM_WORLD, &status );
         for ( k = ROWS - 1; k >= 0; k -- ) {
            for ( l = 0; l < COLUMNS; l++ ) {
               display [i * ROWS + k] [j * COLUMNS + l] = client_matrix[k][l];
            }
         }
      }
   }
 
   print_array (display, ROWS, COLUMNS);
}
