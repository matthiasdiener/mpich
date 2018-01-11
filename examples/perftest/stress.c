int __NUMNODES, __MYPROCID  ;




#include <stdio.h>
/* #undef LOGCOMMDISABLE */
#include <sys/types.h>       
#include <sys/time.h>        
#include <stdio.h>

#include "mpi.h"
extern int __NUMNODES, __MYPROCID;static MPI_Status _mpi_status;static int _n, _MPILEN;



#define STRESS_PRINT_INTERVAL 60
/*
    This is a simple program to stress the communications performance of
    a parallel machine.  The program tcomm does a more exhaustive
    test of the individual links.

    In order to have "safe(0,0)" versions of these for the blocking case,
    we need to order the sends and receives so that there is always a
    consumer of messages. 
 */
    
int EachToAll(), AllToAll(), AllToAllNB(), AllToAllPhased();

#define NPATTERNS 12
static long Patterns[12] = {
    0xffffffff, 0xaaaaaaaa, 0x88888888, 0x80808080, 0x80008000, 0x80000000,
    0x00000000, 0x55555555, 0x77777777, 0x7f7f7f7f, 0x7fff7fff, 0x7fffffff };
static double bytes_sent;

typedef enum { Blocking, NonBlocking } Protocol;

int main(argc,argv)
int argc;
char *argv[];
{
int c;
int (* f)();
long len,size, pattern;
long first,last,incr, svals[3];
Protocol protocol  = Blocking;
int      toall     = 0, isphased = 0;
FILE     *fp = stdout;
int      err, curerr;
struct timeval endtime, currenttime, nextprint, starttime;
char     ttime[50];
int      BeVerbose = 0;
int      loopcount;
double   bytes_so_far, dwork;

MPI_Init( &argc, &argv );
MPI_Comm_size( MPI_COMM_WORLD, &__NUMNODES );
MPI_Comm_rank( MPI_COMM_WORLD, &__MYPROCID );
;

if (SYArgHasName( &argc, argv, 1, "-help" )) {
  if (__MYPROCID != 0) return 0;
  PrintHelp( argv );
  return 0;
  }

if (__NUMNODES < 2) {
    fprintf( stderr, "Must run stress with at least 2 nodes\n" );
    return 1;
    }
f             = EachToAll;
svals[0]      = 32;
svals[1]      = 1024;
svals[2]      = 32;
SYGetDayTime( &endtime );
SYGetDayTime( &currenttime );
starttime = currenttime;
nextprint = currenttime;

if (SYArgHasName( &argc, argv, 1, "-async" ))     protocol  = NonBlocking;
if (SYArgHasName( &argc, argv, 1, "-sync"  ))     protocol  = Blocking;
toall = SYArgHasName( &argc, argv, 1, "-all" );
SYArgGetIntVec( &argc, argv, 1, "-size", 3, svals );
isphased = SYArgHasName( &argc, argv, 1, "-phased" );
if (SYArgGetString( &argc, argv, 1, "-ttime", ttime, 50 )) {
    endtime.tv_sec = SYhhmmtoSec( ttime ) + currenttime.tv_sec;
    }

#if defined(PRGS) && !defined(TOOLSNOX11)
/* This enables a running display of the progress of the test */
if (SYArgHasName( &argc, argv, 1, "-pimonitor" ))
    PPRGSetup( &argc, argv, 1, 0, 0 );
#endif

f        = EachToAll;
switch( protocol ) {
    case NonBlocking:
        if (toall) {
	    f = AllToAllNB;
	    if (__MYPROCID == 0) 
		fprintf( stdout, "All to All non-blocking\n" );
	    }
        else {
            fprintf( stderr, "NonBlocking (-async) not yet supported\n" );
            return 1;
        }
        break;
    case Blocking:      
        if (toall) {
           if (isphased) {
	       f = AllToAllPhased;
	       if (__MYPROCID == 0) 
		   fprintf( stdout, "All to All phased\n" );
	       }
           else {
	       f = AllToAll;
	       if (__MYPROCID == 0)
		   fprintf( stdout, "All to All (requires buffering)\n" );
	       }
            }
        else {
	    f = EachToAll;  
	    if (__MYPROCID == 0)
		fprintf( stdout, "Each to All\n" );
	    }
        break;
    }
first = svals[0];
last  = svals[1];
incr  = svals[2];

/* Disable resource checking */
;
MPI_Bcast(&endtime.tv_sec, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD );
err       = 0;
loopcount = 0;
bytes_sent= 0.0;
fflush( stdout );
do {
    for (pattern=0; pattern<=NPATTERNS; pattern++) {
        for (size=first; size<=last; size+=incr) {
	    if (__MYPROCID == 0) {
		fprintf( fp, "." ); fflush( fp );
                }
            if (__MYPROCID == 0 && BeVerbose) {
                fprintf( fp, "Running size = %d longs with pattern %x\n", 
			 size, (pattern < NPATTERNS) ? Patterns[pattern] : 
			 pattern );
                fflush( fp );
                }
            curerr = (*f)( pattern, size );
            err += curerr;
            if (curerr > 0) {
                fprintf( fp, 
		       "[%d] Error running size = %d longs with pattern %x\n", 
		       __MYPROCID, size, 
		       (pattern < NPATTERNS) ? Patterns[pattern] : pattern );
                fflush( fp );
		}
            }
	if (__MYPROCID == 0) {
	    fprintf( fp, "+\n" ); 
	    fflush( fp );
	    }
        }
    loopcount++;
    /* Make sure that everyone will do the same test */
    MPI_Allreduce(&err, &size, 1, MPI_INT,MPI_SUM,MPI_COMM_WORLD );
memcpy(&err,&size,(1)*sizeof(int));;
    SYGetDayTime( &currenttime );
    MPI_Bcast(&currenttime.tv_sec, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD );
    bytes_so_far = bytes_sent;
    MPI_Allreduce(&bytes_so_far, &dwork, 1, MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD );
memcpy(&bytes_so_far,&dwork,(1)*sizeof(double));;
    if (__MYPROCID == 0 && nextprint.tv_sec <= currenttime.tv_sec) {
	char   str[100];
	double rate;
	rate             = bytes_so_far / 
	                   (1.0e6 * (currenttime.tv_sec -  starttime.tv_sec) );
	nextprint.tv_sec = currenttime.tv_sec + STRESS_PRINT_INTERVAL;
	strcpy(str , "Not available" );
	fprintf( stdout, "stress runs to %s (%d) [%f MB/s aggregate]\n", 
		 str, loopcount, rate );
	  
	fflush( stdout );
	}
    } while (err == 0 && currenttime.tv_sec <= endtime.tv_sec );

if (__MYPROCID == 0) {
    fprintf( stdout, "Stress completed %d tests\n", loopcount );
    fprintf( stdout, "%e bytes sent\n", bytes_so_far );
    }
MPI_Finalize();
return 0;
}

