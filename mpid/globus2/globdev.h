#ifndef __globdev__
#define __globdev__

/* used for all message headers */
#define HEADERLEN 10

#include "mpi2.h" /* MPI-2 Extensions */

#include <globus_io.h>
#include <globus_dc.h>
#include "debug.h" /* debug must come before the other globus2 header files */
#include "mpid.h"
#include "protos.h"
#include "protos_details.h"
#include "mem.h"
#include "vmpi.h"
#include "mpipt2pt.h"

/**************************/
/* begin MPI-2 Extensions */
/**************************/

struct commworldchannels
{
    int nprocs;
    char name[COMMWORLDCHANNELSNAMELEN];
    struct channel_t *channels;
}; /* end struct commworldchannels */

#define COMMWORLDCHANNELS_TABLE_STEPSIZE 100

/************************/
/* end MPI-2 Extensions */
/************************/

#if defined(VMPI)

struct mpircvreq
{
    struct mpircvreq *prev, *next;
    MPIR_RHANDLE *req;
}; /* end struct mpircvreq */

struct mpi_posted_queue
{
    struct mpircvreq *head, *tail;
}; /* end struct mpi_posted_queue */

#endif


/*
 * These macros are used to manipulate the "extra" data in MPI_Status.  The
 * first word of extra data is used to store item such as the remote
 * (dataorigin) format (rightmost byte of the word) and information about
 * whether the count is in local format, remote format, or needs to be obtained
 * from VMPI.  The remaining words are used to store the MPI_Status from VMPI
 * when necessary.
 */

#define STATUS_INFO_FORMAT_MASK       0x00ff

#define STATUS_INFO_SET_FORMAT(S,F) \
{ \
    (S).extra[0] = ((S).extra[0] & ~STATUS_INFO_FORMAT_MASK) \
      | ((F) & 0xff); \
}

#define STATUS_INFO_GET_FORMAT(S) ((S).extra[0] & STATUS_INFO_FORMAT_MASK)

#define STATUS_INFO_COUNT_MASK        0x0700
#define STATUS_INFO_COUNT_LOCAL       0x0100
#define STATUS_INFO_COUNT_REMOTE      0x0200
#define STATUS_INFO_COUNT_VMPI        0x0400

#define STATUS_INFO_SET_COUNT_NONE(S) \
{ \
    (S).extra[0] = ((S).extra[0] & ~STATUS_INFO_COUNT_MASK); \
}

#define STATUS_INFO_SET_COUNT_LOCAL(S) \
{ \
    (S).extra[0] = ((S).extra[0] & ~STATUS_INFO_COUNT_MASK) \
      | STATUS_INFO_COUNT_LOCAL; \
}

#define STATUS_INFO_IS_COUNT_LOCAL(S) \
(((S).extra[0] & STATUS_INFO_COUNT_MASK) == STATUS_INFO_COUNT_LOCAL)

#define STATUS_INFO_SET_COUNT_REMOTE(S) \
{ \
    (S).extra[0] = ((S).extra[0] & ~STATUS_INFO_COUNT_MASK) \
      | STATUS_INFO_COUNT_REMOTE; \
}

#define STATUS_INFO_IS_COUNT_REMOTE(S) \
(((S).extra[0] & STATUS_INFO_COUNT_MASK) == STATUS_INFO_COUNT_REMOTE)

#define STATUS_INFO_SET_COUNT_VMPI(S) \
{ \
    (S).extra[0] = ((S).extra[0] & ~STATUS_INFO_COUNT_MASK) \
      | STATUS_INFO_COUNT_VMPI; \
}

#define STATUS_INFO_IS_COUNT_VMPI(S) \
(((S).extra[0] & STATUS_INFO_COUNT_MASK) == STATUS_INFO_COUNT_VMPI)

#define STATUS_INFO_GET_VMPI_PTR(S) (&((S).extra[1]))

/*
 * These macros are used to manipulate the "extra" data in MPIR_Datatype.  
 * The "extra" field is a placeholder for vMPI to store it's MPI_Datatype.
 */
#define VMPI_PTR_FROM_MPIR_PTR(d) ((void *) ((d)->extra))

/*************************/
/* Function Declarations */
/*************************/

/* init_g.c */
void build_channels(int nprocs,
		    globus_byte_t **mi_protos_vector,
		    struct channel_t **channels);
void select_protocols(int nprocs, struct channel_t *channels);
void print_channels();
struct channel_t *get_channel(int grank);
int get_channel_rowidx(int grank, int *displ /* optional */); 
int commworld_name_to_rowidx(char *name);
int commworld_name_displ_to_grank(char *name, int displ);

/* send_g.c */
int enqueue_tcp_send(struct tcpsendreq *sr);

/* recv_g.c */
#if defined(VMPI)
void remove_and_free_mpircvreq(struct mpircvreq *mp);
#endif
int remote_size(int count, struct MPIR_DATATYPE *datatype, int format);
int extract_data_into_req(MPIR_RHANDLE *req,
                               void *src_buff,
                               int src_len,
                               int src_format,
                               int src_lrank,
                               int src_tag);
int extract_complete_from_buff(globus_byte_t **src,
				globus_byte_t *dest,
				int count,
				struct MPIR_DATATYPE *datatype,
				int format,
				int *nbytes_recvd);
globus_bool_t send_ack_over_tcp(int grank, void *liba, int libasize);
#if defined(VMPI)
int mpi_recv_or_post(MPIR_RHANDLE *in_req, int *error_code);
#endif

/* pack_g.c */
int local_size(int count, struct MPIR_DATATYPE *datatype);

void
mpich_globus2_pack_data(
    void *				src,
    int					count,
    struct MPIR_DATATYPE *		datatype,
    void *				dest_buff_start,
    int *				position,
    int *				error_code);

void
mpich_globus2_unpack_data(
    void *				src_buff_start,
    int *				in_position,
    int					src_format,
    void *				dest_buff_start,
    int					count,
    struct MPIR_DATATYPE *		datatype,
    int *				out_position,
    int *				error_code);

/* probe_g.c */
int get_proto(struct MPIR_COMMUNICATOR *comm, int src_lrank);

/* pr_tcp_g.c */
void listen_callback(void *callback_arg,
                    globus_io_handle_t *handle,
                    globus_result_t result);
void read_callback(void *callback_arg,
                    globus_io_handle_t *handle,
                    globus_result_t result,
                    globus_byte_t *buff,   /* where date resides */
                    globus_size_t nbytes); /* number of bytes read */
void prime_the_line(struct tcp_miproto_t *tp, int dest_grank);

/*
 * Some experimental release-consistent architecures require a lock to be
 * acquired before shared data, modified by another processor/node, will be
 * visible.  As far as I know, none of the commercial systems have this
 * property (yet).  If you encounter a system that has such consistency
 * semantics, you will need to define RC_REQUIRES_ACQUIRE.
 */
#if defined(RC_REQUIRES_ACQUIRE)
#    define RC_mutex_lock(M) globus_mutex_lock(M)
#    define RC_mutex_unlock(M) globus_mutex_unlock(M)
#else
#    define RC_mutex_lock(M)
#    define RC_mutex_unlock(M)
#endif

#endif /* __globdev__ */
