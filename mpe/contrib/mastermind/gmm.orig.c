#include "mpi.h"
#ifdef USE_GRAPHICS
#define MPE_GRAPHICS
#include "mpe.h"
#endif
#include <stdio.h>

#define MAXCOLS  100
#define MAXGUESSES 100

#define GUESS 0
#define ACCEPTED 1
#define NEW_INFO 2
#define EXIT 3

#define HDIST 40
#define VDIST 40
#define ROWS 10
#define RADIUS 10

int numprocs;

int colours, columns;

int guess[MAXCOLS+1];

int secret[MAXCOLS];

/* the structure of the board (should be a struct!)

   bulls, cows, guess[0], ..., guess[columns-1]

*/
int board[MAXGUESSES][MAXCOLS+2];
int next_row;

int freq_counter;
#define FREQUENCY 200

#ifdef USE_GRAPHICS
int height, width;
MPE_XGraph handle;
#endif

main(argc, argv)
int argc;
char *argv[];
{
  int myid;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0)
    master();
  else
    slave(myid);

#ifdef USE_GRAPHICS
  MPE_Close_graphics(&handle);
#endif
  MPI_Finalize();
}

slave(myid)
int myid;
{
  int guesses_remaining;
  int segment_size;
  int numslaves = numprocs-1;
  int done = 0;
  int mystart;
  MPI_Status status;
  int i;
  int choices;
  int flag;

  next_row = 0;
  MPI_Bcast(&colours, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);

#ifdef USE_GRAPHICS
  width = 2*(columns+1)*HDIST;
  height = ROWS*VDIST;

  MPE_Open_graphics( &handle, MPI_COMM_WORLD, (char*)0, 
		     -1, -1, width, height, MPE_GRAPH_INDEPDENT);
#endif

  choices = int_power(colours, columns);
  segment_size = choices/numslaves;

  mystart = (myid-1)*segment_size;
  if (myid < numslaves)
    guesses_remaining = segment_size;
  else
    guesses_remaining = choices - (numslaves-1)*segment_size;
  
  for (i=columns-1; i>=0;  i--)
    {
      guess[i] = mystart % colours;
      mystart /= colours;
    }
  
  printf("[%d] ",myid);
  print_guess("My start: ", guess);
  
  freq_counter = FREQUENCY;

  while(!done)
    {
      if (freq_counter-- == 0)
	{
#ifdef USE_GRAPHICS
	  draw_guess(myid-1,1,guess);
#endif
	  MPI_Iprobe(0, EXIT, MPI_COMM_WORLD, &flag, &status);
	  if (flag == 1)
	    {
	      done = 1;
	      break;
	    }
	  freq_counter=FREQUENCY;
	}
      if (guess_consistent())
	{
	  guess[columns] = next_row;
	  MPI_Send(guess, columns+1, MPI_INT, 0, GUESS, MPI_COMM_WORLD);
	  MPI_Recv(&board[next_row][0], columns+2, MPI_INT, 0, MPI_ANY_TAG, 
		   MPI_COMM_WORLD, &status);
	  switch (status.MPI_TAG)
	    {
	    case EXIT: 
	      done = 1;
	      break;
	    case ACCEPTED:
	      for (i=0; i<columns; i++)
		board[next_row][i+2] = guess[i];
	      next_row++;
	      next_guess();
	      break;
	    case NEW_INFO:
	      next_row++;
	      break;
	    default:
	      fprintf(stderr,"slave %d received invalid type %d\n", 
		      myid, status.MPI_TAG);
	      done = 1;
	    }
	}
      else
	next_guess();
    }
#ifdef USE_GRAPHICS
  draw_guess(myid-1,1,guess);
#endif
}

