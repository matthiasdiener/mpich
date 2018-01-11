




#include <stdio.h>
#include <stdio.h>

#include "mpi.h"
extern int __NUMNODES, __MYPROCID;MPI_Status _mpi_status;static int _n;
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
/* 
    This file contains routines to generate output from the mpptest programs
 */

/* 
   In order to simplify the generation of graphs of multiple data sets, 
   we want to allow the generated cit code to contain the necessary 
   window selection commands.  To do this, we add arguments for
   -wx i n    windows in x, my # and total #
   -wy i n    windows in y, my # and total #
   -lastwindow generate the wait/new page.
 */
typedef struct {
    FILE *fp, *fpdata;
    char *fname2;
    void (*header)();
    void (*dataout)();
    void (*draw)();
    void (*endpage)();
    /* For collective operations */
    void (*headergop)();    
    void (*dataoutgop)();
    void (*drawgop)();
    /* Information about the graph */
    int wxi, wxn, wyi, wyn, is_lastwindow;
    } GraphData;

void PrintGraphHelp( )
{
fprintf( stderr, "\n\
Output\n\
  -cit         Generate data for CIt (default)\n\
  -gnuplot     Generate data for GNUPLOT\n\
  -fname filename             (default is stdout)\n\
               (opened for append, not truncated)\n\
  -noinfo      Do not generate plotter command lines or rate estimate\n\
  -wx i n      windows in x, my # and total #\n\
  -wy i n      windows in y, my # and total #\n\
  -lastwindow  generate the wait/new page (always for 1 window)\n" );
}


void HeaderCIt( ctx, protocol_name, title_string, units )
GraphData *ctx;
char *protocol_name, *title_string, *units;
{
char archname[20], hostname[256], date[30];

fprintf( ctx->fp, "set default\nset font variable\n" );
fprintf( ctx->fp, "set curve window y 0.15 0.90\n" );
if (ctx->wxn > 1 || ctx->wyn > 1) 
    fprintf( ctx->fp, "set window x %d %d y %d %d\n", 
	     ctx->wxi, ctx->wxn, ctx->wyi, ctx->wyn );
fprintf( ctx->fp, "set order d d d x y d\n" );
fprintf( ctx->fp, "title left 'time (us)', bottom 'Size %s',\n", units );
strcpy(archname,"MPI" );
MPI_Get_processor_name(hostname,&_n);
strcpy(date , "Not available" );
/* For systems without a date routine, just leave off the data */
if (!date[0] || strcmp( "Not available", date ) == 0) {
    fprintf( ctx->fp, 
	"      top 'Comm Perf for %s (%s)',\n 'type = %s'\n", 
	archname, hostname, protocol_name );
    }
else {
    fprintf( ctx->fp, 
	"      top 'Comm Perf for %s (%s)',\n 'on %s',\n 'type = %s'\n", 
	archname, hostname, date, protocol_name );
    }
fprintf( ctx->fp, "\n#p0\tp1\tdist\tlen\tave time (us)\trate\n");
fflush( ctx->fp );
}

void HeaderForGopCIt( ctx, test_name, title_string, units )
GraphData *ctx;
char *test_name, *title_string, *units;
{
char archname[20], hostname[256], date[30];

fprintf( ctx->fp, "set default\nset font variable\n" );
fprintf( ctx->fp, "set curve window y 0.15 0.90\n" );
if (ctx->wxn > 1 || ctx->wyn > 1) 
    fprintf( ctx->fp, "set window x %d %d y %d %d\n", 
	     ctx->wxi, ctx->wxn, ctx->wyi, ctx->wyn );
fprintf( ctx->fp, "title left 'time (us)', bottom 'Processes',\n" );
strcpy(archname,"MPI" );
MPI_Get_processor_name(hostname,&_n);
strcpy(date , "Not available" );
/* For systems without a date routine, just leave off the data */
if (!date[0] || strcmp( "Not available", date ) == 0) {
    fprintf( ctx->fp, 
	"      top 'Comm Perf for %s (%s)',\n 'type = %s'\n", 
	archname, hostname, test_name );
    }
else {
    fprintf( ctx->fp, 
	"      top 'Comm Perf for %s (%s)',\n 'on %s',\n 'type = %s'\n", 
	archname, hostname, date, test_name );
    }
fprintf( ctx->fp, "\n#np time (us) for various sizes\n");
fflush( ctx->fp );
}

