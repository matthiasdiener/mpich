#ifndef __MPI_BINDINGS
#define __MPI_BINDINGS

#include "mpi.h"

#if defined(__STDC__) || defined(__cplusplus)
int MPI_Send(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	     MPI_Comm comm);
int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int source, 
	     int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Bsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int MPI_Ssend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int MPI_Rsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int MPI_Buffer_attach( void* buffer, int size);
int MPI_Buffer_detach( void* buffer, int* size);
int MPI_Isend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm, MPI_Request *request);
int MPI_Ibsend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Issend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Irsend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, 
	      int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Wait(MPI_Request *request, MPI_Status *status);
int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
int MPI_Request_free(MPI_Request *request);
int MPI_Waitany(int, MPI_Request *, int *, MPI_Status *);
int MPI_Testany(int, MPI_Request *, int *, int *, MPI_Status *);
int MPI_Waitall(int count, MPI_Request *array_of_requests, 
		MPI_Status *array_of_statuses);
int MPI_Testall(int count, MPI_Request *array_of_requests, int *flag, 
		MPI_Status *array_of_statuses);
int MPI_Waitsome(int incount, MPI_Request *array_of_requests, int *outcount, 
		 int *array_of_indices, MPI_Status *array_of_statuses);
int MPI_Testsome(int incount, MPI_Request *array_of_requests, int *outcount, 
		 int *array_of_indices, MPI_Status *array_of_statuses);
int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, 
	       MPI_Status *status);
int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Cancel(MPI_Request *request);
int MPI_Test_cancelled(MPI_Status *status, int *flag);
int MPI_Send_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		  int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Bsend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Ssend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Rsend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Recv_init(void* buf, int count, MPI_Datatype datatype, int source, 
		  int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Start(MPI_Request *request);
int MPI_Startall(int count, MPI_Request *array_of_requests);
int MPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
		 int dest, int sendtag, void *recvbuf, int recvcount, 
		 MPI_Datatype recvtype, int source, int recvtag, 
		 MPI_Comm comm, MPI_Status *status);
int MPI_Sendrecv_replace(void* buf, int count, MPI_Datatype datatype, 
			 int dest, int sendtag, int source, int recvtag, 
			 MPI_Comm comm, MPI_Status *status);
int MPI_Type_contiguous(int count, MPI_Datatype oldtype, 
			MPI_Datatype *newtype);