int EachToAll( pattern, size )
int pattern;
int size;
{
int sender, dest, tag, from, err=0, bufmsize, actsize, bufsize;
char *buffer;

buffer = (char *)malloc((unsigned)(size * sizeof(long) ));  if (!buffer)return 0;;
bufsize = size * sizeof(long);
for (sender=0; sender < __NUMNODES; sender++) {
    tag = sender;
    if (__MYPROCID == sender) {
	for (dest=0; dest < __NUMNODES; dest++) {
	    if (sender != dest) {
		SetBuffer( buffer, size, pattern );
		MPI_Send(buffer,bufsize,MPI_BYTE,dest,tag,MPI_COMM_WORLD);
		bytes_sent += bufsize;
		}
	    }
	}
    else {
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, sender, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, pattern );
	}
    }
free(buffer );
return err;
}

/* This routine requires that the message passing system buffer significant
   amounts of data.  The routine AllToAllNB using nonblocking receives
 */
int AllToAll( pattern, size )
int pattern;
int size;
{
int sender, dest, tag, from, err=0, bufmsize, actsize, bufsize;
char *buffer;

buffer  = (char *)malloc((unsigned)(size * sizeof(long) ));  if (!buffer)return 0;;
bufsize = size * sizeof(long);
tag     = __MYPROCID;
for (dest=0; dest < __NUMNODES; dest++) {
    if (__MYPROCID != dest) {
	SetBuffer( buffer, size, pattern );
	MPI_Send(buffer,bufsize,MPI_BYTE,dest,tag,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	}
    }
for (sender=0; sender<__NUMNODES; sender++) {
    tag = sender;
    if (__MYPROCID != sender) {
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, sender, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, pattern );
	}
    }
free(buffer );
return err;
}