void DataoutGraph( ctx, proc1, proc2, distance, len, t, mean_time, rate )
GraphData *ctx;
int     proc1, proc2, distance, len;
double  t, mean_time, rate;
{
fprintf( ctx->fpdata, "%d\t%d\t%d\t%d\t%f\t%.2f\n",
	    proc1, proc2, distance, len, mean_time*1.0e6, rate );
}

void DataoutGraphForGop( ctx, len, t, mean_time, rate )
GraphData *ctx;
int     len;
double  t, mean_time, rate;
{
fprintf( ctx->fpdata, "%f ", mean_time*1.0e6 );
fflush( ctx->fpdata );
}

void DataendForGop( ctx )
GraphData *ctx;
{
fprintf( ctx->fpdata, "\n" );
}

void DatabeginForGop( ctx, np )
GraphData *ctx;
int       np;
{
fprintf( ctx->fpdata, "%d ", np );
}

void RateoutputGraph( ctx, sumlen, sumtime, sumlentime, sumlen2, ntest, S, R )
GraphData *ctx;
double  sumlen, sumtime, sumlentime, sumlen2;
int     ntest;
double  *S, *R;
{
double  s, r;

PIComputeRate( sumlen, sumtime, sumlentime, sumlen2, ntest, &s, &r );
s = s * 0.5;
r = r * 0.5;
fprintf( ctx->fp, "# Model complexity is (%e + n * %e)\n", s, r );
fprintf( ctx->fp, "# startup = " );
if (s > 1.0e-3)
    fprintf( ctx->fp, "%.2f msec ", s * 1.0e3 );
else
    fprintf( ctx->fp, "%.2f usec ", s * 1.0e6 );
fprintf( ctx->fp, "and transfer rate = " );
if (r > 1.e-6)
    fprintf( ctx->fp, "%.2f Kbytes/sec\n", 1.0e-3 / r );
else
    fprintf( ctx->fp, "%.2f Mbytes/sec\n", 1.0e-6 / r );

*S = s;
*R = r;
}

void DrawCIt( ctx, first, last, s, r )
GraphData *ctx;
int     first, last;
double  s, r;
{

/* Convert to one-way performance */
fprintf( ctx->fp, "plot square\njoin\n" );
/* fit some times fails in Gnuplot; use the s and r parmeters instead */
/* fit '1'+'x'\njoin dots\n   */
fprintf( ctx->fp, "set function x %d %d '%f+%f*x'\n", 
	 first, last, s*1.0e6, r*1.0e6 );
fprintf( ctx->fp, "join dots\n" );
}

void DrawGopCIt( ctx, first, last, s, r, nsizes, sizelist )
GraphData *ctx;
int     first, last, nsizes, *sizelist;
double  s, r;
{
int i, j;
/* Do this in reverse order to help keep the scales correct */
fprintf( ctx->fp, "set limits ymin 0\n" );
for (i=nsizes-1; i>=0; i--) {
    fprintf( ctx->fp, "set order x" );
    for (j=0; j<i; j++)
	fprintf( ctx->fp, " d" );
    fprintf( ctx->fp, " y" );
    for (j=i+1; j<nsizes; j++) 
	fprintf( ctx->fp, " d" );
    fprintf( ctx->fp, "\nplot square\njoin '%d'\n", sizelist[i] );
    }
}

/*
   Redisplay using rate instead of time (not used yet)
 */