master()
{
  int row_num, source, bulls, cows, i;
  int numslaves = numprocs-1;
  int game_over = 0;
  double starttime, endtime;
  MPI_Status status;

  printf("Number of colours: ");
  fflush(stdout);
  scanf("%d", &colours);
  printf("Number of columns: ");
  fflush(stdout);
  scanf("%d", &columns);
 
  get_secret();

  MPI_Bcast(&colours, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);

#ifdef USE_GRAPHICS
  width = 2*(columns+1)*HDIST;
  height = ROWS*VDIST;
  MPE_Open_graphics( &handle, MPI_COMM_WORLD, (char*)0, 
		     -1, -1, width, height, MPE_GRAPH_INDEPDENT); 
#endif

  starttime = MPI_Wtime();

  while(!game_over)
    {
      MPI_Recv(guess, columns+1, MPI_INT, MPI_ANY_SOURCE, GUESS, 
		  MPI_COMM_WORLD, &status);
      source = status.MPI_SOURCE;
      row_num = guess[columns];
      if (row_num == next_row)
	{
	  eval_guess(guess, secret, &bulls, &cows);

	  board[next_row][0] = bulls;
	  board[next_row][1] = cows;
	  for (i=0; i<columns; i++)
	    board[next_row][i+2] = guess[i];

	  print_guess("Accepted: ", guess);
	  printf(" From: %d,  Bulls = %d, Cows = %d\n", source, bulls, cows);
	  
	  if (bulls == columns) /* game over */
	    {
	      for (i = 1; i <= numslaves; i++)
		MPI_Send(NULL, 0, MPI_INT, i , EXIT, MPI_COMM_WORLD);
	      game_over = 1;
	    }
	  else
	    MPI_Send(&board[next_row][0], 2, MPI_INT, source, ACCEPTED, 
		     MPI_COMM_WORLD);
#ifdef USE_GRAPHICS
	  draw_guess(next_row, 0, guess);
#endif
	  next_row++;
	}
      else /* row_num < next_row */
	{
	  MPI_Send(&board[row_num][0], columns+2, MPI_INT, source, 
		   NEW_INFO, MPI_COMM_WORLD);
	}
    }

  endtime = MPI_Wtime();
  print_guess("secret code: ", secret);
  printf("%d slaves, %f seconds\n", numslaves, endtime - starttime);

#ifdef USE_GRAPHICS
  printf("Any key to exit\n");
  scanf("%c",&i);
  scanf("%c",&i);
#endif
}

next_guess()
{
  int i;

  for (i = columns-1; i>=0; i--)
    if (guess[i] < colours-1)
      {
	guess[i]++;
	break;
      }
    else /* guess[i] == colours-1 */
      guess[i] = 0;
}
	  
guess_consistent()
{
  int row, bulls, cows;

  for (row = 0; row < next_row; row++)
    {
      eval_guess(guess, &board[row][2], &bulls, &cows);
      if (! ( bulls == board[row][0] && cows == board[row][1]))
	return 0;
    }
  
  return 1;
}

eval_guess(guess, code, bulls, cows)
     int guess[], code[];
     int *bulls, *cows;
{
  int i,j;
  int tmp[MAXCOLS];

/*  print_guess("eval, guess:", guess);
    print_guess("eval, code: ", code);
*/
  for (i=0; i<columns; i++)
    tmp[i] = guess[i];

  *bulls = 0;
  *cows = 0;
  for (i=0; i<columns; i++)
    if (guess[i] == code[i])
      (*bulls)++;
  for (i=0; i<columns; i++)
    for (j=0; j<columns; j++)
      if (code[i] == tmp[j])
	{
	  (*cows)++;
	  tmp[j] = -1; /* nonexistent colour */
	  break;
	}
  *cows -= *bulls;
/*  printf("eval: bulls = %d, cows = %d\n", *bulls, *cows); */
}

print_guess(text, guess)
     char *text;
     int guess[];
{
  int i;

  printf("%s", text);
  for (i=0; i<columns; i++)
    {
      printf("%d ", guess[i]);
    }
  printf("\n");
  fflush(stdout);
}

#ifdef USE_GRAPHICS
draw_guess(row, col, guess)
     int row, col;
     int guess[];
{
  int i;
  int hpos = (columns+1)*HDIST*col+HDIST;
  int vpos = (row+1)*VDIST;
  
  for (i=0; i<columns; i++)
    {
      MPE_Fill_circle( handle, hpos, vpos, RADIUS, (MPE_Color)(guess[i]+1) );
      hpos += HDIST;
    }
  MPE_Update(handle);
}
#endif

get_secret()
{
  int i;

  for (i=0; i<columns; i++)
    secret[i] = i;
}

int_power(n, m)
int n,m;
{
  int i;
  int pw = 1;

  for (i=0; i<m; i++)
    pw*=n;

  return pw;
}
