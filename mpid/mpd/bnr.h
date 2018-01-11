#ifndef BNR_INCLUDE
#define BNR_INCLUDE

typedef int BNR_gid;
#define BNR_MAXATTRLEN 64
#define BNR_MAXVALLEN  1024

int BNR_Init( BNR_gid * );
int BNR_Fence( BNR_gid );
int BNR_Put( BNR_gid, int, char *, char * );
int BNR_Get( BNR_gid, char *, char * );
int BNR_Rank( BNR_gid, int * );
int BNR_Size( BNR_gid, int * );
/* compat for mpich-1 */
int BNR_Pre_init( void (*)(char *) );
int BNR_Man_msgs_fd( int *);
int BNR_Poke_peer( BNR_gid, int, char * );

#endif
