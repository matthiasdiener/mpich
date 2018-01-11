
/*------------------------------ mpd implementation of BNR interface ------------*/

#include "bnr.h"
#include "mpdlib.h"
#include "mpd.h"

int man_msgs_fd;
extern void (*MPD_user_peer_msg_handler)(char *);

int BNR_Init( BNR_gid *mygid )
/* returns primary group id assigned at creation */
{
    MPD_Init( MPD_user_peer_msg_handler );
    *mygid = 0;			/* default for now */
    man_msgs_fd = MPD_Man_msgs_fd( );

    return(0);
}

int BNR_Fence( BNR_gid gid )
/* barriers all processes in group; puts done before 
   the fence are accessible by gets after the fence */
{
    int i;
    char buf[MAXLINE];
    int grank, gsize;
    
    BNR_Rank( gid, &grank );
    BNR_Size( gid, &gsize );
    sprintf( buf, "cmd=client_bnr_fence_in gid=%d grank=%d gsize=%d\n", gid, grank, gsize );
    write( man_msgs_fd, buf, strlen(buf) );
    i = read_line( man_msgs_fd, buf, MAXLINE );  
    mpdprintf( 0, "MPDLIB rc=%d fence_out msg=>:%s:\n", i, buf );
    parse_keyvals( buf );
    getval( "cmd", buf );
    if ( strcmp( "client_bnr_fence_out", buf ) != 0 ) {
        mpdprintf( 1, "expecting client_bnr_fence_out; got :%s:\n", buf );
        return(-1);
    }
    return(0);
}

int BNR_Put( BNR_gid group, int loc, char *attr, char *val )
/* puts attr-value pair for retrieval by other processes in group;
   attr is a string of length < BNR_MAXATTRLEN;
   val is string of length < BNR_MAXVALLEN
   loc is an advisory-only suggested location */
{
    char buf[MAXLINE];

    sprintf( buf, "cmd=client_bnr_put gid=%d attr=%s val=%s loc=%d\n", group, attr, val, loc );
    write( man_msgs_fd, buf, strlen(buf) );
    return(0);
}

int BNR_Get( BNR_gid group, char *attr, char *val )
/* matches attr, retrieves corresponding value into val,
   which is a buffer of length = BNR_MAXVALLEN */
{
    int i;
    char buf[MAXLINE];

    sprintf( buf, "cmd=client_bnr_get gid=%d attr=%s\n", group, attr );
    write( man_msgs_fd, buf, strlen(buf) );
    i = read_line( man_msgs_fd, buf, MAXLINE );  
    mpdprintf( 0, "MPDLIB rc=%d bnr_get msg=>:%s:\n", i, buf );
    parse_keyvals( buf );
    getval( "cmd", buf );
    if ( strcmp( "client_bnr_get_output", buf ) == 0 ) {
        if ( ! getval( "val", val ) )
            return(-1);
    }
    else if ( strcmp( "client_bnr_get_failed", buf ) == 0 ) {
        mpdprintf( 1, "client_bnr_get failed\n", buf );
        return(-1);  /* not found */
    }
    else {
        mpdprintf( 1, "expecting client_bnr_get_output; got :%s:\n", buf );
        return(-1);  /* not found */
    }
    return(0);
}

int BNR_Rank( BNR_gid group, int *myrank )
/* returns rank in group */
{
    *myrank = MPD_Rank( );	/* ignores group for now */
    return 0;
}

int BNR_Size( BNR_gid group, int *groupsize )
/* returns size of group */
{
    *groupsize = MPD_Size( );	/* ignores group for now */
    return 0;
}

/* The following parts of BNR are not yet implemented */

#if 0


int BNR_allocate_gid( BNR_gid *new_gid )
/* allocates a new, unique group id */
{
}


int BNR_Spawn( BNR_gid group, int root, BNR_gid remote_group, char *command,
// 	       char *argv[], char *env[], MPI_Info info, int array_of_errcodes[],
	       int (notify_fn)(BNR_gid group, int rank, int exit_code) )
/* collective over group, arguments valid only at root.
   Note we pass *in* the new group id, assumed to have
   been BNR_allocated.  notify_fn is called if a process
   exits, and gets the group, rank, and return code.
   argv and env arrays are null terminated*/
{
}

int BNR_merge( BNR_gid local_group, BNR_gid remote_group, BNR_gid *new_group )
/* calling process must be in the local group and must not be in
   the remote group.  Collective over the union of the two groups. */
{
}

int BNR_Spawn_multiple( BNR_gid group, int root, BNR_gid remote_group, int count,
			char *array_of_commands[], char *array_of_argv[],
// 			char *array_of_env[], MPI_Info array_of_info[],
			int array_of_errcodes[],
			int (notify_fn)(BNR_gid group, int rank, int exit_code) )
/* like BNR_Spawn, with arrays of length count */
{
}

int BNR_Parent( BNR_gid *mygid, BNR_gid *parent_gid )
/* returns 1 if no parent at all; 2 if parent is mpiexec */
{
}

int BNR_Free( BNR_gid gid )
/* frees gid for re-use. */
{
}

#endif


/* ------------------------- backward compatibility for mpich-1 -----------------*/

int BNR_Pre_init( void (*peer_msg_handler)(char *) )
{
    MPD_user_peer_msg_handler = peer_msg_handler;
    return( 0 );
}

int BNR_Man_msgs_fd( int *fd )
{
    *fd = MPD_Man_msgs_fd();
    return( 0 );
}

int BNR_Poke_peer( BNR_gid gid, int dest_rank, char *msg )
{
    MPD_Poke_peer( gid, dest_rank, msg );
    return( 0 );
}
