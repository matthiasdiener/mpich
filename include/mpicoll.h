/*
 * Collective operations (this allows choosing either an implementation
 * in terms of point-to-point, or a special version exploiting special
 * facilities, in a communicator by communicator fashion).
 */
#ifndef MPIR_COLLOPS_DEF
#define MPIR_COLLOPS_DEF
struct _MPIR_COLLOPS {
    int (*Barrier) ANSI_ARGS((MPI_Comm));
    int (*Bcast) ANSI_ARGS((void*, int, MPI_Datatype, int, MPI_Comm ));
    int (*Gather) ANSI_ARGS((void*, int, MPI_Datatype, 
		  void*, int, MPI_Datatype, int, MPI_Comm)); 
    int (*Gatherv) ANSI_ARGS((void*, int, MPI_Datatype, 
		   void*, int *, int *, MPI_Datatype, int, MPI_Comm)); 
    int (*Scatter) ANSI_ARGS((void*, int, MPI_Datatype, 
		   void*, int, MPI_Datatype, int, MPI_Comm));
    int (*Scatterv) ANSI_ARGS((void*, int *, int *, MPI_Datatype, void*, int, 
		    MPI_Datatype, int, MPI_Comm));
    int (*Allgather) ANSI_ARGS((void*, int, MPI_Datatype, 
		     void*, int, MPI_Datatype, MPI_Comm));
    int (*Allgatherv) ANSI_ARGS((void*, int, MPI_Datatype, 
		      void*, int *, int *, MPI_Datatype, MPI_Comm));
    int (*Alltoall) ANSI_ARGS((void*, int, MPI_Datatype, 
		    void*, int, MPI_Datatype, MPI_Comm));
    int (*Alltoallv) ANSI_ARGS((void*, int *, int *, 
		     MPI_Datatype, void*, int *, 
		     int *, MPI_Datatype, MPI_Comm));
    int (*Reduce) ANSI_ARGS((void*, void*, int, 
		  MPI_Datatype, MPI_Op, int, MPI_Comm));
    int (*Allreduce) ANSI_ARGS((void*, void*, int, 
		     MPI_Datatype, MPI_Op, MPI_Comm));
    int (*Reduce_scatter) ANSI_ARGS((void*, void*, int *, 
			  MPI_Datatype, MPI_Op, MPI_Comm));
    int (*Scan) ANSI_ARGS((void*, void*, int, MPI_Datatype, MPI_Op, 
			   MPI_Comm ));
    int ref_count;     /* So we can share it */
};

/* Predefined function tables for collective routines, the device
 * can also use its own, but these are the defaults.
 */
extern MPIR_COLLOPS MPIR_inter_collops;   /* Simply raises appropriate error */
extern MPIR_COLLOPS MPIR_intra_collops;   /* Do the business using pt2pt     */


#endif
