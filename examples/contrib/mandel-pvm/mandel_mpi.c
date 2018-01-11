/*
 * mandel_mpi.c
 *
 * Mandelbrot calculator. Calculates strips from the requested region,
 * concanenates strips, and writes to file.
 *
 * Usage: mandel_mpi width height x1 y1 x2 y2 filename ( mandel_mpi 256 256
 * -2.0 -2.0 2.0 2.0 whole_mandelbrot )
 *
 * last revision: 15 Feb 1993 
 *                25 Jan 1993 Anthony Skjellum tony@cs.msstate.edu
 *
 * This is a major re-write of the PVM 2.x code by Robert Manchek, last dated 22
 * September, 1991.  Robert Manchek's version is available through netlib,
 * ftp: netlib2.cs.utk.edu.
 *
 * 15-Feb-94...
 * Note: *) the program does not currently work properly with the NEW_STYLE mapping.
 *       *) the program does not do 2D decomposition yet.
 *       *) no further info on the particular mandelbrot set is documented in original
 *          program.
 *       *) the program writes a file suitable for the "xpx" application, rather
 *          than rendering in parallel via the MPE library (new).
 */

#include <mpi.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

/* constants */
#define OLD_STYLE	/* currently, use OLD_STYLE of tile mapping */
#undef NEW_STYLE
			
#define MIN_WIDHGT 1
#define MAX_WIDHGT 2048
#define out_of_bounds(lambda) ((lambda < MIN_WIDHGT) || (lambda > MAX_WIDHGT))

/* for the model implementation */
#define MPICH

/* message tags: */
#define MANDEL_INT_TAG  1000
#define MANDEL_REAL_TAG 1001
#define MANDEL_TILE_TAG 1002

/* typedefs */
typedef struct _Mandel
{
    int wd;
    int ht;
    double x1, y1, x2, y2;
    
}   Mandel;

/* prototypes... */
void init_mandelbrot();
void retrieve_mandelbrot();
void calc_mandel_tile();

/* global variables... */

/* DONE */

