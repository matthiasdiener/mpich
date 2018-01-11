

#include <stdio.h>
#include "mpi.h"
/* #include "comm/examples/angst/angst.h" */
MPI_Status _mpi_status, _mpi_sstatus;
int __NUMNODES, __MYPROCID;


/*
    This is a simple program to test the communications performance of
    a parallel machine.  The program tcomm does a more exhaustive
    test of the individual links

    Note on the codeing style of this program.
    This program has grown tremendously over time and contains many 
    dubious practices, such as extensive use of global variables.  This
    is not encouraged; at some time, I'll clean up this program. 
 */
    
double exchange_forcetype();
double exchange_async();
double exchange_sync();

double round_trip_sync();
double round_trip_force();
double round_trip_async();

double RunSingleTest();

typedef enum { HEADtoHEAD, ROUNDTRIP } CommType;
typedef enum { Blocking, NonBlocking, ReadyReceiver } Protocol;

/* These statics (globals) are used to estimate the parameters in the
   basic (s + rn) complexity model */
static double sumtime = 0.0, sumlentime = 0.0;
static double sumlen  = 0,  sumlen2 = 0;
static int    ntest   = 0;

/* If doinfo is 0, don't write out the various text lines */
static int    doinfo = 1;
/* In order to simplify the generation of graphs of multiple data sets, 
   we want to allow the generated cit code to contain the necessary 
   window selection commands.  To do this, we add arguments for
   -wx i n    windows in x, my # and total #
   -wy i n    windows in y, my # and total #
   -lastwindow generate the wait/new page.
 */
static int wxi = 1, wxn = 1, 
           wyi = 1, wyn = 1,
           is_lastwindow = 1;

/* This is the number of times to run a test, taking as time the minimum
   achieve timing.  Eventually, we should combine this with an adaptive 
   method that uses this as a limit, but also stops when, say, three values 
   are within a few percent of the current minimum */
static int    minreps       = 30;
static int    minThreshTest = 3;
static double repsThresh    = 0.05;
static int    NatThresh     = 3;
static char   *protocol_name;
/* 
   We would also like to adaptively modify the number of repetitions to 
   meet a time estimate (later, we'd like to meet a statistical estimate).
   
   One relatively easy way to do this is to use a linear estimate (either
   extrapolation or interpolation) based on 2 other computations.
   That is, if the goal time is T and the measured tuples (time,reps,len)
   are, the formula for the local time is s + r n, where

   r = (time2/reps2 - time1/reps1) / (len2 - len1)
   s = time1/reps1 - r * len1

   Then the appropriate number of repititions to use is

   Tgoal / (s + r * len) = reps
 */
static double Tgoal = 1.0;
/* If less than Tgoalmin is spent, increase the number of repititions */
static double TgoalMin = 0.5;
static int    AutoReps = 0;

/* This structure allows a collection of arbitray sizes to be specified */
#define MAX_SIZE_LIST 256
static int sizelist[MAX_SIZE_LIST];
static int nsizes = 0;


/* This is used to contain results for a single test */
typedef struct {
    double len, t, mean_time, rate;
    int    reps;
    } TwinResults;
typedef struct {
    double (*f)();
    int    reps, proc1, proc2;
    /* Here is where we should put "recent" timing data used to estimate
       the values of reps */
    double t1, t2;
    int    len1, len2;
    } TwinTest;

/* 
   This function manages running a test and putting the data into the 
   accumulators.  The information is placed in result.  

   This function is intended for use by TSTAuto1d
 */
double GeneralF( x, result, ctx )
double      x;
TwinResults *result;
TwinTest    *ctx;
{
double t, mean_time;
int    len = (int)x, k;
int    reps = ctx->reps;
int    flag, iwork;

/*
if (__MYPROCID == ctx->proc1) 
    printf( "testing len = %d\n", len ); 
 */

if (AutoReps) {
    if (ctx->t1 > 0 && ctx->t2 > 0) 
	reps = ComputeGoodReps( ctx->t1, ctx->len1, ctx->t2, ctx->len2, len );
    MPI_Bcast(&reps, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD );
    }

t = RunSingleTest( ctx->f, reps, len, ctx->proc1, ctx->proc2 );

mean_time         = t / reps;              /* take average over trials */
result->t         = t;
result->len       = x;
result->reps      = reps;
result->mean_time = mean_time;
result->rate      = ((double)len) / mean_time;

/* Save the most recent timing data */
ctx->t1     = ctx->t2;
ctx->len1   = ctx->len2;
ctx->t2     = mean_time;
ctx->len2   = len;
 
sumlen     += len;
sumtime    += mean_time;
sumlen2    += ((double)len) * ((double)len);
sumlentime += mean_time * len;
ntest      ++;

/* We need to insure that everyone gets the same result */
MPI_Bcast(&result->rate, sizeof(double), MPI_BYTE, 0, MPI_COMM_WORLD );
return result->rate;
}