int MPI_Type_vector(int count, int blocklength, int stride, 
		    MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_hvector(int count, int blocklength, MPI_Aint stride, 
		     MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_indexed(int count, int *array_of_blocklengths, 
		     int *array_of_displacements, MPI_Datatype oldtype, 
		     MPI_Datatype *newtype);
int MPI_Type_hindexed(int count, int *array_of_blocklengths, 
		      MPI_Aint *array_of_displacements, MPI_Datatype oldtype, 
		      MPI_Datatype *newtype);
int MPI_Type_struct(int count, int *array_of_blocklengths, 
		    MPI_Aint *array_of_displacements, 
		    MPI_Datatype *array_of_types, MPI_Datatype *newtype);
int MPI_Address(void* location, MPI_Aint *address);
int MPI_Type_extent(MPI_Datatype datatype, MPI_Aint *extent);

/* See the 1.1 version of the Standard; I think that the standard is in 
   error; however, it is the standard */
/* int MPI_Type_size(MPI_Datatype datatype, MPI_Aint *size); */
int MPI_Type_size(MPI_Datatype datatype, int *size);
int MPI_Type_count(MPI_Datatype datatype, int *count);
int MPI_Type_lb(MPI_Datatype datatype, MPI_Aint* displacement);
int MPI_Type_ub(MPI_Datatype datatype, MPI_Aint* displacement);
int MPI_Type_commit(MPI_Datatype *datatype);
int MPI_Type_free(MPI_Datatype *datatype);
int MPI_Get_elements(MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Pack(void* inbuf, int incount, MPI_Datatype datatype, void *outbuf, 
	     int outsize, int *position,  MPI_Comm comm);
int MPI_Unpack(void* inbuf, int insize, int *position, void *outbuf, 
	       int outcount, MPI_Datatype datatype, MPI_Comm comm);
int MPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, 
		  int *size);
int MPI_Barrier(MPI_Comm comm );
int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, 
	      MPI_Comm comm );
int MPI_Gather(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	       void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	       int root, MPI_Comm comm); 
int MPI_Gatherv(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		void* recvbuf, int *recvcounts, int *displs, 
		MPI_Datatype recvtype, int root, MPI_Comm comm); 
int MPI_Scatter(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		int root, MPI_Comm comm);
int MPI_Scatterv(void* sendbuf, int *sendcounts, int *displs, 
		 MPI_Datatype sendtype, void* recvbuf, int recvcount, 
		 MPI_Datatype recvtype, int root, MPI_Comm comm);
int MPI_Allgather(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		  void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		  MPI_Comm comm);
int MPI_Allgatherv(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		   void* recvbuf, int *recvcounts, int *displs, 
		   MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Alltoall(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		 void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		 MPI_Comm comm);
int MPI_Alltoallv(void* sendbuf, int *sendcounts, int *sdispls, 
		  MPI_Datatype sendtype, void* recvbuf, int *recvcounts, 
		  int *rdispls, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Reduce(void* sendbuf, void* recvbuf, int count, 
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Op_create(MPI_User_function *, int, MPI_Op *);
int MPI_Op_free( MPI_Op *);
int MPI_Allreduce(void* sendbuf, void* recvbuf, int count, 
		  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Reduce_scatter(void* sendbuf, void* recvbuf, int *recvcounts, 
		       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Scan(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, 
	     MPI_Op op, MPI_Comm comm );
int MPI_Group_size(MPI_Group group, int *size);
int MPI_Group_rank(MPI_Group group, int *rank);
int MPI_Group_translate_ranks (MPI_Group group1, int n, int *ranks1, 
			       MPI_Group group2, int *ranks2);
int MPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result);
int MPI_Comm_group(MPI_Comm comm, MPI_Group *group);
int MPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int MPI_Group_intersection(MPI_Group group1, MPI_Group group2, 
			   MPI_Group *newgroup);
int MPI_Group_difference(MPI_Group group1, MPI_Group group2, 
			 MPI_Group *newgroup);
int MPI_Group_incl(MPI_Group group, int n, int *ranks, MPI_Group *newgroup);
int MPI_Group_excl(MPI_Group group, int n, int *ranks, MPI_Group *newgroup);
int MPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *newgroup);
int MPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *newgroup);
int MPI_Group_free(MPI_Group *group);
int MPI_Comm_size(MPI_Comm comm, int *size);
int MPI_Comm_rank(MPI_Comm comm, int *rank);
int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
int MPI_Comm_free(MPI_Comm *comm);
int MPI_Comm_test_inter(MPI_Comm comm, int *flag);
int MPI_Comm_remote_size(MPI_Comm comm, int *size);
int MPI_Comm_remote_group(MPI_Comm comm, MPI_Group *group);
int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader, 
			 MPI_Comm peer_comm, int remote_leader, 
			 int tag, MPI_Comm *newintercomm);
int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintracomm);
int MPI_Keyval_create(MPI_Copy_function *copy_fn, 
		      MPI_Delete_function *delete_fn, 
		      int *keyval, void* extra_state);
int MPI_Keyval_free(int *keyval);
int MPI_Attr_put(MPI_Comm comm, int keyval, void* attribute_val);
int MPI_Attr_get(MPI_Comm comm, int keyval, void *attribute_val, int *flag);
int MPI_Attr_delete(MPI_Comm comm, int keyval);
int MPI_Topo_test(MPI_Comm comm, int *status);
int MPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *periods,
		    int reorder, MPI_Comm *comm_cart);
int MPI_Dims_create(int nnodes, int ndims, int *dims);
int MPI_Graph_create(MPI_Comm, int, int *, int *, int, MPI_Comm *);
int MPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges);
int MPI_Graph_get(MPI_Comm, int, int, int *, int *);
int MPI_Cartdim_get(MPI_Comm comm, int *ndims);
int MPI_Cart_get(MPI_Comm comm, int maxdims, int *dims, int *periods,
		 int *coords);
int MPI_Cart_rank(MPI_Comm comm, int *coords, int *rank);
int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int *coords);
int MPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors);
int MPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors,
			int *neighbors);
