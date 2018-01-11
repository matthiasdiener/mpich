#include "mpi.h"
#ifdef USE_GRAPHICS
#define MPE_GRAPHICS
#include "mpe.h"
#endif
#include <stdio.h>

/* for debugging: */

/* message types */
#define GUESS 0
#define ACCEPTED 1
#define NEW_INFO 2
#define EXIT 3
#define WON 4

/* graphical data */
#define HDIST 40
#define VDIST 50
#define ROWS 15
#define RADIUS 10
#define SCORE_RADIUS 3
#define SCORE_VDIST 8
#define SCORE_HDIST 8
#define SCORE_ROWS 3
#define SCORE_COLS 3
#define SCORE_WIDTH SCORE_COLS*SCORE_HDIST
#define WORKER_WIDTH 10
#define WORKER_HEIGHT 10
#define WORKER_HDIST 20
#define COLOURSCALE_WIDTH 20
#define COLOURSCALE_HDIST 30
#define SUCCESS_HEIGHT 4

/* limits of the mastermind board */
#ifdef USE_GRAPHICS
#define MAXCOLS  SCORE_ROWS*SCORE_COLS
#define MAXCOLOURS 10
#else
#define MAXCOLS  100
#define MAXCOLOURS 100
#endif
#define MAXGUESSES 100

/* global variables */
int numprocs;
int colours, columns;
int guesses_remaining;
int guess[MAXCOLS+1];
int secret[MAXCOLS];

#define GUESS_START 2

/* the structure of the board (should be a struct!)
   board[i][0]  board[i][1]  board[i][2] ...  board[i][columns+1] 
   -----------  -----------  -----------      -------------------
   bulls,       cows,        guess[0],   ..., guess[columns-1]

*/
int board[MAXGUESSES][MAXCOLS+2];
int sources[MAXGUESSES];
int next_row;

int freq_counter;
#define FREQUENCY 500

#ifdef USE_GRAPHICS
int height, width, left_col_width;
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

#ifdef USE_GRAPHICS
  if (numprocs > ROWS+1)
    {
      if (myid == 0)
	printf("Maximal number of processes is %d, exiting\n",
	       ROWS+1);
      MPI_Finalize();
      return;
    }
  else