int main(argc,argv)
int argc;
char *argv[];
{
int c;
double (* f)();
long reps,proc1,proc2,len,error_flag,distance_flag,distance;
double t;
long first,last,incr, svals[3];
CommType comm_type = ROUNDTRIP;
Protocol protocol  = Blocking;
char     filename[1024];
FILE     *fp = stdout;
int      autosize = 0, autodx;
double   autorel  = 0.02;
int      wsize[2];

MPI_Init( &argc, &argv );

MPI_Comm_rank( MPI_COMM_WORLD, &__MYPROCID );
MPI_Comm_size( MPI_COMM_WORLD, &__NUMNODES );

SYusc_init();
protocol_name = "blocking";

if (SYArgHasName( &argc, argv, 1, "-help" )) {
  if (__MYPROCID != 0) return 0;
  fprintf( stderr, "%s - test individual communication speeds\n", argv[0] );
  fprintf( stderr, 
"[-head | -roundtrip ] [-sync | -async | -force] [-size start end stride]\n\
[-reps n] [-fname name]\n\
Test a single communication link by various methods.  The tests are \n\
combinations of\n\
  Protocol: \n\
  -sync        Blocking sends/receives    (default)\n\
  -async       NonBlocking sends/receives\n\
  -force       Ready-receiver (with a null message)\n\
\n\
  Message pattern:\n\
  -roundtrip   Roundtrip messages         (default)\n\
  -head        Head-to-head messages\n\
    \n" );
  fprintf( stderr, 
"  Message sizes:\n\
  -size start end stride                  (default 0 1024 32)\n\
               Messages of length (start + i*stride) for i=0,1,... until\n\
               the length is greater than end.\n\
  -sizelist n1,n2,...\n\
               Messages of length n1, n2, etc are used.  This overrides \n\
               -size\n\
  -auto        Compute message sizes automatically (to create a smooth\n\
               graph.  Use -size values for lower and upper range\n\
  -autodx n    Minimum number of bytes between samples when using -auto\n\
  -autorel d   Relative error tolerance when using -auto (0.02 by default)\n");

  fprintf( stderr, "\n\
  Number of tests\n\
  -reps n      Number of times message is sent (default 1000)\n\
  -autoreps    Compute the number of times a message is sent automatically\n\
  -tgoal  d    Time that each test should take, in seconds.  Use with \n\
               -autoreps\n\
  -rthresh d   Fractional threshold used to determine when minimum time\n\
               has been found.  The default is 0.05.\n\
\n\
  Output\n\
  -fname filename             (default is stdout)\n\
  -noinfo      Do not generate plotter command lines or rate estimate\n\
  -wx i n      windows in x, my # and total #\n\
  -wy i n      windows in y, my # and total #\n\
  -lastwindow  generate the wait/new page (always for 1 window)\n" );
  return 0;
  }

if (__NUMNODES < 2) {
    fprintf( stderr, "Must run twin with at least 2 nodes\n" );
    return 1;
    }
/* This depends on proc1 always being 0 */
if (__MYPROCID == 0 && 
    SYArgGetString( &argc, argv, 1, "-fname", filename, 1024 )) {
    fp = fopen( filename, "w" );
    if (!fp) {
	fprintf( stderr, "Could not open file %s\n", filename );
	return 1;
	}
    }
/* Graphics layout */
if (SYArgGetIntVec( &argc, argv, 1, "-wx", 2, wsize )) {
    wxi           = wsize[0];
    wxn           = wsize[1];
    is_lastwindow = 0;
    }
if (SYArgGetIntVec( &argc, argv, 1, "-wy", 2, wsize )) {
    wyi           = wsize[0];
    wyn           = wsize[1];
    is_lastwindow = 0;
    }
is_lastwindow = SYArgHasName( &argc, argv, 1, "-lastwindow" );
if (wxn == 1 && wyn == 1) is_lastwindow = 1;

/* Virtual front panel display */
#if defined(VFP)
  PVFPSetup( (char *)0, 0, 0 );  /* PVFP will determine the display */
#endif
reps          = DEFAULT_REPS;
proc1         = 0;
proc2         = __NUMNODES-1;
error_flag    = 0;
distance_flag = 0;
f             = round_trip_sync;
svals[0]      = 0;
svals[1]      = 1024;
svals[2]      = 32;

if (SYArgHasName( &argc, argv, 1, "-force" )) {
    protocol      = ReadyReceiver;
    protocol_name = "ready receiver";
    }
if (SYArgHasName( &argc, argv, 1, "-async" )) {
    protocol      = NonBlocking;
    protocol_name = "nonblocking";
    }
if (SYArgHasName( &argc, argv, 1, "-sync"  )) {
    protocol      = Blocking;
    protocol_name = "blocking";
    }
if (SYArgHasName( &argc, argv, 1, "-head"  ))     comm_type = HEADtoHEAD;
if (SYArgHasName( &argc, argv, 1, "-roundtrip" )) comm_type = ROUNDTRIP;
if (SYArgHasName( &argc, argv, 1, "-distance" ))  distance_flag++;
SYArgGetIntVec( &argc, argv, 1, "-size", 3, svals );
SYArgGetInt(    &argc, argv, 1, "-reps", &reps );
if (SYArgHasName( &argc, argv, 1, "-autoreps" ))  AutoReps  = 1;
if (SYArgGetDouble( &argc, argv, 1, "-tgoal", &Tgoal )) {
                                                  AutoReps = 1;
    if (TgoalMin > 0.1 * Tgoal) TgoalMin = 0.1 * Tgoal;
    }
SYArgGetDouble( &argc, argv, 1, "-rthresh", &repsThresh );
if (SYArgHasName( &argc, argv, 1, "-noinfo" ))    doinfo    = 0;

nsizes = SYArgGetIntList( &argc, argv, 1, "-sizelist", MAX_SIZE_LIST, 
                          sizelist );

autosize = SYArgHasName( &argc, argv, 1, "-auto" );
if (autosize) {
    autodx = 4;
    SYArgGetInt( &argc, argv, 1, "-autodx", &autodx );
    autorel = 0.02;
    SYArgGetDouble( &argc, argv, 1, "-autorel", &autorel );
    }

if (comm_type == ROUNDTRIP) {
    switch( protocol ) {
	case ReadyReceiver: f = round_trip_force; break;
	case NonBlocking:   f = round_trip_async; break;
	case Blocking:      f = round_trip_sync;  break;
	}
    }
else {
    switch( protocol ) {
	case ReadyReceiver: f = exchange_forcetype; break;
	case NonBlocking:   f = exchange_async;     break;
	case Blocking:      f = exchange_sync;      break;
	}
    }
first = svals[0];
last  = svals[1];
incr  = svals[2];

if(distance_flag) {
   for(distance=1;distance<1+network_width();distance++) {
      proc2 = network_proc_by_dist(distance,proc1);
      time_function(reps,first,last,incr,proc1,proc2,f,fp,
                    autosize,autodx,autorel);
      }
  }
else{
    time_function(reps,first,last,incr,proc1,proc2,f,fp,
                  autosize,autodx,autorel);
    }
MPI_Finalize();    
}

