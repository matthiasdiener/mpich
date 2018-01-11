#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "bnr.h"

/* this pgm should be run with at least 2 processes */

int main( int argc, char *argv[] )
{
    int rc, my_bnr_rank, my_bnr_gid;
    char attr[100], val[100];

    rc = BNR_Init( &my_bnr_gid );
    printf("bnrtest: init: rc=%d my_bnr_gid=%d\n", rc, my_bnr_gid);  fflush(stdout);
    rc = BNR_Rank( my_bnr_gid, &my_bnr_rank );
    sprintf(attr,"attr%d",my_bnr_rank);
    sprintf(val,"%d",my_bnr_rank);
    rc = BNR_Put( 0, 1, attr, val );
    printf("bnrtest: put: rc=%d \n",rc);  fflush(stdout);
    strcpy(val,"     ");  /* just to make sure we get a new copy */
    BNR_Fence(my_bnr_gid);
    if (my_bnr_rank == 0)
	sprintf(attr,"attr%d",1);
    else
	sprintf(attr,"attr%d",0);
    rc = BNR_Get( 0, attr, val );
    printf("bnrtest %d: get: rc=%d val=%s\n", my_bnr_rank,rc,val);  fflush(stdout);
    return 0;
}
