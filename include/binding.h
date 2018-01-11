#ifndef __MPI_BINDINGS
#define __MPI_BINDINGS

#include "mpi.h"

/* We require that the C compiler support prototypes */
int MPI_Send(void*, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Recv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int MPI_Get_count(MPI_Status *, MPI_Datatype, int *);
int MPI_Bsend(void*, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Ssend(void*, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Rsend(void*, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Buffer_attach( void*, int);
int MPI_Buffer_detach( void*, int*);
int MPI_Isend(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Ibsend(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Issend(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Irsend(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Irecv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Wait(MPI_Request *, MPI_Status *);
int MPI_Test(MPI_Request *, int *, MPI_Status *);
int MPI_Request_free(MPI_Request *);
int MPI_Waitany(int, MPI_Request *, int *, MPI_Status *);
int MPI_Testany(int, MPI_Request *, int *, int *, MPI_Status *);
int MPI_Waitall(int, MPI_Request *, MPI_Status *);
int MPI_Testall(int, MPI_Request *, int *, MPI_Status *);
int MPI_Waitsome(int, MPI_Request *, int *, int *, MPI_Status *);
int MPI_Testsome(int, MPI_Request *, int *, int *, MPI_Status *);
int MPI_Iprobe(int, int, MPI_Comm, int *flag, MPI_Status *);
int MPI_Probe(int, int, MPI_Comm, MPI_Status *);
int MPI_Cancel(MPI_Request *);
int MPI_Test_cancelled(MPI_Status *, int *flag);
int MPI_Send_init(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Bsend_init(void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int MPI_Ssend_init(void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int MPI_Rsend_init(void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int MPI_Recv_init(void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int MPI_Start(MPI_Request *);
int MPI_Startall(int, MPI_Request *);
int MPI_Sendrecv(void *, int, MPI_Datatype,int, int, void *, int, 
		 MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int MPI_Sendrecv_replace(void*, int, MPI_Datatype, 
			 int, int, int, int, MPI_Comm, MPI_Status *);
int MPI_Type_contiguous(int, MPI_Datatype, MPI_Datatype *);
int MPI_Type_vector(int, int, int, MPI_Datatype, MPI_Datatype *);
int MPI_Type_hvector(int, int, MPI_Aint, MPI_Datatype, MPI_Datatype *);
int MPI_Type_indexed(int, int *, int *, MPI_Datatype, MPI_Datatype *);
int MPI_Type_hindexed(int, int *, MPI_Aint *, MPI_Datatype, MPI_Datatype *);
int MPI_Type_struct(int, int *, MPI_Aint *, MPI_Datatype *, MPI_Datatype *);
int MPI_Address(void*, MPI_Aint *);
int MPI_Type_extent(MPI_Datatype, MPI_Aint *);

/* See the 1.1 version of the Standard; I think that the standard is in 
   error; however, it is the standard */
/* int MPI_Type_size(MPI_Datatype, MPI_Aint *size); */
int MPI_Type_size(MPI_Datatype, int *);
int MPI_Type_count(MPI_Datatype, int *);
int MPI_Type_lb(MPI_Datatype, MPI_Aint*);
int MPI_Type_ub(MPI_Datatype, MPI_Aint*);
int MPI_Type_commit(MPI_Datatype *);
int MPI_Type_free(MPI_Datatype *);
int MPI_Get_elements(MPI_Status *, MPI_Datatype, int *);
int MPI_Pack(void*, int, MPI_Datatype, void *, int, int *,  MPI_Comm);
int MPI_Unpack(void*, int, int *, void *, int, MPI_Datatype, MPI_Comm);
int MPI_Pack_size(int, MPI_Datatype, MPI_Comm, int *);
int MPI_Barrier(MPI_Comm );
int MPI_Bcast(void*, int, MPI_Datatype, int, MPI_Comm );
int MPI_Gather(void* , int, MPI_Datatype, void*, int, MPI_Datatype, 
	       int, MPI_Comm); 
int MPI_Gatherv(void* , int, MPI_Datatype, void*, int *, int *, 
		MPI_Datatype, int, MPI_Comm); 
int MPI_Scatter(void* , int, MPI_Datatype, void*, int, MPI_Datatype, 
		int, MPI_Comm);
int MPI_Scatterv(void* , int *, int *,  MPI_Datatype, void*, int, 
		 MPI_Datatype, int, MPI_Comm);
int MPI_Allgather(void* , int, MPI_Datatype, void*, int, MPI_Datatype, 
		  MPI_Comm);
int MPI_Allgatherv(void* , int, MPI_Datatype, void*, int *, int *, 
		   MPI_Datatype, MPI_Comm);
int MPI_Alltoall(void* , int, MPI_Datatype, void*, int, MPI_Datatype, 
		 MPI_Comm);
int MPI_Alltoallv(void* , int *, int *, MPI_Datatype, void*, int *, 
		  int *, MPI_Datatype, MPI_Comm);
int MPI_Reduce(void* , void*, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int MPI_Op_create(MPI_User_function *, int, MPI_Op *);
int MPI_Op_free( MPI_Op *);
int MPI_Allreduce(void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm);
int MPI_Reduce_scatter(void* , void*, int *, MPI_Datatype, MPI_Op, MPI_Comm);
int MPI_Scan(void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm );
int MPI_Group_size(MPI_Group group, int *);
int MPI_Group_rank(MPI_Group group, int *);
int MPI_Group_translate_ranks (MPI_Group, int n, int *, MPI_Group, int *);
int MPI_Group_compare(MPI_Group, MPI_Group, int *);
int MPI_Comm_group(MPI_Comm, MPI_Group *);
int MPI_Group_union(MPI_Group, MPI_Group, MPI_Group *);
int MPI_Group_intersection(MPI_Group, MPI_Group, MPI_Group *);
int MPI_Group_difference(MPI_Group, MPI_Group, MPI_Group *);
int MPI_Group_incl(MPI_Group group, int n, int *, MPI_Group *);
int MPI_Group_excl(MPI_Group group, int n, int *, MPI_Group *);
int MPI_Group_range_incl(MPI_Group group, int n, int [][3], MPI_Group *);
int MPI_Group_range_excl(MPI_Group group, int n, int [][3], MPI_Group *);
int MPI_Group_free(MPI_Group *);
int MPI_Comm_size(MPI_Comm, int *);
int MPI_Comm_rank(MPI_Comm, int *);
int MPI_Comm_compare(MPI_Comm, MPI_Comm, int *);
int MPI_Comm_dup(MPI_Comm, MPI_Comm *);
int MPI_Comm_create(MPI_Comm, MPI_Group, MPI_Comm *);
int MPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
int MPI_Comm_free(MPI_Comm *);
int MPI_Comm_test_inter(MPI_Comm, int *);
int MPI_Comm_remote_size(MPI_Comm, int *);
int MPI_Comm_remote_group(MPI_Comm, MPI_Group *);
int MPI_Intercomm_create(MPI_Comm, int, MPI_Comm, int, int, MPI_Comm * );
int MPI_Intercomm_merge(MPI_Comm, int, MPI_Comm *);
int MPI_Keyval_create(MPI_Copy_function *, MPI_Delete_function *, 
		      int *, void*);
int MPI_Keyval_free(int *);
int MPI_Attr_put(MPI_Comm, int, void*);
int MPI_Attr_get(MPI_Comm, int, void *, int *);
int MPI_Attr_delete(MPI_Comm, int);
int MPI_Topo_test(MPI_Comm, int *);
int MPI_Cart_create(MPI_Comm, int, int *, int *, int, MPI_Comm *);
int MPI_Dims_create(int, int, int *);
int MPI_Graph_create(MPI_Comm, int, int *, int *, int, MPI_Comm *);
int MPI_Graphdims_get(MPI_Comm, int *, int *);
int MPI_Graph_get(MPI_Comm, int, int, int *, int *);
int MPI_Cartdim_get(MPI_Comm, int *);
int MPI_Cart_get(MPI_Comm, int, int *dims, int *, int *);
int MPI_Cart_rank(MPI_Comm, int *coords, int *rank);
int MPI_Cart_coords(MPI_Comm, int, int, int *);
int MPI_Graph_neighbors_count(MPI_Comm, int, int *);
int MPI_Graph_neighbors(MPI_Comm, int, int, int *);
int MPI_Cart_shift(MPI_Comm, int, int, int *, int *);
int MPI_Cart_sub(MPI_Comm, int *, MPI_Comm *);
int MPI_Cart_map(MPI_Comm, int, int *, int *, int *);
int MPI_Graph_map(MPI_Comm, int, int *, int *, int *);
int MPI_Get_processor_name(char *, int *);
int MPI_Get_version(int *, int *);
int MPI_Errhandler_create(MPI_Handler_function *, 
			  MPI_Errhandler *);
int MPI_Errhandler_set(MPI_Comm, MPI_Errhandler);
int MPI_Errhandler_get(MPI_Comm, MPI_Errhandler *);
int MPI_Errhandler_free(MPI_Errhandler *);
int MPI_Error_string(int, char *, int *);
int MPI_Error_class(int, int *);
double MPI_Wtime(void);
double MPI_Wtick(void);
#ifndef MPI_Wtime
double PMPI_Wtime(void);
double PMPI_Wtick(void);
#endif
int MPI_Init(int *, char ***);
int MPI_Finalize(void);
int MPI_Initialized(int *);
int MPI_Abort(MPI_Comm, int);

/* MPI-2 communicator naming functions */
int MPI_Comm_set_name(MPI_Comm, char *);
int MPI_Comm_get_name(MPI_Comm, char *, int *);

#ifdef HAVE_NO_C_CONST
/* Default Solaris compiler does not accept const but does accept prototypes */
int MPI_Pcontrol(int, ...);
#else
int MPI_Pcontrol(const int, ...);
#endif

int MPI_NULL_COPY_FN ( MPI_Comm, int, void *, void *, void *, int * );
int MPI_NULL_DELETE_FN ( MPI_Comm, int, void *, void * );
int MPI_DUP_FN ( MPI_Comm, int, void *, void *, void *, int * );

/* misc2 (MPI2) */
int MPI_Status_f2c( MPI_Fint *, MPI_Status * );
int MPI_Status_c2f( MPI_Status *, MPI_Fint * );
int MPI_Finalized( int * );
int MPI_Type_create_indexed_block(int, int, int *, MPI_Datatype, 
				  MPI_Datatype *);
int MPI_Type_get_envelope(MPI_Datatype, int *, int *, int *, int *); 
int MPI_Type_get_contents(MPI_Datatype, int, int, int, int *, 
             MPI_Aint *, MPI_Datatype *);

int MPI_Type_create_subarray(int, int *, int *, int *, int, 
                      MPI_Datatype, MPI_Datatype *);

int MPI_Type_create_darray(int, int, int, int *, int *, int *, int *, 
                    int, MPI_Datatype, MPI_Datatype *);

int MPI_Info_create(MPI_Info *);
int MPI_Info_set(MPI_Info, char *key, char *value);
int MPI_Info_delete(MPI_Info, char *key);
int MPI_Info_get(MPI_Info, char *key, int valuelen, 
                         char *value, int *flag);
int MPI_Info_get_valuelen(MPI_Info, char *key, int *valuelen, 
                                  int *flag);
int MPI_Info_get_nkeys(MPI_Info, int *nkeys);
int MPI_Info_get_nthkey(MPI_Info, int n, char *key);
int MPI_Info_dup(MPI_Info, MPI_Info *newinfo);
int MPI_Info_free(MPI_Info *info);

MPI_Fint MPI_Info_c2f(MPI_Info);
MPI_Info MPI_Info_f2c(MPI_Fint);

MPI_Fint MPI_Request_c2f( MPI_Request );




/* Here are the bindings of the profiling routines */
#if !defined(MPI_BUILD_PROFILING)
int PMPI_Send(void*, int, MPI_Datatype, int, int, 
	     MPI_Comm);
int PMPI_Recv(void*, int, MPI_Datatype, int, 
	     int, MPI_Comm, MPI_Status *);
int PMPI_Get_count(MPI_Status *, MPI_Datatype, int *);
int PMPI_Bsend(void*, int, MPI_Datatype, int, int, 
	      MPI_Comm);
int PMPI_Ssend(void*, int, MPI_Datatype, int, int, 
	      MPI_Comm);
int PMPI_Rsend(void*, int, MPI_Datatype, int, int, 
	      MPI_Comm);
int PMPI_Buffer_attach( void* buffer, int size);
int PMPI_Buffer_detach( void* buffer, int* size);
int PMPI_Isend(void*, int, MPI_Datatype, int, int, 
	      MPI_Comm, MPI_Request *);
int PMPI_Ibsend(void*, int, MPI_Datatype, int, 
	       int, MPI_Comm, MPI_Request *);
int PMPI_Issend(void*, int, MPI_Datatype, int, 
	       int, MPI_Comm, MPI_Request *);
int PMPI_Irsend(void*, int, MPI_Datatype, int, 
	       int, MPI_Comm, MPI_Request *);
int PMPI_Irecv(void*, int, MPI_Datatype, int, 
	      int, MPI_Comm, MPI_Request *);
int PMPI_Wait(MPI_Request *, MPI_Status *);
int PMPI_Test(MPI_Request *, int *flag, MPI_Status *);
int PMPI_Request_free(MPI_Request *);
int PMPI_Waitany(int, MPI_Request *, int *, MPI_Status *);
int PMPI_Testany(int, MPI_Request *, int *, int *, MPI_Status *);
int PMPI_Waitall(int, MPI_Request *, 
		MPI_Status *);
int PMPI_Testall(int, MPI_Request *, int *flag, 
		MPI_Status *);
int PMPI_Waitsome(int, MPI_Request *, int *, 
		 int *, MPI_Status *);
int PMPI_Testsome(int, MPI_Request *, int *, 
		 int *, MPI_Status *);
int PMPI_Iprobe(int, int, MPI_Comm, int *flag, 
	       MPI_Status *);
int PMPI_Probe(int, int, MPI_Comm, MPI_Status *);
int PMPI_Cancel(MPI_Request *);
int PMPI_Test_cancelled(MPI_Status *, int *flag);
int PMPI_Send_init(void*, int, MPI_Datatype, int, 
		  int, MPI_Comm, MPI_Request *);
int PMPI_Bsend_init(void*, int, MPI_Datatype, int, 
		   int, MPI_Comm, MPI_Request *);
int PMPI_Ssend_init(void*, int, MPI_Datatype, int, 
		   int, MPI_Comm, MPI_Request *);
int PMPI_Rsend_init(void*, int, MPI_Datatype, int, 
		   int, MPI_Comm, MPI_Request *);
int PMPI_Recv_init(void*, int, MPI_Datatype, int, 
		  int, MPI_Comm, MPI_Request *);
int PMPI_Start(MPI_Request *);
int PMPI_Startall(int, MPI_Request *);
int PMPI_Sendrecv(void *, int, MPI_Datatype, 
		 int, int, void *, int, 
		 MPI_Datatype, int, int, 
		 MPI_Comm, MPI_Status *);
int PMPI_Sendrecv_replace(void*, int, MPI_Datatype, 
			 int, int, int, int, 
			 MPI_Comm, MPI_Status *);
int PMPI_Type_contiguous(int, MPI_Datatype, 
			MPI_Datatype *);
int PMPI_Type_vector(int, int, int, 
		    MPI_Datatype, MPI_Datatype *);
int PMPI_Type_hvector(int, int, MPI_Aint, 
		     MPI_Datatype, MPI_Datatype *);
int PMPI_Type_indexed(int, int *, 
		     int *, MPI_Datatype, 
		     MPI_Datatype *);
int PMPI_Type_hindexed(int, int *, 
		      MPI_Aint *, MPI_Datatype, 
		      MPI_Datatype *);
int PMPI_Type_struct(int, int *, 
		    MPI_Aint *, 
		    MPI_Datatype *, MPI_Datatype *);
int PMPI_Address(void*, MPI_Aint *);
int PMPI_Type_extent(MPI_Datatype, MPI_Aint *);

/* See the 1.1 version of the Standard; I think that the standard is in 
   error; however, it is the standard */
/* int PMPI_Type_size(MPI_Datatype, MPI_Aint *); */
int PMPI_Type_size(MPI_Datatype, int *);
int PMPI_Type_count(MPI_Datatype, int *);
int PMPI_Type_lb(MPI_Datatype, MPI_Aint*);
int PMPI_Type_ub(MPI_Datatype, MPI_Aint*);
int PMPI_Type_commit(MPI_Datatype *);
int PMPI_Type_free(MPI_Datatype *);
int PMPI_Get_elements(MPI_Status *, MPI_Datatype, int *);
int PMPI_Pack(void* inbuf, int, MPI_Datatype, void *outbuf, 
	     int outsize, int *position,  MPI_Comm);
int PMPI_Unpack(void*, int, int *, void *, 
	       int, MPI_Datatype, MPI_Comm);
int PMPI_Pack_size(int, MPI_Datatype, MPI_Comm, 
		  int *);
int PMPI_Barrier(MPI_Comm );
int PMPI_Bcast(void* buffer, int, MPI_Datatype, int, 
	      MPI_Comm );
int PMPI_Gather(void* , int, MPI_Datatype, 
	       void*, int, MPI_Datatype, 
	       int, MPI_Comm); 
int PMPI_Gatherv(void* , int, MPI_Datatype, 
		void*, int *, int *displs, 
		MPI_Datatype, int, MPI_Comm); 
int PMPI_Scatter(void* , int, MPI_Datatype, 
		void*, int, MPI_Datatype, 
		int, MPI_Comm);
int PMPI_Scatterv(void* , int *, int *displs, 
		 MPI_Datatype, void*, int, 
		 MPI_Datatype, int, MPI_Comm);
int PMPI_Allgather(void* , int, MPI_Datatype, 
		  void*, int, MPI_Datatype, 
		  MPI_Comm);
int PMPI_Allgatherv(void* , int, MPI_Datatype, 
		   void*, int *, int *displs, 
		   MPI_Datatype, MPI_Comm);
int PMPI_Alltoall(void* , int, MPI_Datatype, 
		 void*, int, MPI_Datatype, 
		 MPI_Comm);
int PMPI_Alltoallv(void* , int *, int *, 
		  MPI_Datatype, void*, int *, 
		  int *, MPI_Datatype, MPI_Comm);
int PMPI_Reduce(void* , void*, int, 
	       MPI_Datatype, MPI_Op, int, MPI_Comm);
int PMPI_Op_create(MPI_User_function *, int, MPI_Op *);
int PMPI_Op_free( MPI_Op *);
int PMPI_Allreduce(void* , void*, int, 
		  MPI_Datatype, MPI_Op, MPI_Comm);
int PMPI_Reduce_scatter(void* , void*, int *, 
		       MPI_Datatype, MPI_Op, MPI_Comm);
int PMPI_Scan(void* , void*, int, MPI_Datatype, 
	     MPI_Op, MPI_Comm );
int PMPI_Group_size(MPI_Group group, int *);
int PMPI_Group_rank(MPI_Group group, int *rank);
int PMPI_Group_translate_ranks (MPI_Group, int n, int *ranks1, 
			       MPI_Group, int *ranks2);
int PMPI_Group_compare(MPI_Group, MPI_Group, int *result);
int PMPI_Comm_group(MPI_Comm, MPI_Group *);
int PMPI_Group_union(MPI_Group, MPI_Group, MPI_Group *);
int PMPI_Group_intersection(MPI_Group, MPI_Group, 
			   MPI_Group *);
int PMPI_Group_difference(MPI_Group, MPI_Group, 
			 MPI_Group *);
int PMPI_Group_incl(MPI_Group group, int n, int *, MPI_Group *);
int PMPI_Group_excl(MPI_Group group, int n, int *, MPI_Group *);
int PMPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *);
int PMPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *);
int PMPI_Group_free(MPI_Group *);
int PMPI_Comm_size(MPI_Comm, int *);
int PMPI_Comm_rank(MPI_Comm, int *rank);
int PMPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
int PMPI_Comm_dup(MPI_Comm, MPI_Comm *);
int PMPI_Comm_create(MPI_Comm, MPI_Group group, MPI_Comm *);
int PMPI_Comm_split(MPI_Comm, int color, int key, MPI_Comm *);
int PMPI_Comm_free(MPI_Comm *comm);
int PMPI_Comm_test_inter(MPI_Comm, int *flag);
int PMPI_Comm_remote_size(MPI_Comm, int *);
int PMPI_Comm_remote_group(MPI_Comm, MPI_Group *);
int PMPI_Intercomm_create(MPI_Comm local_comm, int local_leader, 
			 MPI_Comm peer_comm, int remote_leader, 
			 int, MPI_Comm *newintercomm);
int PMPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintracomm);
int PMPI_Keyval_create(MPI_Copy_function *copy_fn, 
		      MPI_Delete_function *delete_fn, 
		      int *, void* extra_state);
int PMPI_Keyval_free(int *);
int PMPI_Attr_put(MPI_Comm, int, void* attribute_val);
int PMPI_Attr_get(MPI_Comm, int, void *attribute_val, int *flag);
int PMPI_Attr_delete(MPI_Comm, int);
int PMPI_Topo_test(MPI_Comm, int *);
int PMPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *,
		    int reorder, MPI_Comm *comm_cart);
int PMPI_Dims_create(int nnodes, int ndims, int *dims);
int PMPI_Graph_create(MPI_Comm, int, int *, int *, int, MPI_Comm *);
int PMPI_Graphdims_get(MPI_Comm, int *nnodes, int *nedges);
int PMPI_Graph_get(MPI_Comm, int, int, int *, int *);
int PMPI_Cartdim_get(MPI_Comm, int *ndims);
int PMPI_Cart_get(MPI_Comm, int maxdims, int *dims, int *,
		 int *coords);
int PMPI_Cart_rank(MPI_Comm, int *coords, int *rank);
int PMPI_Cart_coords(MPI_Comm, int, int maxdims, int *coords);
int PMPI_Graph_neighbors_count(MPI_Comm, int, int *nneighbors);
int PMPI_Graph_neighbors(MPI_Comm, int, int maxneighbors,
			int *neighbors);
int PMPI_Cart_shift(MPI_Comm, int direction, int disp, 
		   int *rank_source, int *rank_dest);
int PMPI_Cart_sub(MPI_Comm, int *remain_dims, MPI_Comm *);
int PMPI_Cart_map(MPI_Comm, int ndims, int *dims, int *, 
		 int *newrank);
int PMPI_Graph_map(MPI_Comm, int, int *, int *, int *);
int PMPI_Get_processor_name(char *name, int *result_len);
int PMPI_Get_version(int *, int *);
int PMPI_Errhandler_create(MPI_Handler_function *function, 
			  MPI_Errhandler *errhandler);
int PMPI_Errhandler_set(MPI_Comm, MPI_Errhandler errhandler);
int PMPI_Errhandler_get(MPI_Comm, MPI_Errhandler *errhandler);
int PMPI_Errhandler_free(MPI_Errhandler *errhandler);
int PMPI_Error_string(int errorcode, char *string, int *result_len);
int PMPI_Error_class(int errorcode, int *errorclass);

int PMPI_Type_get_envelope(MPI_Datatype datatype, int *num_integers,
             int *num_addresses, int *num_datatypes, int *combiner); 
int PMPI_Type_get_contents(MPI_Datatype datatype, int max_integers, int
             max_addresses, int max_datatypes, int *array_of_integers, 
             MPI_Aint *array_of_addresses, MPI_Datatype *array_of_datatypes);

int PMPI_Type_create_subarray(int ndims, int *array_of_sizes, int
                      *array_of_subsizes, int *array_of_starts, int order, 
                      MPI_Datatype oldtype, MPI_Datatype *newtype);

int PMPI_Type_create_darray(int size, int, int ndims, 
                    int *array_of_gsizes, int *array_of_distribs, 
                    int *array_of_dargs, int *array_of_psizes, 
                    int order, MPI_Datatype oldtype, MPI_Datatype *newtype);


int PMPI_Info_create(MPI_Info *info);
int PMPI_Info_set(MPI_Info, char *key, char *value);
int PMPI_Info_delete(MPI_Info, char *key);
int PMPI_Info_get(MPI_Info, char *key, int valuelen, 
                         char *value, int *flag);
int PMPI_Info_get_valuelen(MPI_Info, char *key, int *valuelen, 
                                  int *flag);
int PMPI_Info_get_nkeys(MPI_Info, int *nkeys);
int PMPI_Info_get_nthkey(MPI_Info, int n, char *key);
int PMPI_Info_dup(MPI_Info, MPI_Info *newinfo);
int PMPI_Info_free(MPI_Info *info);

MPI_Fint PMPI_Info_c2f(MPI_Info);
MPI_Info PMPI_Info_f2c(MPI_Fint);

/* Wtime done above */
int PMPI_Init(int *, char ***);
int PMPI_Finalize(void);
int PMPI_Initialized(int *flag);
int PMPI_Abort(MPI_Comm, int);
/* MPI-2 communicator naming functions */
int PMPI_Comm_set_name(MPI_Comm, char *);
int PMPI_Comm_get_name(MPI_Comm, char **);
#ifdef HAVE_NO_C_CONST
/* Default Solaris compiler does not accept const but does accept prototypes */
int PMPI_Pcontrol(int, ...);
#else
int PMPI_Pcontrol(const int, ...);
#endif
#endif  /* MPI_BUILD_PROFILING */


#endif