#endif

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
  int segment_size;
  int numslaves = numprocs-1;
  int done = 0;
  int mystart;
  MPI_Status status;
  int i;
  int choices;
  int flag;
  int col_to_change;

  next_row = 0;
  initialize_mm(myid);

  choices = int_power(colours, columns);
  segment_size = choices/numslaves;

  mystart = (myid-1)*segment_size;
  if (myid < numslaves)
    guesses_remaining = segment_size - 1;
  else
    guesses_remaining = choices - 1 - (numslaves-1)*segment_size;
  
  for (i=columns-1; i>=0;  i--)
    {
      guess[i] = mystart % colours;
      mystart /= colours;
    }
  
  trace_guess("STARTING: ", myid, "\n");
  
  freq_counter = FREQUENCY;

  while(!done)
    {
      if (freq_counter-- == 0)
	{
#ifdef USE_GRAPHICS
	  draw_guess(myid-1, 1, guess, myid);
	  draw_progress(myid-1, segment_size, 0);
	  MPE_Update(handle);
#endif
	  MPI_Iprobe(0, EXIT, MPI_COMM_WORLD, &flag, &status);
	  if (flag == 1)
	    {
	      done = 1;
	      break;
	    }

	  do
	    {
	      MPI_Iprobe(0, NEW_INFO, MPI_COMM_WORLD, &flag, &status);
	      if (flag == 1)
		{
		  MPI_Recv(&board[next_row][0], columns+2, MPI_INT, 0, 
			   NEW_INFO, MPI_COMM_WORLD, &status);
#ifdef USE_GRAPHICS
		  draw_progress(myid-1, segment_size, 1);  
		  MPE_Update(handle);
#endif
		  printf("%d: NEW INFO, row num: %d\n", myid, next_row);
		  next_row++;
		}
	    } while (flag);
	  
	  freq_counter=FREQUENCY;
	}

      if (guess_consistent(myid, &col_to_change))
	{
#ifdef DEBUG	 
	  trace_guess("sending:  ", myid, "\n");
#endif
	  guess[columns] = next_row;
	  MPI_Send(guess, columns+1, MPI_INT, 0, GUESS, MPI_COMM_WORLD);
	  MPI_Recv(&board[next_row][0], columns+2, MPI_INT, 0, MPI_ANY_TAG, 
		   MPI_COMM_WORLD, &status);
	  switch (status.MPI_TAG)
	    {
	    case EXIT: 
	      done = 1;
	      break;
	    case WON:
	      done = 1;
#ifdef USE_GRAPHICS
	      draw_progress(myid-1, segment_size, 2);  
	      MPE_Update(handle);
#endif
	      break;
	    case ACCEPTED:
#ifdef USE_GRAPHICS
	      draw_progress(myid-1, segment_size, 2);  
	      MPE_Update(handle);
#endif
	      for (i=0; i<columns; i++)
		board[next_row][i+2] = guess[i];
	      next_row++;
	      next_guess(columns-1, &guesses_remaining);
	      break;
	    case NEW_INFO:
#ifdef USE_GRAPHICS
	      draw_progress(myid-1, segment_size, 1);  
	      draw_progress(myid-1, segment_size, -1);  
	      MPE_Update(handle);
#endif
	      printf("%d: NEW INFO, row num: %d\n", myid, next_row);
	      next_row++;

	      break;
	    default:
	      fprintf(stderr,"slave %d received invalid type %d\n", 
		      myid, status.MPI_TAG);
	      done = 1;
	    }
	}
      else
	{
#ifdef DEBUG	  
	  trace_guess("inconsis: ", myid, ", ");
	  printf("col_to_change = %d\n", col_to_change);
#endif
	  next_guess(col_to_change, &guesses_remaining);
	}
      if (guesses_remaining <= 0)
	{
	  done = 1;
	}
    }

  trace_guess("LAST:     ", myid, "\n");

#ifdef USE_GRAPHICS
  draw_guess(myid-1, 1, guess, myid);
  draw_progress(myid-1, segment_size, 0);
  MPE_Update(handle);
#endif
}

master()
{
  int row_num, source, bulls, cows, i, j;
  int numslaves = numprocs-1;
  int game_over = 0;
  MPI_Status status;

  while (1)
    {
      printf("Number of colours: ");
      fflush(stdout);
      scanf("%d", &colours);
      if (colours > 1 && colours <=MAXCOLOURS )
	break;
      else
	printf("Number of colours should be between 2 and %d\n", MAXCOLOURS);
    }

  while (1)
    {
      printf("Number of columns: ");
      fflush(stdout);
      scanf("%d", &columns);
      if (columns > 1 && columns <=MAXCOLS )
	break;
      else
	printf("Number of columns should be between 2 and %d\n", MAXCOLS);
    }
    
 
  get_secret();

  initialize_mm(0);

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

	  printf("%d: ", source);
	  print_guess("ACCEPTED: ", guess);
	  printf("(Bulls = %d, Cows = %d, Row = %d)\n", bulls, cows, next_row);
	  
	  if (bulls == columns) /* game over */
	    {
	      for (i = 1; i <= numslaves; i++)
		MPI_Send(NULL, 0, MPI_INT, i,
			 (i==source?WON:EXIT), MPI_COMM_WORLD);
	      game_over = 1;
	    }
	  else
	    {
	      for (i = 1; i <= numslaves; i++)
		if (i == source)
		  MPI_Send(&board[next_row][0], 2, MPI_INT, source, ACCEPTED, 
			   MPI_COMM_WORLD);
		else
		  MPI_Send(&board[row_num][0], columns+2, MPI_INT, i,
			   NEW_INFO, MPI_COMM_WORLD);
	    }
#ifdef USE_GRAPHICS
	  sources[next_row] = source;
	  if (next_row < ROWS)
	    {
	      draw_guess(next_row, 0, guess, source);
	      draw_score(next_row, bulls, cows);
	    }
	  else
	    for (i = next_row-ROWS+1, j = 0; i <=next_row; i++, j++)
	      {
		draw_guess(j, 0, &board[i][2], sources[i]);
		draw_score(j, board[i][0], board[i][1]);
	      }
	  MPE_Update(handle);
#endif
	  next_row++;
	}
    }