main(argc, argv)
     int argc;
     char **argv;
{
    MPI_Comm worker_comm;
    int nproc;
    int my_rank;
    
    int error = 0;
    Mandel mandel, work;
    
    char *pix;		/* image data */
    char *local_pix;	/* local image data */
    int fd;			/* output file */
    
    int i;
    char *dfn;		/* name of file to write to */
    
    /* enroll in mpi */
    MPI_Init(&argc, &argv);

    /* work in a duplicate of MPI comm world, for good etiquette: */
    MPI_Comm_dup(MPI_COMM_WORLD,  &worker_comm);
    MPI_Comm_size(worker_comm, &nproc);
    MPI_Comm_rank(worker_comm, &my_rank);
    
    if (my_rank == 0)
    {
/*      if (argc != 8) {  */ /* P4 command line bug */
	if (argc < 8)
	{
	    fputs("usage: mandel_main width height x1 y1 x2 y2 filename\n",
		  stderr);
	    error = 1;
	}
	else
	{
	    mandel.wd = atoi(argv[1]);
	    mandel.ht = atoi(argv[2]);
	    mandel.x1 = atof(argv[3]);
	    mandel.y1 = atof(argv[4]);
	    mandel.x2 = atof(argv[5]);
	    mandel.y2 = atof(argv[6]);
	    dfn = argv[7];
	    
	    if (out_of_bounds(mandel.wd) || out_of_bounds(mandel.ht))
	    {
		fprintf(stderr, "Width and height must be between %d and %d\n",
			MIN_WIDHGT, MAX_WIDHGT);
		error = 2;
	    }
	    else
	    {
		if (nproc > mandel.wd)
		    nproc = mandel.wd;
		fprintf(stderr, "using %d tile servers in calculation\n", nproc);
	    }
	}
    }
    MPI_Bcast(&error, 1, MPI_INT, 0, worker_comm);	/* everyone tests for
							   error */
    if (error != 0)
    {
	MPI_Finalize();
	return;
    }
    
    /* do detailed initialization and work... */
    init_mandelbrot(&mandel, &work, worker_comm);
    calc_mandel_tile(&work, &local_pix);
    retrieve_mandelbrot(&mandel, &work, local_pix, &pix, worker_comm);
    
    if (my_rank == 0)
    {
	fputs("writing output file\n", stderr);
	if ((fd = open(dfn, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
	{
	    perror(dfn);
	}
	else
	{
	    (void) write(fd, pix, mandel.wd * mandel.ht);
	    (void) close(fd);
	    fprintf(stderr, "%s: written\n", dfn);
	}
    }
    MPI_Comm_free(&worker_comm);
    MPI_Finalize();
}

void
    init_mandelbrot(mandel, work, comm)
Mandel *mandel, *work;
MPI_Comm comm;
{
    int p;
    double delta_x;
    
    int integers[2];
    double reals[4];
    
    int rank;
    int nproc;
    MPI_Status status;
    
    int local_ht;
    int local_wd;
    
    double local_x1;
    double local_y1;
    double local_x2;
    double local_y2;
    
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nproc);
    
    if (rank == 0)
    {
	/* divide up area into strips and assign them to processors */
	
#ifdef OLD_STYLE
	int *tpos = (int *) malloc((nproc + 1) * sizeof(int));
#endif
	
	delta_x = mandel->x2 - mandel->x1;
	local_ht = mandel->ht;	/* constant for vertical strip
				   decomposition */
	local_y1 = mandel->y1;
	local_y2 = mandel->y2;
	
#ifdef OLD_STYLE
	local_x1 = mandel->x1;
	tpos[0] = 0;
#endif
	
	for (p = 0; p < nproc; p++)
	{
	    
#ifdef OLD_STYLE
	    tpos[p + 1] = ((p + 1) * mandel->wd) / nproc;
	    local_wd = tpos[p + 1] - tpos[p];
	    local_x2 = (tpos[p + 1] * delta_x) / mandel->wd + mandel->x1;
#endif
	    
#ifdef NEW_STYLE
	    local_wd = (mandel->wd + p) / nproc;	/* linear, load balanced *
							   distribution */
	    local_x1 = ((p * delta_x) / nproc) + mandel->x1;
	    local_x2 = (((p + 1) * delta_x) / nproc) + mandel->x1;
#endif
	    
	    /* pack the quantities... */
	    integers[0] = local_wd;
	    integers[1] = local_ht;
	    reals[0] = local_x1;
	    reals[1] = local_y1;
	    reals[2] = local_x2;
	    reals[3] = local_y2;
	    if (p != 0)
	    {
		MPI_Send(integers, 2, MPI_INT, p, MANDEL_INT_TAG, comm);
		MPI_Send(reals, 4, MPI_DOUBLE, p, MANDEL_REAL_TAG, comm);
	    }
	    else
	    {
		work->x1 = reals[0];
		work->y1 = reals[1];
		work->x2 = reals[2];
		work->y2 = reals[3];
		work->wd = integers[0];
		work->ht = integers[1];
	    }
	    
#ifdef OLD_STYLE
	    local_x1 = local_x2;
#endif
	}

#ifdef OLD_STYLE
	free(tpos);
#endif
    }
    else
    {
	MPI_Recv(integers, 2, MPI_INT, 0, MANDEL_INT_TAG, comm, &status);
	MPI_Recv(reals, 4, MPI_DOUBLE, 0, MANDEL_REAL_TAG, comm, &status);
	
	work->wd = integers[0];
	work->ht = integers[1];
	work->x1 = reals[0];
	work->y1 = reals[1];
	work->x2 = reals[2];
	work->y2 = reals[3];
	
    }
}

void calc_mandel_tile(work, pix)/* DONE */
     Mandel *work;
     char **pix;
{
    char *calc_tile();
    
    *pix = calc_tile(work->x1, work->y1, work->x2, work->y2,
		     work->wd, work->ht);
}

char *
    calc_tile(x1, y1, x2, y2, wd, ht)	/* DONE */
double x1, y1, x2, y2;		/* tile corner coords */
int wd, ht;			/* size of tile */
{
    char *pix;		/* calculated image */
    int ix, iy;		/* loop indices */
    double x, y;		/* pixel coords */
    double ar, ai, a;	/* accums */
    int ite;		/* number of iter until divergence */
    
    double delta_x, delta_y;
    int rank;
    
    pix = (char *) malloc(wd * ht);
    if (pix == NULL)
	return (NULL);
    
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    
    for (iy = 0; iy < ht; iy++)
    {
	y = (iy * delta_y) / ht + y1;
	for (ix = 0; ix < wd; ix++)
	{
	    x = (ix * delta_x) / wd + x1;
	    ar = x;
	    ai = y;
	    for (ite = 0; ite < 255; ite++)
	    {
		a = (ar * ar) + (ai * ai);
		if (a > 4.0)
		    break;
		a = ar * ar - ai * ai;
		ai *= 2 * ar;
		ar = a + x;
		ai += y;
	    }
	    pix[iy * wd + ix] = ~ite;
	}
    }
    return (pix);
}

void
    retrieve_mandelbrot(mandel, work, local_pix, pix, comm)
Mandel *mandel, *work;
char *local_pix;
char **pix;
MPI_Comm comm;
{
    
    int rank, nproc;
    int i;
    int *displs, *counts;
    char *tile;
    int twd;
    MPI_Status status;
    char *ba1, *ba2;
    int y;
    
    /* get size of communicator and my rank in it */
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nproc);
    
    /* collect results and patch together */
    
    /* calculate displacement and counts arrays */
    if (rank == 0)
    {
	displs = (int *) malloc((nproc + 1) * sizeof(int));
	for (i = 0; i <= nproc; i++)
	    displs[i] = ((i * mandel->wd) / nproc);
	(*pix) = (char *) malloc(mandel->wd * mandel->ht);
	
	/* Store my own local pixmap */
	ba1 = local_pix;
	ba2 = (*pix);
	for (y = mandel->ht; y-- > 0;)
	{
	    memcpy(ba2, ba1, displs[1]);
	    ba1 += displs[1];
	    ba2 += mandel->wd;
	}
	
	tile = (char *) malloc((work->wd + 1) * mandel->ht);
	fputs("processors responding:", stderr);
	fflush(stderr);
	
	for (i = 1; i < nproc; i++)
	{
	    twd = displs[i + 1] - displs[i];
	    /* we need MPI_Probe before we can write this like
	       PVM version */
	    MPI_Recv(tile, twd * mandel->ht, MPI_BYTE, i, MANDEL_TILE_TAG, comm, &status);
	    fprintf(stderr, " %d", i);
	    fflush(stderr);
	    ba1 = tile;
	    ba2 = (*pix) + displs[i];
	    for (y = mandel->ht; y-- > 0;)
	    {
		memcpy(ba2, ba1, twd);
		ba1 += twd;
		ba2 += mandel->wd;
	    }
	}
	
	free(tile);
	free(displs);
    }
    
    /* Else I'm not node 0, send my results to 0 */
    else
	MPI_Send(local_pix, work->wd * work->ht, MPI_BYTE, 0, MANDEL_TILE_TAG, comm);
    
    free(local_pix);
}