void ChangeToRate( ctx, n_particip )
GraphData *ctx;
int     n_particip;
{
fprintf( ctx->fp, "set order d d d x d d y\njoin\n" );
}

/*
   Generate an end-of-page
 */
void EndPageCIt( ctx )
GraphData *ctx;
{
if (ctx->is_lastwindow)
    fprintf( ctx->fp, "wait\nnew page\n" );
}

/*
    GNUplot output 
 */
void HeaderGnuplot( ctx, protocol_name, title_string, units )
GraphData *ctx;
char *protocol_name, *title_string, *units;
{
char archname[20], hostname[256], date[30];

fprintf( ctx->fp, "set xlabel \"Size %s\"\n", units );
fprintf( ctx->fp, "set ylabel \"time (us)\"\n" );
strcpy(archname,"MPI" );
MPI_Get_processor_name(hostname,&_n);
strcpy(date , "Not available" );
if (!date[0] || strcmp( "Not available", date ) == 0) {
    fprintf( ctx->fp, "set title \"Comm Perf for %s (%s) type %s\"\n", 
	archname, hostname, protocol_name );
    }
else {
    fprintf( ctx->fp, "set title \"Comm Perf for %s (%s) on %s type %s\"\n", 
	    archname, hostname, date, protocol_name );
    }
fprintf( ctx->fpdata, "\n#p0\tp1\tdist\tlen\tave time (us)\trate\n");
fflush( ctx->fp );
}

void HeaderForGopGnuplot( ctx, protocol_name, title_string, units )
GraphData *ctx;
char *protocol_name, *title_string, *units;
{
char archname[20], hostname[256], date[30];

fprintf( ctx->fp, "set xlabel \"Processes\"\n" );
fprintf( ctx->fp, "set ylabel \"time (us)\"\n" );
strcpy(archname,"MPI" );
MPI_Get_processor_name(hostname,&_n);
strcpy(date , "Not available" );
if (!date[0] || strcmp( "Not available", date ) == 0) {
    fprintf( ctx->fp, "set title \"Comm Perf for %s (%s) type %s\"\n", 
	archname, hostname, protocol_name );
    }
else {
    fprintf( ctx->fp, "set title \"Comm Perf for %s (%s) on %s type %s\"\n", 
	    archname, hostname, date, protocol_name );
    }
fprintf( ctx->fpdata, "\n#np time (us) for various sizes\n");
fflush( ctx->fp );
}

void DrawGnuplot( ctx, first, last, s, r )
GraphData *ctx;
int     first, last;
double  s, r;
{

#ifdef GNUVERSION_HAS_BOXES
fprintf( ctx->fp, "plot '%s' using 4:5 with boxes,\\\n\
'%s' using 4:7 with lines,\\\n", ctx->fname2, ctx->fname2 );
fprintf( ctx->fp, "%f+%f*x with dots\n", 
	 s*1.0e6, r*1.0e6  );
#else
fprintf( ctx->fp, "plot '%s' using 4:5 with lines,\\\n", ctx->fname2 );
fprintf( ctx->fp, "%f+%f*x with dots\n", 
	 s*1.0e6, r*1.0e6  );
#endif
}

void DrawGopGnuplot( ctx, first, last, s, r, nsizes, sizelist )
GraphData *ctx;
int     first, last, nsizes, *sizelist;
double  s, r;
{
int i;
for (i=0; i<nsizes; i++) {
#ifdef GNUVERSION_HAS_BOXES
    fprintf( ctx->fp, "plot '%s' using 1:%d with boxes,\\\n\
'%s' using 1:%d with lines,\\\n", ctx->fname2, i+2, ctx->fname2, i+2 );
#else
    fprintf( ctx->fp, "plot '%s' using 1:%d with lines,\\\n", 
	     ctx->fname2, i+2 );
#endif
    }
}

/*
   Generate an end-of-page
 */