#ifdef USE_GRAPHICS
  printf("Any key to exit\n");
  scanf("%c",&i);
  scanf("%c",&i);
#endif
}

next_guess(col, remaining)
     int col;
     int *remaining;
{
  int i, pos = 1, cnt = 0;
  for (i = columns-1; i>=col+1; i--)
    {
      cnt += guess[i]*pos;   
      guess[i] = 0;
      pos *= colours;
    }
  (*remaining) -= pos - cnt;

  for (i = col; i>=0; i--)
    if (guess[i] < colours-1)
      {
	guess[i]++;
	break;
      }
    else /* guess[i] == colours-1 */
      guess[i] = 0;
}
	  

eval_guess(guess, code, bulls, cows)
     int guess[], code[];
     int *bulls, *cows;
{
  int i,j;
  int tmp[MAXCOLS];

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
}

/*
   col initially set to 'columns', i.e. the column after the last one
   (columns being numbered from 0 to columns-1).

   When an inconsistency with a row of the board is found, col is set to
   the column where the inconsistency is detected.

   We keep checking the board, so that further rows of the board can cause
   col to further move leftwards (I don't know if this pays off, perhaps
   worth profiling).

   If all rows tested and col still points to 'columns', than the guess is
   found to be consistent with the board.  
*/

guess_consistent(myid, col_to_change)
     int myid;
     int *col_to_change;
{
  int row, bulls, bullscows, col;
  int *board_row;
  int i,j;
  int tmp[MAXCOLS];

  col = columns;

  for (row = 0; row < next_row; row++)
    {
      board_row = &board[row][2];
      bulls = board[row][0];
      bullscows = board[row][1]+bulls;

      for (i=0; i<columns; i++)
	tmp[i] = board_row[i];

      for (i=0; i < col; i++)
	{
	  if (guess[i] == board_row[i]) /* bull */
	    {
	      if ((bulls--) <= 0)
	      break;
	    }
	  j = 0;
	  while ( j < columns && guess[i] != tmp[j])
	    j++;
	  if (j < columns ) /* bull or cow */
	    if (bullscows-- <= 0)
	      break;
	    else
	      tmp[j] = -1;	/* nonexistent colour */
	  if (bullscows >= columns-i)
	    break;
	}
      col = i;
    }

  if (col == columns)
    return 1;
  
  *col_to_change = col;
  return 0;      
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
}

#ifdef USE_GRAPHICS
draw_guess(row, col, guess, id)
     int row, col, id;
     int guess[];
{
  int i;
  int hpos = left_col_width*col+HDIST+WORKER_HDIST;
  int vpos = (row+2)*VDIST;
  
  MPE_Fill_rectangle(handle, hpos-(HDIST-2*RADIUS+WORKER_WIDTH), 
		     (int)(vpos-WORKER_HEIGHT/2), WORKER_WIDTH, WORKER_HEIGHT,
		     (MPE_Color) id);

  for (i=0; i<columns; i++)
    {
      MPE_Fill_circle( handle, hpos, vpos, RADIUS, (MPE_Color)(guess[i]+1) );
      hpos += HDIST;
    }
}

