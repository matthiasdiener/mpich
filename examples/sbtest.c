#include "tools.h"
#include "system/system.h"
#include "system/sbcnst.h"

/* 

   Program to test the fixed-block allocator 

 */
main( argc, argv )
int  argc;
char **argv;
{
void *sb;
void *(p[35]);
int  i, space, fr;

sb = SBinit( 4096, 25, 10 );
if (!sb) return 10;

for (i=0; i<35; i++) {
    p[i] = SBalloc( sb );
    }

for (i=34; i>=0; i--) 
    SBfree( sb, p[i] );

TRSPACE( &space, &fr );
printf( "Completed sbtest (free): %d bytes remaining and %d fragments\n", 
        space, fr );
SBdestroy( sb );
TRSPACE( &space, &fr );
printf( "Completed sbtest (destroy): %d bytes remaining and %d fragments\n", 
        space, fr );

sb = SBinit( 4096, 25, 10 );
if (!sb) return 10;

for (i=0; i<35; i++) {
    p[i] = SBalloc( sb );
    }

for (i=34; i>=0; i--) 
    SBrelease( sb, p[i] );

TRSPACE( &space, &fr );
printf( "Completed sbtest (release): %d bytes remaining and %d fragments\n", 
        space, fr );
SBFlush( sb );
TRSPACE( &space, &fr );
printf( "Completed sbtest (flush): %d bytes remaining and %d fragments\n", 
        space, fr );
SBdestroy( sb );
TRSPACE( &space, &fr );
printf( 
     "Completed sbtest (final destroy): %d bytes remaining and %d fragments\n",
     space, fr );

return 0;
}
