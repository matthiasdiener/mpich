
/*
 * CVS Id: $Id: mem.c,v 1.2 2002/12/10 14:24:16 karonis Exp $
 */


#include "comm.h"   /* for struct MPIR_COMMUNICATOR */
#include "mpi.h"   /* for MPI_Comm_rank/size, MPI_COMM_WORLD */
#include <globus_common.h>   /* for globus_libc_malloc, size_t */


/**********************************************************************/
/* allocate memory and check the return pointer.  MPID_Abort if NULL */

void *
g_malloc_chk_internal (const size_t size, const char * file, const int line)
{
   void *ptr;
  
   if ( size == 0 ) return NULL;
  
   ptr = globus_libc_malloc(size);

   if ( ptr == NULL )
   {
      int world_rank, world_size;

      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      globus_libc_fprintf(stderr, "[%d/%d:%s:%d] failed malloc %d bytes\n",
                          world_rank, world_size, file, line, size);
      MPID_Abort((struct MPIR_COMMUNICATOR*) 0, 2, "MPICH-G2 Internal",
                 "failed malloc");
   }

   return ptr;
}
