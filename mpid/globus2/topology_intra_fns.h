
#ifndef TOPOLOGY_INTRA_FNS_H
#define TOPOLOGY_INTRA_FNS_H

#include "mpid.h"


/*********************/
/* public prototypes */
/*********************/

extern int MPID_FN_Bcast (void *, int, struct MPIR_DATATYPE *, int,
                          struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Barrier (struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Gather (void *, int, struct MPIR_DATATYPE *, void *, int,
                           struct MPIR_DATATYPE *, int,
                           struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Gatherv (void *, int, struct MPIR_DATATYPE *, void *, int *,
                            int *, struct MPIR_DATATYPE *, int,
                            struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Scatter (void *, int, struct MPIR_DATATYPE *, void *, int,
                            struct MPIR_DATATYPE *, int,
                            struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Scatterv (void *, int *, int *, struct MPIR_DATATYPE *,
                             void *, int, struct MPIR_DATATYPE *, int,
                             struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Allgather (void *, int, struct MPIR_DATATYPE *, void *, int,
                              struct MPIR_DATATYPE *,
                              struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Allgatherv (void *, int, struct MPIR_DATATYPE *, void *,
                               int *, int *, struct MPIR_DATATYPE *,
                               struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Alltoall (void *, int, struct MPIR_DATATYPE *, void *, int,
                             struct MPIR_DATATYPE *,
                             struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Alltoallv (void *, int *, int *, struct MPIR_DATATYPE *,
                              void *, int *, int *, struct MPIR_DATATYPE *,
                              struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Reduce (void *, void *, int, struct MPIR_DATATYPE *, MPI_Op,
                           int, struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Allreduce (void *, void *, int, struct MPIR_DATATYPE *,
                              MPI_Op, struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Reduce_scatter (void *, void *, int *,
                                   struct MPIR_DATATYPE *, MPI_Op,
                                   struct MPIR_COMMUNICATOR *);
extern int MPID_FN_Scan (void *, void *, int, struct MPIR_DATATYPE *, MPI_Op,
                         struct MPIR_COMMUNICATOR *);

#endif   /* TOPOLOGY_INTRA_FNS_H */