int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, 
		   int *rank_source, int *rank_dest);
int MPI_Cart_sub(MPI_Comm comm, int *remain_dims, MPI_Comm *newcomm);
int MPI_Cart_map(MPI_Comm comm, int ndims, int *dims, int *periods, 
		 int *newrank);
int MPI_Graph_map(MPI_Comm, int, int *, int *, int *);
int MPI_Get_processor_name(char *name, int *result_len);
int MPI_Errhandler_create(MPI_Handler_function *function, 
			  MPI_Errhandler *errhandler);
int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int MPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler *errhandler);
int MPI_Errhandler_free(MPI_Errhandler *errhandler);
int MPI_Error_string(int errorcode, char *string, int *result_len);
int MPI_Error_class(int errorcode, int *errorclass);
double MPI_Wtime(void);
double MPI_Wtick(void);
#ifndef MPI_Wtime
double PMPI_Wtime(void);
double PMPI_Wtick(void);
#endif
int MPI_Init(int *argc, char ***argv);
int MPI_Finalize(void);
int MPI_Initialized(int *flag);
int MPI_Abort(MPI_Comm comm, int errorcode);
int MPI_Pcontrol(const int level, ...);

int MPI_NULL_COPY_FN ( MPI_Comm oldcomm, int keyval, void *extra_state, 
		       void *attr_in, void *attr_out, int *flag );
int MPI_NULL_DELETE_FN ( MPI_Comm comm, int keyval, void *attr, 
			 void *extra_state );
int MPI_DUP_FN ( MPI_Comm comm, int keyval, void *extra_state, void *attr_in,
		 void *attr_out, int *flag );

/* Here are the bindings of the profiling routines */
#if !defined(MPI_BUILD_PROFILING)
int PMPI_Send(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	     MPI_Comm comm);
int PMPI_Recv(void* buf, int count, MPI_Datatype datatype, int source, 
	     int tag, MPI_Comm comm, MPI_Status *status);
int PMPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count);
int PMPI_Bsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int PMPI_Ssend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int PMPI_Rsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm);
int PMPI_Buffer_attach( void* buffer, int size);
int PMPI_Buffer_detach( void* buffer, int* size);
int PMPI_Isend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, 
	      MPI_Comm comm, MPI_Request *request);
int PMPI_Ibsend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Issend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Irsend(void* buf, int count, MPI_Datatype datatype, int dest, 
	       int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, 
	      int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Wait(MPI_Request *request, MPI_Status *status);
int PMPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
int PMPI_Request_free(MPI_Request *request);
int PMPI_Waitany(int, MPI_Request *, int *, MPI_Status *);
int PMPI_Testany(int, MPI_Request *, int *, int *, MPI_Status *);
int PMPI_Waitall(int count, MPI_Request *array_of_requests, 
		MPI_Status *array_of_statuses);
int PMPI_Testall(int count, MPI_Request *array_of_requests, int *flag, 
		MPI_Status *array_of_statuses);
int PMPI_Waitsome(int incount, MPI_Request *array_of_requests, int *outcount, 
		 int *array_of_indices, MPI_Status *array_of_statuses);
int PMPI_Testsome(int incount, MPI_Request *array_of_requests, int *outcount, 
		 int *array_of_indices, MPI_Status *array_of_statuses);
int PMPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, 
	       MPI_Status *status);
int PMPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
int PMPI_Cancel(MPI_Request *request);
int PMPI_Test_cancelled(MPI_Status *status, int *flag);
int PMPI_Send_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		  int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Bsend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Ssend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Rsend_init(void* buf, int count, MPI_Datatype datatype, int dest, 
		   int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Recv_init(void* buf, int count, MPI_Datatype datatype, int source, 
		  int tag, MPI_Comm comm, MPI_Request *request);
int PMPI_Start(MPI_Request *request);
int PMPI_Startall(int count, MPI_Request *array_of_requests);
int PMPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
		 int dest, int sendtag, void *recvbuf, int recvcount, 
		 MPI_Datatype recvtype, int source, int recvtag, 
		 MPI_Comm comm, MPI_Status *status);
int PMPI_Sendrecv_replace(void* buf, int count, MPI_Datatype datatype, 
			 int dest, int sendtag, int source, int recvtag, 
			 MPI_Comm comm, MPI_Status *status);
