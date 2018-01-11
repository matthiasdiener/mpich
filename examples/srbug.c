
#include "mpi.h"

#define SIZE 4
#define TYPE MPI_SHORT_INT

struct {
  short   val; /* <-- change this if you change the "TYPE" */
  int    loc;
} din[SIZE+1], dout[SIZE+1];

main (argc, argv)
int argc;
char **argv;
{
  int rank, size, i, bufsize;
  int dest, src;
  char *buf;
  int position = 0;
  int errors = 0;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
  MPI_Comm_size ( MPI_COMM_WORLD, &size );

  for (i=0; i<SIZE; i++) {
    din[i].val = rank+1+i;
    din[i].loc = rank+1+i;
  }
  dout[SIZE].val = -1;
  dout[SIZE].loc = -1;

  MPIR_Pack_size ( SIZE, TYPE, MPI_COMM_WORLD, &bufsize );
  buf = (char *)malloc(bufsize+8);
  for (i=0; i<8; i++) buf[bufsize+i] = 'e';
  MPIR_Pack ( din, SIZE, TYPE, buf );
  position = 0;
  MPIR_Unpack ( dout, SIZE, TYPE, buf );
  for (i=0; i<SIZE; i++) {
    if (din[i].val!=dout[i].val)
      errors++;
    if (din[i].loc!=dout[i].loc)
      errors++;
    dout[i].val = 0;
    dout[i].loc = 0;
  }
  if (dout[SIZE].val != -1 || dout[SIZE].loc != -1) {
      errors++;
      fprintf( stdout, "Overwrite of dout\n" );
      }
  for (i=0; i<8; i++) {
      if (buf[bufsize+i] != 'e') {
	  errors++;
	  fprintf( stdout, "Overwrite of buf\n" );
	  }
      }
  MPIR_Pack_size ( SIZE, TYPE, MPI_COMM_WORLD, &bufsize );
  free(buf);
  buf = (char *)malloc(bufsize+8);
  for (i=0; i<8; i++) buf[bufsize+i] = 'e';
  MPIR_Pack ( din, SIZE, TYPE, buf );
  position = 0;
  MPIR_Unpack ( dout, SIZE, TYPE, buf );

  for (i=0; i<SIZE; i++) {
    if (din[i].val!=dout[i].val)
      errors++;
    if (din[i].loc!=dout[i].loc)
      errors++;
    dout[i].val = 0;
    dout[i].loc = 0;
  }
  if (dout[SIZE].val != -1 || dout[SIZE].loc != -1) {
      errors++;
      fprintf( stdout, "Overwrite of dout\n" );
      }
  for (i=0; i<8; i++) {
      if (buf[bufsize+i] != 'e') {
	  errors++;
	  fprintf( stdout, "Overwrite of buf\n" );
	  }
      }

  if (errors)
    printf("[%d] ERRORS - Unsuccessful pack/unpack test!\n",rank);
  else
    printf("[%d] Successful pack/unpack test!\n",rank);

  errors = 0;

  dest = 0;
  src  = size - 1;
  if (rank == src) {
    MPI_Send ( din, SIZE, TYPE, dest, 0, MPI_COMM_WORLD );
    MPI_Send ( din, SIZE, TYPE, dest, 0, MPI_COMM_WORLD );
    MPI_Send ( din, SIZE, TYPE, dest, 0, MPI_COMM_WORLD );
  }
  if (rank == dest) {
    MPI_Status st;
    MPI_Recv ( dout, SIZE, TYPE, src, 0, MPI_COMM_WORLD, &st );
    for (i=0; i<SIZE; i++) {
      if (src+1+i != dout[i].val)
        errors++;
      if (src+1+i != dout[i].loc)
        errors++;
      dout[i].val = 0;
      dout[i].loc = 0;
    }
    if (dout[SIZE].val != -1 || dout[SIZE].loc != -1) {
	errors++;
	fprintf( stdout, "Overwrite of dout\n" );
	}
    if (errors)
      printf("[%d] failed 1st send -- errors = %d\n",rank,errors);
    else
      printf("[%d] Successful 1st send\n",rank);
    errors = 0;

    MPI_Recv ( dout, SIZE, TYPE, src, 0, MPI_COMM_WORLD, &st );
    for (i=0; i<SIZE; i++) {
      if (src + 1 + i !=dout[i].val)
        errors++;
      if (src + 1 + i !=dout[i].loc)
        errors++;
      dout[i].val = 0;
      dout[i].loc = 0;
    }
    if (dout[SIZE].val != -1 || dout[SIZE].loc != -1) {
	errors++;
	fprintf( stdout, "Overwrite of dout\n" );
	}
    if (errors)
      printf("[%d] failed 2nd send -- errors = %d\n",rank,errors);
    else
      printf("[%d] Successful 2nd send\n",rank);
    errors = 0;

    MPI_Recv ( dout, SIZE, TYPE, src, 0, MPI_COMM_WORLD, &st );
    for (i=0; i<SIZE; i++) {
      if (src + 1 + i!=dout[i].val) {
	  fprintf( stdout, "Got (%d) %d expected %d (for val)\n", 
		   i, dout[i].val, src + 1 + i );
	  errors++;
	  }
      if (src + 1 + i !=dout[i].loc) {
	  fprintf( stdout, "Got (%d) %d expected %d (for loc)\n", 
		   i, dout[i].loc, src + 1 + i);
	  errors++;
	  }
      dout[i].val = 0;
      dout[i].loc = 0;
    }
    if (dout[SIZE].val != -1 || dout[SIZE].loc != -1) {
	errors++;
	fprintf( stdout, "Overwrite of dout\n" );
	}
    if (errors)
      printf("[%d] failed 3rd send -- errors = %d\n",rank,errors);
    else
      printf("[%d] Successful 3rd send\n",rank);
    errors = 0;
  }

  MPI_Finalize();
}