void EndPageGnuplot( ctx )
GraphData *ctx;
{
if (ctx->is_lastwindow)
    fprintf( ctx->fp, "pause -1 \"Press <return> to continue\"\nclear\n" );
}

/* Common operations */
void HeaderGraph( ctx, protocol_name, title_string, units )
GraphData *ctx;
char *protocol_name, *title_string, *units;
{
(*ctx->header)( ctx, protocol_name, title_string, units );
}

void HeaderForGopGraph( ctx, protocol_name, title_string, units )
GraphData *ctx;
char *protocol_name, *title_string, *units;
{
(*ctx->headergop)( ctx, protocol_name, title_string, units );
}

void DrawGraph( ctx, first, last, s, r )
GraphData *ctx;
int     first, last;
double  s, r;
{
(*ctx->draw)( ctx, first, last, s, r ) ;
}

void DrawGraphGop( ctx, first, last, s, r, nsizes, sizelist )
GraphData *ctx;
int     first, last, nsizes, sizelist;
double  s, r;
{
(*ctx->drawgop)( ctx, first, last, s, r, nsizes, sizelist ) ;
}

void EndPageGraph( ctx )
GraphData *ctx;
{
(*ctx->endpage)( ctx );
}

/* Common create */
void *SetupGraph( argc, argv )
int *argc;
char **argv;
{
GraphData *new;
char     filename[1024];
int      wsize[2];
int      isgnu;

new = (GraphData *)malloc(sizeof(GraphData));    if (!new)return 0;;

filename[0] = 0;
isgnu = SYArgHasName( argc, argv, 1, "-gnuplot" );
if (SYArgHasName( argc, argv, 1, "-cit" )) isgnu = 0;
if (SYArgGetString( argc, argv, 1, "-fname", filename, 1024 ) &&
    __MYPROCID == 0) {
    new->fp = fopen( filename, "a" );
    if (!new->fp) {
	fprintf( stderr, "Could not open file %s\n", filename );
	return 0;
	}
    }
else 
    new->fp = stdout;
/* Graphics layout */
new->wxi = 1;
new->wxn = 1;
new->wyi = 1;
new->wyn = 1;
if (SYArgGetIntVec( argc, argv, 1, "-wx", 2, wsize )) {
    new->wxi           = wsize[0];
    new->wxn           = wsize[1];
    }
if (SYArgGetIntVec( argc, argv, 1, "-wy", 2, wsize )) {
    new->wyi           = wsize[0];
    new->wyn           = wsize[1];
    }
new->is_lastwindow = SYArgHasName( argc, argv, 1, "-lastwindow" );
if (new->wxn == 1 && new->wyn == 1) new->is_lastwindow = 1;

if (!isgnu) {
    new->header	    = HeaderCIt;
    new->dataout    = DataoutGraph;
    new->headergop  = HeaderForGopCIt;
    new->dataoutgop = DataoutGraphForGop;
    new->draw	    = DrawCIt;
    new->drawgop    = DrawGopCIt;
    new->endpage    = EndPageCIt;
    new->fpdata	    = new->fp;
    new->fname2	    = 0;
    }
else {
    char filename2[256];
    new->header	    = HeaderGnuplot;
    new->dataout    = DataoutGraph;
    new->headergop  = HeaderForGopGnuplot;
    new->dataoutgop = DataoutGraphForGop;
    new->draw	    = DrawGnuplot;
    new->drawgop    = DrawGopGnuplot;
    new->endpage    = EndPageGnuplot;
    sprintf( filename2, "%s.gpl", (filename[0] ? filename : "mppout" ) );
    new->fpdata	    = fopen( filename2, "w" );
    if (!new->fpdata) {
	fprintf( stderr, "Could not open file %s\n\
used for holding data for GNUPLOT\n", filename2 );
	return 0;
	}
    new->fname2 = (char *)malloc((unsigned)(strlen(filename2 ) + 1 ));
    strcpy( new->fname2, filename2 );
    }
return (void *)new;
}
