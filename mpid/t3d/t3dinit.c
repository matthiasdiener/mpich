#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "mpid.h"
#include "packets.h"
#include "t3dpriv.h"

volatile T3D_PKT_T          *t3d_recv_bufs;
volatile T3D_Buf_Status     *t3d_dest_flags;
volatile T3D_Long_Send_Info *t3d_lsi;
volatile T3D_Long_Send_Info *blocking_lsi;
int                          t3d_lsi_window_head = 0;
int                          t3d_lsi_window_tail = 0;
int                          t3d_num_lsi = T3D_DEFAULT_NUM_ENVELOPES;

extern  int MPID_MyWorldRank;
extern  int MPID_MyWorldSize;
double  t3d_reference_time;
char   *t3d_heap_limit;

void T3D_Init(argc,argv)
int argc;
char *argv[];
{
  int i,j,numprocs;
  int size;
  T3D_PKT_T *pkt;
  
  MPID_MyWorldRank       = _my_pe();
  MPID_MyWorldSize       = _num_pes();
  t3d_reference_time = (double)rtclock();
  t3d_heap_limit     = sbrk(0);
  
  numprocs = MPID_MyWorldSize;
  for (i=1; i < argc; i++)
  {
    if (strcmp(argv[i],"-np") == 0)
    {
      if ( (i+1) == argc )
      {
	fprintf(stderr,
		"Missing argument to -np for number of processes\n");
	exit(1);
      }
      numprocs = atoi(argv[i+1]);
      argv[i] = NULL;
      argv[i+1] = NULL;
      i++;
      continue;
    }
    else if (strcmp(argv[i],"-envelope") == 0)
    {
      if ( (i+1) == argc )
      {
	fprintf(stderr,
		"Missing argument to -envelope for number of envelopes\n");
	exit(1);
      }
      t3d_num_lsi = atoi(argv[i+1]);
      if (1 > t3d_num_lsi)
	t3d_num_lsi = T3D_DEFAULT_NUM_ENVELOPES;
      argv[i] = NULL;
      argv[i+1] = NULL;
      i++;
      continue;
    }
  }
  if ((numprocs <= 0) || (numprocs > MPID_MyWorldSize))
  {
    fprintf(stderr,
	    "Invalid number of processes (%d)\n",numprocs);
    exit(1);
  }
  MPID_MyWorldSize = numprocs;

  shmem_set_cache_inv();

  size = sizeof(T3D_PKT_T) * MPID_MyWorldSize;
  t3d_recv_bufs = (T3D_PKT_T*)shmalloc(size);
  if (t3d_recv_bufs == NULL)
  {
    fprintf(stderr,
	    "[%d] Unable to allocate shared memory recv packets!\n",MPID_MyWorldRank);
    fflush(stderr);
    globalexit(1);
    abort();
  }

  size = sizeof(T3D_Buf_Status) * MPID_MyWorldSize;
  t3d_dest_flags = (T3D_Buf_Status*)shmalloc(size);
  if (t3d_recv_bufs == NULL)
  {
    fprintf(stderr,
	    "[%d] Unable to allocate shared memory flags!\n",MPID_MyWorldRank);
    fflush(stderr);
    globalexit(1);
    abort();
  }

  size = sizeof(T3D_Long_Send_Info) * t3d_num_lsi;
  t3d_lsi = (T3D_Long_Send_Info*)shmalloc(size);
  if (t3d_lsi == NULL)
  {
    fprintf(stderr,
	    "[%d] Unable to allocate shared memory envelopes!\n",MPID_MyWorldRank);
    fflush(stderr);
    globalexit(1);
    abort();
  }
  memset(t3d_lsi,0,size);

  size = sizeof(T3D_Long_Send_Info);
  blocking_lsi = (T3D_Long_Send_Info*)shmalloc(size);
  if (blocking_lsi == NULL)
  {
    fprintf(stderr,
	    "[%d] Unable to allocate shared memory blocking envelope!\n",MPID_MyWorldRank);
    fflush(stderr);
    globalexit(1);
    abort();    
  }
  memset(blocking_lsi,0,size);

  for (i=0; i < MPID_MyWorldSize; i++)
  {
    t3d_dest_flags[i] = T3D_BUF_AVAIL;
    t3d_recv_bufs[i].head.status = T3D_BUF_AVAIL;
  }

  barrier();

  if (MPID_MyWorldRank >= MPID_MyWorldSize)
  {
    shfree( (void*) t3d_recv_bufs );
    shfree( (void*) t3d_dest_flags );
    shfree( (void*) t3d_lsi );
    shfree( (void*) blocking_lsi );
    shmem_clear_cache_inv();

    exit(0);
  }
}

/***************************************************************************
  Node_name
 ***************************************************************************/
void MPID_Node_name( name, len )
char *name;
int   len;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag)
      DEBUG_PRINT_MSG("T3D_Node_name\n");
#   endif

    /* Get the host name */
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT)
    if (gethostname(name, len) != 0) {
        T3D_Error(T3D_ERR_UNKNOWN, "unable to get host name\n");
        name[0] = '\0';
        return;
    }
#   else
    (void)gethostname(name, len);
#   endif
}