draw_score(row, bulls, cows)
     int row, bulls, cows;
{
  int r,c, i;
  int vpos = (row+2)*VDIST-RADIUS+SCORE_RADIUS;
  int hpos = left_col_width-HDIST-SCORE_WIDTH;

  for (r=0; r<SCORE_ROWS; r++)
    for (c=0; c<SCORE_COLS; c++)
      {
	i = SCORE_COLS*r+c;
	if (i < bulls)
	  MPE_Fill_circle( handle, hpos+SCORE_HDIST*c, 
			  vpos+SCORE_VDIST*r, SCORE_RADIUS, MPE_BLACK);
	else if (i < bulls+cows)
	  MPE_Draw_circle( handle, hpos+SCORE_HDIST*c, 
			  vpos+SCORE_VDIST*r, SCORE_RADIUS, MPE_BLACK);
	else
	  break;
      }
}

draw_progress(row, segment_size, success)
     int row, segment_size, success;
{
  float portion_done = (float) (segment_size-guesses_remaining)/segment_size;
  int hpos = left_col_width+HDIST+WORKER_HDIST-RADIUS;
  int vpos = (row+2)*VDIST+2*RADIUS;
  int length = (int)(portion_done* ((columns-1)*HDIST+2*RADIUS));

  MPE_Draw_line(handle, hpos, vpos, hpos+length, vpos, MPE_BLACK);
  if (success == 0)
    return;
  MPE_Draw_line(handle, hpos+length, vpos, hpos+length, 
		vpos-success*SUCCESS_HEIGHT, MPE_BLACK);
}

#endif

get_secret()
{
  int i;

  for (i=0; i<columns; i++)
    if (i<colours)
      secret[i] = colours-1-i;
    else
      secret[i] = 0;
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

initialize_mm(myid)
     int myid;
{
  int right_col_width, colourscale_width, i;
  
  MPI_Bcast(&colours, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
#ifdef USE_GRAPHICS
  left_col_width = WORKER_HDIST+(columns+2)*HDIST+SCORE_WIDTH;
  right_col_width = (columns+1)*HDIST;
  colourscale_width = (colours+2)*COLOURSCALE_HDIST;
  if (right_col_width < colourscale_width)
    right_col_width = colourscale_width;
  width = left_col_width+WORKER_HDIST+right_col_width;
  height = (ROWS+2)*VDIST + VDIST/2;
  
  MPE_Open_graphics( &handle, MPI_COMM_WORLD, (char*)0, 
		    -1, -1, width, height, MPE_GRAPH_INDEPDENT);

  if (myid > 0)
    return;

  for (i=0; i<columns; i++)
    {
      MPE_Fill_circle( handle, HDIST*(i+1)+WORKER_HDIST, 
		      (int)(0.6*VDIST), RADIUS, (MPE_Color)(secret[i]+1) );
    }
  for (i=0; i<colours; i++)
    {
      MPE_Fill_rectangle(handle, left_col_width+HDIST+WORKER_HDIST-RADIUS
			 +i*COLOURSCALE_HDIST, (int)(0.6*VDIST)-RADIUS, 
			 COLOURSCALE_WIDTH, 2*RADIUS,
			 (MPE_Color) (i+1));
    }
  MPE_Draw_line(handle, 0, (int)(1.3*VDIST), width, (int)(1.3*VDIST), 
		MPE_BLACK);
  MPE_Draw_line(handle, 0, (int)(1.4*VDIST), width, (int)(1.4*VDIST), 
		MPE_BLACK);
  MPE_Draw_line(handle, (int)(left_col_width-0.3*HDIST), 0, 
		(int)(left_col_width-0.3*HDIST), height,
		MPE_BLACK);
  MPE_Draw_line(handle, (int)(left_col_width-0.4*HDIST), 0, 
		(int)(left_col_width-0.4*HDIST), height,
		MPE_BLACK);
#endif
}

trace_guess(txt1, id, txt2)
char *txt1, *txt2;
int id;
{
  printf("%d: ", id);
  print_guess(txt1, guess);
  printf(", guesses_remaining = %d%s", guesses_remaining, txt2);
}