int AllToAllNB( pattern, size )
int pattern;
int size;
{
int sender, dest, tag, from, err=0, bufmsize, actsize, bufsize;
MPI_Request *rc;
char *buffer;

rc      = (MPI_Request *)malloc((unsigned)(__NUMNODES * sizeof(MPI_Request) ));
if (!rc)exit(1);;

buffer  = (char *)malloc((unsigned)(__NUMNODES * size * sizeof(long) ));  if (!buffer)return 0;;
bufsize = size * sizeof(long);
for (sender = 0; sender < __NUMNODES; sender++) {
    if (sender != __MYPROCID) {
	tag = sender;
	MPI_Irecv(buffer+sender*bufsize,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&(rc[sender] ));
	}
    }
tag     = __MYPROCID;
for (dest=0; dest < __NUMNODES; dest++) {
    if (__MYPROCID != dest) {
	SetBuffer( buffer, size, pattern );
	MPI_Send(buffer,bufsize,MPI_BYTE,dest,tag,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	}
    }
for (sender=0; sender<__NUMNODES; sender++) {
    tag = sender;
    if (__MYPROCID != sender) {
	bufmsize = bufsize;
	MPI_Wait(&(rc[sender] ),&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, sender, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, pattern );
	}
    }
free(buffer );
free(rc );
return err;
}

/* This version alternates sends and recieves depending on the mask value 
   
 */
int AllToAllPhased( pattern, size )
int pattern;
int size;
{
int  d, np, mytid, to, from, idx, err=0, bufmsize, bufsize;
char *buffer;

np      = __NUMNODES;
mytid   = __MYPROCID;
buffer  = (char *)malloc((unsigned)(size * sizeof(long) ));  if (!buffer)return 0;;
bufsize = size * sizeof(long);

for (d=1; d<=np/2; d++) {
    idx  = mytid / d;
    to   = (mytid + d) % np;
    from = (mytid + np - d) % np;
    if (idx & 0x1) {
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,from,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, from, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, 
			        pattern );
	SetBuffer( buffer, size, pattern );
	MPI_Send(buffer,bufsize,MPI_BYTE,to,mytid,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	MPI_Send(buffer,bufsize,MPI_BYTE,from,mytid,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,to,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, to, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, 
			        pattern );
	}
    else {
	SetBuffer( buffer, size, pattern );
	MPI_Send(buffer,bufsize,MPI_BYTE,to,mytid,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,from,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, from, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, 
			        pattern );
	bufmsize = bufsize;
	MPI_Recv(buffer,bufmsize,MPI_BYTE,MPI_ANY_SOURCE,to,MPI_COMM_WORLD,&_mpi_status);
	err += ErrTest( _mpi_status.MPI_SOURCE, to, (MPI_Get_count(_mpi_status,MPI_BYTE,&_MPILEN),_MPILEN), bufsize, buffer, 
			        pattern );
	SetBuffer( buffer, size, pattern );
	MPI_Send(buffer,bufsize,MPI_BYTE,from,mytid,MPI_COMM_WORLD);
	bytes_sent += bufsize;
	}
    }

free(buffer );
return err;
}

/*---------------------------------------------------------------------------
  These routines set and check the buffers by inserting the specified pattern
  and checking it.
 --------------------------------------------------------------------------- */
SetBuffer( buf, size, pattern )
long *buf;
int  size, pattern;
{
long val;
int  i;

if (pattern < NPATTERNS) {
    val = Patterns[pattern];
    for (i=0; i<size; i++) 
	buf[i] = val;
    }
else {
    for (i=0; i<size; i++) {
	buf[i] = Patterns[pattern % NPATTERNS];
	}
    }
}

int CheckBuffer( buf, size, pattern )
long *buf;
int  size, pattern;
{
long val;
int  i;

if (pattern < NPATTERNS) {
    val = Patterns[pattern];
    for (i=0; i<size; i++) 
	if (buf[i] != val) return 1;
    }
else {
    for (i=0; i<size; i++) {
	if (buf[i] != Patterns[pattern % NPATTERNS]) return 1;
	}
    }
return 0;
}


int ErrTest( from, partner, actsize, bufsize, buffer, pattern )
int  from, partner, actsize, bufsize, pattern;
char *buffer;
{
int err = 0;

if (from != partner) {
    fprintf( stderr, 
	    "Message from %d should be from %d\n", from, partner );
    err++;
    }
if (actsize != bufsize) {
    fprintf( stderr, "Message from %d is wrong size (%d != %d)\n", 
	     partner, actsize, bufsize );
    err++;
    }
if (CheckBuffer( buffer, actsize / sizeof(long), pattern )) {
    fprintf( stderr, "Message from %d is corrupt\n", partner );
    err++;
    }
return err;
}


PrintHelp( argv )
char **argv;
{
  fprintf( stderr, "%s - stress test communication\n", argv[0] );
  fprintf( stderr, 
"[-sync | -async  [-size start end stride]\n\
Stress communication links by various methods.  The tests are \n\
combinations of\n\
  Protocol: \n\
  -sync        Blocking sends/receives    (default)\n\
  -async       NonBlocking sends/receives\n\
  -all         AllToAll instead of EachToAll\n\
  -phased      Use ordered sends/receives for systems will little buffering\n\
\n" );
  fprintf( stderr, 
"  Message sizes:\n\
  -size start end stride                  (default 0 1024 32)\n\
               Messages of length (start + i*stride) for i=0,1,... until\n\
               the length is greater than end.\n\
\n\
  Number of tests\n\
  -ttime hh:mm Total time to run test (for AT LEAST this long)\n\
\n" );
}