int PMPI_Type_contiguous(int count, MPI_Datatype oldtype, 
			MPI_Datatype *newtype);
int PMPI_Type_vector(int count, int blocklength, int stride, 
		    MPI_Datatype oldtype, MPI_Datatype *newtype);
int PMPI_Type_hvector(int count, int blocklength, MPI_Aint stride, 
		     MPI_Datatype oldtype, MPI_Datatype *newtype);
int PMPI_Type_indexed(int count, int *array_of_blocklengths, 
		     int *array_of_displacements, MPI_Datatype oldtype, 
		     MPI_Datatype *newtype);
int PMPI_Type_hindexed(int count, int *array_of_blocklengths, 
		      MPI_Aint *array_of_displacements, MPI_Datatype oldtype, 
		      MPI_Datatype *newtype);
int PMPI_Type_struct(int count, int *array_of_blocklengths, 
		    MPI_Aint *array_of_displacements, 
		    MPI_Datatype *array_of_types, MPI_Datatype *newtype);
int PMPI_Address(void* location, MPI_Aint *address);
int PMPI_Type_extent(MPI_Datatype datatype, MPI_Aint *extent);

/* See the 1.1 version of the Standard; I think that the standard is in 
   error; however, it is the standard */
/* int PMPI_Type_size(MPI_Datatype datatype, MPI_Aint *size); */
int PMPI_Type_size(MPI_Datatype datatype, int *size);
int PMPI_Type_count(MPI_Datatype datatype, int *count);
int PMPI_Type_lb(MPI_Datatype datatype, MPI_Aint* displacement);
int PMPI_Type_ub(MPI_Datatype datatype, MPI_Aint* displacement);
int PMPI_Type_commit(MPI_Datatype *datatype);
int PMPI_Type_free(MPI_Datatype *datatype);
int PMPI_Get_elements(MPI_Status *status, MPI_Datatype datatype, int *count);
int PMPI_Pack(void* inbuf, int incount, MPI_Datatype datatype, void *outbuf, 
	     int outsize, int *position,  MPI_Comm comm);
int PMPI_Unpack(void* inbuf, int insize, int *position, void *outbuf, 
	       int outcount, MPI_Datatype datatype, MPI_Comm comm);
int PMPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, 
		  int *size);
int PMPI_Barrier(MPI_Comm comm );
int PMPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, 
	      MPI_Comm comm );
int PMPI_Gather(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	       void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	       int root, MPI_Comm comm); 
int PMPI_Gatherv(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		void* recvbuf, int *recvcounts, int *displs, 
		MPI_Datatype recvtype, int root, MPI_Comm comm); 
int PMPI_Scatter(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		int root, MPI_Comm comm);
int PMPI_Scatterv(void* sendbuf, int *sendcounts, int *displs, 
		 MPI_Datatype sendtype, void* recvbuf, int recvcount, 
		 MPI_Datatype recvtype, int root, MPI_Comm comm);
int PMPI_Allgather(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		  void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		  MPI_Comm comm);
int PMPI_Allgatherv(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		   void* recvbuf, int *recvcounts, int *displs, 
		   MPI_Datatype recvtype, MPI_Comm comm);
int PMPI_Alltoall(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		 void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		 MPI_Comm comm);
int PMPI_Alltoallv(void* sendbuf, int *sendcounts, int *sdispls, 
		  MPI_Datatype sendtype, void* recvbuf, int *recvcounts, 
		  int *rdispls, MPI_Datatype recvtype, MPI_Comm comm);
int PMPI_Reduce(void* sendbuf, void* recvbuf, int count, 
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int PMPI_Op_create(MPI_User_function *, int, MPI_Op *);
int PMPI_Op_free( MPI_Op *);
int PMPI_Allreduce(void* sendbuf, void* recvbuf, int count, 
		  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int PMPI_Reduce_scatter(void* sendbuf, void* recvbuf, int *recvcounts, 
		       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int PMPI_Scan(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, 
	     MPI_Op op, MPI_Comm comm );
int PMPI_Group_size(MPI_Group group, int *size);
int PMPI_Group_rank(MPI_Group group, int *rank);
int PMPI_Group_translate_ranks (MPI_Group group1, int n, int *ranks1, 
			       MPI_Group group2, int *ranks2);
int PMPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result);
int PMPI_Comm_group(MPI_Comm comm, MPI_Group *group);
int PMPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int PMPI_Group_intersection(MPI_Group group1, MPI_Group group2, 
			   MPI_Group *newgroup);
int PMPI_Group_difference(MPI_Group group1, MPI_Group group2, 
			 MPI_Group *newgroup);
int PMPI_Group_incl(MPI_Group group, int n, int *ranks, MPI_Group *newgroup);
int PMPI_Group_excl(MPI_Group group, int n, int *ranks, MPI_Group *newgroup);
int PMPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *newgroup);
int PMPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], 
			 MPI_Group *newgroup);
