#include <stdio.h>
#include <unistd.h>
#include "bnr.h"

int main( int argc, char *argv[] )
{
    int i, rc, my_bnr_gid, my_bnr_group_size, my_bnr_rank;
    char attr[100], val[100];

    rc = BNR_Init( &my_bnr_gid );
    rc = BNR_Rank( my_bnr_gid, &my_bnr_rank );
    rc = BNR_Size( my_bnr_gid, &my_bnr_group_size );

    sprintf( attr, "rank_%d", my_bnr_rank );
    sprintf( val, "%d", getpid() );
    rc = BNR_Put( my_bnr_gid, -1, attr, val );

    rc = BNR_Fence( my_bnr_gid );
    
    for ( i=0; i < my_bnr_group_size; i++ ) {
	sprintf( attr, "rank_%d", i );
	rc = BNR_Get( my_bnr_gid, attr, val ); 
	printf( "bnrtest %d: rank=%s pid=%s\n", my_bnr_rank, attr, val );  fflush( stdout );
    }

    rc = BNR_Get( 0, "SHMEMKEY", val );
    printf( "bnrtest %d: SHMEMKEY=%s\n", my_bnr_rank, val );  fflush( stdout );
    return( 0 );
}