time_function(reps,first,last,incr,proc1,proc2,f,fp,
              autosize,autodx,autorel)
long   reps,first,last,incr,proc1,proc2,autosize,autodx;
double autorel;
double (* f)();
FILE   *fp;
{
  long   len,distance,myproc;
  int    k;
  double mean_time,rate;
  double t;
  double s, r;
  double T1, T2;
  int    Len1, Len2;
  int    flag, iwork;


  
  myproc   = __MYPROCID;
  distance = network_distance(proc1,proc2);
  if (doinfo && myproc == proc1) {
      char archname[20], hostname[256], date[30];
      fprintf( fp, "\n#p0\tp1\tdist\tlen\ttime\t\tave time (us)\trate\n");
      fprintf( fp, "set default\nset font variable\n" );
      fprintf( fp, "set curve window y 0.15 0.90\n" );
      if (wxn > 1 || wyn > 1) 
	  fprintf( fp, "set window x %d %d y %d %d\n", wxi, wxn, wyi, wyn );
      fprintf( fp, "set order d d d x d y d\n" );
      fprintf( fp, "title left 'time (us)', bottom 'Size (bytes)',\n" );
      SYGetArchType( archname, 20 );
      SYGetHostName( hostname, 256 );
      SYGetDate( date );
      fprintf( fp, 
	      "      top 'Comm Perf for %s (%s)',\n 'on %s',\n 'type = %s'\n", 
	       archname, hostname, date, protocol_name );
      fflush( fp );
      }

  ntest = 0;
  if (autosize) {
      int    maxvals = 256, nvals, i;
      int    dxmax;
      TwinTest ctx;
      TwinResults *results;
      
      /* We should really set maxvals as 2+(last-first)/autodx */
      results  = (TwinResults *)MALLOC( maxvals * sizeof(TwinResults) );
      CHKPTR(results);
      ctx.reps = reps;
      ctx.f    = f;
      ctx.proc1= proc1;
      ctx.proc2= proc2;
      ctx.t1   = 0.0;
      ctx.t2   = 0.0;
      ctx.len1 = 0;
      ctx.len2 = 0;

      /* We need to pick a better minimum resolution */
      dxmax = (last - first) / 16;
      /* make dxmax a multiple of 4 */
      dxmax = (dxmax & ~0x3);
      if (dxmax < 4) dxmax = 4;
      
      nvals = TSTAuto1d( (double)first, (double)last, (double)autodx,
			 (double)dxmax, autorel, 1.0e-10, 
			 results, sizeof(TwinResults),
			 maxvals, GeneralF, &ctx );
      if (myproc == proc1) {
	  TSTRSort( results, sizeof(TwinResults), nvals );
	  for (i = 0; i < nvals; i++) {
	      fprintf(fp, "%d\t%d\t%d\t%d\t%f\t%f\t%.2f\n",
                      proc1,proc2,distance,(int)results[i].len,
		      results[i].t,results[i].mean_time*1.0e6,results[i].rate);

	      }
	  }
      }
  else {
      T1 = 0;
      T2 = 0;
      if (nsizes) {
	  int i;
	  for (i=0; i<nsizes; i++) {
	      len = sizelist[i];
	      if (AutoReps) {
		  if (T1 > 0 && T2 > 0) 
		      reps = ComputeGoodReps( T1, Len1, T2, Len2, len );
		  MPI_Bcast(&reps, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD );
		  }
	      t = RunSingleTest( f, reps, len, proc1, proc2 );
	      mean_time = t;
	      mean_time = mean_time / reps;  /* take average over trials */
	      rate      = ((double)len)/mean_time;
	      if(myproc==proc1) {
		  fprintf(fp, "%d\t%d\t%d\t%d\t%f\t%f\t%.2f\n",
			  proc1,proc2,distance,len,t,mean_time*1.0e6,rate);
		  fflush( fp );
		  }
	      
	      T1   = T2;
	      Len1 = Len2;
	      T2   = mean_time;
	      Len2 = len;
	      }
	  }
      else {
	  for(len=first;len<=last;len+=incr){
	      if (AutoReps) {
		  if (T1 > 0 && T2 > 0) 
		      reps = ComputeGoodReps( T1, Len1, T2, Len2, len );
		  MPI_Bcast(&reps, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD );
		  }
	      t = RunSingleTest( f, reps, len, proc1, proc2 );
	      mean_time = t;
	      mean_time = mean_time / reps;  /* take average over trials */
	      rate      = ((double)len)/mean_time;
	      if(myproc==proc1) {
		  fprintf(fp, "%d\t%d\t%d\t%d\t%f\t%f\t%.2f\n",
			  proc1,proc2,distance,len,t,mean_time*1.0e6,rate);
		  fflush( fp );
		  }
	      T1   = T2;
	      Len1 = Len2;
	      T2   = mean_time;
	      Len2 = len;
	      }
	  }
      }

if (doinfo && myproc == proc1) {
    PIComputeRate( sumlen, sumtime, sumlentime, sumlen2, ntest, &s, &r );
    fprintf( fp, "# Model complexity is (%e + n * %e)\n", s, r );
    fprintf( fp, "# startup = " );
    if (s > 1.0e-3)
	fprintf( fp, "%.2f msec ", s * 1.0e3 );
    else
	fprintf( fp, "%.2f usec ", s * 1.0e6 );
    fprintf( fp, "and transfer rate = " );
    if (r > 1.e-6)
	fprintf( fp, "%.2f Kbytes/sec\n", 1.0e-3 / r );
    else
	fprintf( fp, "%.2f Mbytes/sec\n", 1.0e-6 / r );
    fprintf( fp, 
"# These are round trip values; one way values can be estimated as\n" );
    s = s * 0.5;
    r = r * 0.5;
    fprintf( fp, "# startup = " );
    if (s > 1.0e-3)
	fprintf( fp, "%.2f msec ", s * 1.0e3 );
    else
	fprintf( fp, "%.2f usec ", s * 1.0e6 );
    fprintf( fp, "and transfer rate = " );
    if (r > 1.e-6)
	fprintf( fp, "%.2f Kbytes/sec\n", 1.0e-3 / r );
    else
	fprintf( fp, "%.2f Mbytes/sec\n", 1.0e-6 / r );
    /* Convert to one-way performance */
    fprintf( fp, "change y 'x/2'\nplot square\njoin\n" );
    /* fit some times fails in cit; use the s and r parmeters instead */
    /* fit '1'+'x'\njoin dots\n   */
    fprintf( fp, "set function x %d %d '%f+%f*x'\n", 
	         first, last, s*1.0e6, r*1.0e6 );
    fprintf( fp, "join dots\n" );
    if (is_lastwindow)
	fprintf( fp, "wait\nnew page\n" );
    }

}