int PMPI_Group_free(MPI_Group *group);
int PMPI_Comm_size(MPI_Comm comm, int *size);
int PMPI_Comm_rank(MPI_Comm comm, int *rank);
int PMPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
int PMPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int PMPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
int PMPI_Comm_free(MPI_Comm *comm);
int PMPI_Comm_test_inter(MPI_Comm comm, int *flag);
int PMPI_Comm_remote_size(MPI_Comm comm, int *size);
int PMPI_Comm_remote_group(MPI_Comm comm, MPI_Group *group);
int PMPI_Intercomm_create(MPI_Comm local_comm, int local_leader, 
			 MPI_Comm peer_comm, int remote_leader, 
			 int tag, MPI_Comm *newintercomm);
int PMPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintracomm);
int PMPI_Keyval_create(MPI_Copy_function *copy_fn, 
		      MPI_Delete_function *delete_fn, 
		      int *keyval, void* extra_state);
int PMPI_Keyval_free(int *keyval);
int PMPI_Attr_put(MPI_Comm comm, int keyval, void* attribute_val);
int PMPI_Attr_get(MPI_Comm comm, int keyval, void *attribute_val, int *flag);
int PMPI_Attr_delete(MPI_Comm comm, int keyval);
int PMPI_Topo_test(MPI_Comm comm, int *status);
int PMPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *periods,
		    int reorder, MPI_Comm *comm_cart);
int PMPI_Dims_create(int nnodes, int ndims, int *dims);
int PMPI_Graph_create(MPI_Comm, int, int *, int *, int, MPI_Comm *);
int PMPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges);
int PMPI_Graph_get(MPI_Comm, int, int, int *, int *);
int PMPI_Cartdim_get(MPI_Comm comm, int *ndims);
int PMPI_Cart_get(MPI_Comm comm, int maxdims, int *dims, int *periods,
		 int *coords);
int PMPI_Cart_rank(MPI_Comm comm, int *coords, int *rank);
int PMPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int *coords);
int PMPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors);
int PMPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors,
			int *neighbors);
int PMPI_Cart_shift(MPI_Comm comm, int direction, int disp, 
		   int *rank_source, int *rank_dest);
int PMPI_Cart_sub(MPI_Comm comm, int *remain_dims, MPI_Comm *newcomm);
int PMPI_Cart_map(MPI_Comm comm, int ndims, int *dims, int *periods, 
		 int *newrank);
int PMPI_Graph_map(MPI_Comm, int, int *, int *, int *);
int PMPI_Get_processor_name(char *name, int *result_len);
int PMPI_Errhandler_create(MPI_Handler_function *function, 
			  MPI_Errhandler *errhandler);
int PMPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int PMPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler *errhandler);
int PMPI_Errhandler_free(MPI_Errhandler *errhandler);
int PMPI_Error_string(int errorcode, char *string, int *result_len);
int PMPI_Error_class(int errorcode, int *errorclass);
/* Wtime done above */
int PMPI_Init(int *argc, char ***argv);
int PMPI_Finalize(void);
int PMPI_Initialized(int *flag);
int PMPI_Abort(MPI_Comm comm, int errorcode);
int PMPI_Pcontrol(const int level, ...);
#endif  /* MPI_BUILD_PROFILING */

#else 
extern double MPI_Wtime();
extern double MPI_Wtick();
#ifndef MPI_Wtime
extern double PMPI_Wtime();
extern double PMPI_Wtick();
#endif

extern int MPI_NULL_COPY_FN(), MPI_NULL_DELETE_FN();
#endif

#endif
