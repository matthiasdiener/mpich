
#define CACHELINESIZE 64  /*must be minimum of 32 bytes */

typedef struct {
  int *flag;
  int *ival;
  void **addr;
  double *dval;
  float *rval;
} MPID_Barf;

typedef struct {
  void *barrier;
  int np,mypid,nc,myhypernode;
  MPID_Barf *barf; /* flag array */
} MPID_Fastbar;

int MPID_SHMEM_Wait_barrier();
void *MPID_SHMEM_Init_barrier();
void MPID_SHMEM_Free_barrier();
void MPID_SHMEM_Free_collect();
int MPID_SHMEM_First_barrier();
int MPID_SHMEM_Reduce_scatter(); 
int MPID_SHMEM_Reduce_scatterv(); 
int MPID_SHMEM_Allgatherv();
int MPID_SHMEM_Allgather();
int MPID_SHMEM_Allreduce();
int MPID_SHMEM_Reduce();
int MPID_SHMEM_Bcast();

typedef struct {
    int bsize;
    int *blockcounts, *displs;
} MPID_Fastcoll;