/* Synchronous exchange (head-to-head) */
double exchange_sync(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double elapsed_time;
  long i,msg_id,myproc;
  char *sbuffer,*rbuffer;
  double t0, t1;

  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  myproc = __MYPROCID;
  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&_mpi_status);
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Send(sbuffer,len,MPI_BYTE,proc2,1,MPI_COMM_WORLD);
      MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
    for(i=0;i<reps;i++){
      MPI_Send(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
      MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&_mpi_status);
    }
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/* Asynchronous exchange (head-to-head) */
double exchange_async(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double         elapsed_time;
  long           i,myproc;
  ASYNCRecvId_t  msg_id;
  char           *sbuffer,*rbuffer;
  double   t0, t1;

  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  myproc = __MYPROCID;
  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&_mpi_status);  	
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(msg_id));
      MPI_Send(sbuffer,len,MPI_BYTE,proc2,1,MPI_COMM_WORLD);
      MPI_Wait(&(msg_id),&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(msg_id));
      MPI_Send(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
      MPI_Wait(&(msg_id),&_mpi_status);
    }
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/* head-to-head exchange using forcetypes.  This uses null messages to
   let the sender know when the receive is ready */
double exchange_forcetype(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double         elapsed_time;
  long           i,myproc, d1, *dmy = &d1;
  ASYNCRecvId_t  msg_id;
  char           *sbuffer,*rbuffer;
  double   t0, t1;

  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  myproc = __MYPROCID;
  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,3,MPI_COMM_WORLD,&_mpi_status);
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&(msg_id));
      MPI_Send(NULL,0,MPI_BYTE,proc2,2,MPI_COMM_WORLD);
      MPI_Recv(dmy,0,MPI_BYTE,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&_mpi_status);
      MPI_Rsend(sbuffer,len,MPI_BYTE,proc2,0,MPI_COMM_WORLD);
      MPI_Wait(&(msg_id),&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,3,MPI_COMM_WORLD);
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&(msg_id));
      MPI_Send(NULL,0,MPI_BYTE,proc1,2,MPI_COMM_WORLD);
      MPI_Recv(dmy,0,MPI_BYTE,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&_mpi_status);
      MPI_Rsend(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
      MPI_Wait(&(msg_id),&_mpi_status);
    }
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/* This is a round trip (always unidirectional) */
double round_trip_sync(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double elapsed_time;
  double mean_time;
  long i,pid,myproc;
  char *rbuffer,*sbuffer;
  double t0, t1;

  myproc = __MYPROCID;
  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&_mpi_status);
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Send(sbuffer,len,MPI_BYTE,proc2,1,MPI_COMM_WORLD);
      MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
    for(i=0;i<reps;i++){
      MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&_mpi_status);
      MPI_Send(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
    }
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/* This is a round trip (always unidirectional) */
double round_trip_force(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double elapsed_time;
  double mean_time;
  long i,pid,myproc;
  char *rbuffer,*sbuffer;
  double t0, t1;
  ASYNCRecvId_t rid;

  myproc = __MYPROCID;
  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&_mpi_status);
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
      MPI_Rsend(sbuffer,len,MPI_BYTE,proc2,1,MPI_COMM_WORLD);
      MPI_Wait(&(rid),&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
    for(i=0;i<reps-1;i++){
      MPI_Wait(&(rid),&_mpi_status);
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
      MPI_Rsend(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
    }
    MPI_Wait(&(rid),&_mpi_status);
    MPI_Rsend(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/* This is a round trip (always unidirectional) */
double round_trip_async(reps,len,proc1,proc2)
long reps,len,proc1,proc2;
{
  double elapsed_time;
  double mean_time;
  long i,pid,myproc;
  char *rbuffer,*sbuffer;
  double t0, t1;
  ASYNCRecvId_t rid;

  myproc = __MYPROCID;
  sbuffer = (char *)malloc(len);
  rbuffer = (char *)malloc(len);

  elapsed_time = 0;
  if(myproc==proc1){
    MPI_Recv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&_mpi_status);
    *(&t0)=MPI_Wtime();
    for(i=0;i<reps;i++){
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
      MPI_Send(sbuffer,len,MPI_BYTE,proc2,1,MPI_COMM_WORLD);
      MPI_Wait(&(rid),&_mpi_status);
    }
    *(&t1)=MPI_Wtime();
    elapsed_time = *(&t1 )-*(&t0);
  }

  if(myproc==proc2){
    MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,0,MPI_COMM_WORLD);
    for(i=0;i<reps-1;i++){
      MPI_Wait(&(rid),&_mpi_status);
      MPI_Irecv(rbuffer,len,MPI_BYTE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&(rid));
      MPI_Send(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
    }
    MPI_Wait(&(rid),&_mpi_status);
    MPI_Send(sbuffer,len,MPI_BYTE,proc1,1,MPI_COMM_WORLD);
  }

  free(sbuffer);
  free(rbuffer);
  return(elapsed_time);
}

/*
   This routine computes a good number of repititions to use based on 
   previous computations
 */
int ComputeGoodReps( t1, len1, t2, len2, len )
double t1, t2;
int    len1, len2, len;
{
double s, r;
int    reps;

r = (t2 - t1) / (len2 - len1);
s = t1 - r * len1;

reps = Tgoal / (s + r * len );
 
if (reps < 1) reps = 1;

return reps;
}


/*
  This runs the tests for a single size.  It adapts to the number of 
  tests necessary to get a reliable value for the minimum time.
  It also keeps track of the average and maximum times (which are unused
  for now).

  We can estimate the variance of the trials by using the following 
  formulas:

  variance = (1/N) sum (t(i) - (s+r n(i))**2
           = (1/N) sum (t(i)**2 - 2 t(i)(s + r n(i)) + (s+r n(i))**2)
	   = (1/N) (sum t(i)**2 - 2 s sum t(i) - 2 r sum t(i)n(i) + 
	      sum (s**2 + 2 r s n(i) + r**2 n(i)**2))
  Since we compute the parameters s and r, we need only maintain
              sum t(i)**2
              sum t(i)n(i)
              sum n(i)**2
  We already keep all of these in computing the (s,r) parameters; this is
  simply a different computation.

  In the case n == constant (that is, inside a single test), we can use
  a similar test to estimate the variance of the individual measurements.
  In this case, 

  variance = (1/N) sum (t(i) - s**2
           = (1/N) sum (t(i)**2 - 2 t(i)s + s**2)
	   = (1/N) (sum t(i)**2 - 2 s sum t(i) + sum s**2)
  Here, s = sum t(i)/N
  (For purists, the divison should be slightly different from (1/N) in the
  variance formula.  I'll deal with that later.)

 */
double RunSingleTest( f, reps, len, proc1, proc2 )
double (*f)();
int    reps, proc1, proc2;
{
int    flag, k, iwork, natmin;
double t, tmin, mean_time, tmax, tsum;


flag   = 0;
tmin   = 1.0e+38;
tmax   = tsum = 0.0;
natmin = 0;

for (k=0; k<minreps; k++) {
    MPI_Allreduce(&flag, &flag, 1, MPI_INT,MPI_SUM,MPI_COMM_WORLD );
    if (flag > 0) break;
    t = (* f) (reps,len,proc1,proc2);
    if (__MYPROCID == proc1) {
	tsum += t;
	if (t > tmax) tmax = t;
	if (t < tmin) {
	    tmin   = t;
	    natmin = 0;
	    }
	else if (minThreshTest < k && tmin * (1.0 + repsThresh) > t) {
	    /* This time is close to the minimum; use this to decide
	       that we've gotten close enough */
	    natmin++;
	    if (natmin >= NatThresh) 
		flag = 1;
	    }
	}
    }

mean_time  = tmin / reps;
sumlen     += len;
sumtime    += mean_time;
sumlen2    += ((double)len)*((double)len);
sumlentime += mean_time * len;
ntest      ++;

return tmin;
}
