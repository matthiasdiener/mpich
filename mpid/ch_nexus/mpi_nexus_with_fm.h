/**********************************************************************
$Id$

$Source$

$Log$

Header for MPI-over-Nexus functions for interfacing with FM runtime.
**********************************************************************/


#ifndef MPI_NEXUS_WITH_FM_H
#define MPI_NEXUS_WITH_FM_H

#include "nexus.h"

		/* Function called by NexusBoot() to register all MPI
		   handlers. */
void MPID_NEXUS_register_MPI_handlers(void);

		/* Function called by FM runtime initialization code on
		   each node to tell MPI the whole set of nodes in the
		   program. */
void
MPID_NEXUS_get_FM_nodes(int n_nodes, nexus_global_pointer_t nodes[]);


#endif	/* #ifndef MPI_NEXUS_WITH_FM_H */

