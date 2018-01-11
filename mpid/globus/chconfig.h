#ifndef NEXUSCONFIG
#define NEXUSCONFIG

#define private_count extra[0]

#define MPID_HAS_HETERO

/* Put macro-definitions of routines here */

#define MPID_CommInit(oldcomm,newcomm) MPI_SUCCESS
#endif
