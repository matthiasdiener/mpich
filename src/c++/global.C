/*
 *  $Id: global.C,v 1.1 1994/05/12 07:17:42 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include <mpi++P.h>

// Global variables for C++ interface
MPIX_Comm_world  MPIX_COMM_WORLD;
MPIX_Comm        MPIX_COMM_NULL;
MPIX_Group       MPIX_GROUP_NULL, MPIX_GROUP_EMPTY;

// Define static member in MPIX_Comm_world class
int MPIX_Comm_world::init = 0;

// Initial datatypes
MPIX_Datatype  MPIX_CHAR;
MPIX_Datatype  MPIX_SHORT;
MPIX_Datatype  MPIX_INT;
MPIX_Datatype  MPIX_LONG;
MPIX_Datatype  MPIX_UNSIGNED_CHAR;
MPIX_Datatype  MPIX_UNSIGNED_SHORT;
MPIX_Datatype  MPIX_UNSIGNED;
MPIX_Datatype  MPIX_UNSIGNED_LONG;
MPIX_Datatype  MPIX_FLOAT;
MPIX_Datatype  MPIX_DOUBLE;
MPIX_Datatype  MPIX_LONG_DOUBLE;
MPIX_Datatype  MPIX_BYTE;
MPIX_Datatype  MPIX_PACKED;

// Initial ops
MPIX_Op  MPIX_SUM;
MPIX_Op  MPIX_MAX;
MPIX_Op  MPIX_MIN;
MPIX_Op  MPIX_PROD;
MPIX_Op  MPIX_LAND;
MPIX_Op  MPIX_BAND;
MPIX_Op  MPIX_LOR;
MPIX_Op  MPIX_BOR;
MPIX_Op  MPIX_LXOR;
MPIX_Op  MPIX_BXOR;

// Init function 
MPIX_Comm_world::Init(int& argc, char **argv) 
{
  // Have we been initialized before? 
  if (init)
	return (MPI_ERR_INIT);

  // Initialize stuff
  int error = MPI_Init (&argc, &argv); 
  comm = MPI_COMM_WORLD;

  // Initialize MPI constants
  MPIX_COMM_NULL.comm      = MPI_COMM_NULL;
  MPIX_GROUP_NULL.group    = MPI_GROUP_NULL;
  MPIX_GROUP_EMPTY.group   = MPI_GROUP_EMPTY;
  MPIX_CHAR.type           = MPI_CHAR;
  MPIX_SHORT.type          = MPI_SHORT;
  MPIX_INT.type            = MPI_INT;
  MPIX_LONG.type           = MPI_LONG;
  MPIX_UNSIGNED_CHAR.type  = MPI_UNSIGNED_CHAR;
  MPIX_UNSIGNED_SHORT.type = MPI_UNSIGNED_SHORT;
  MPIX_UNSIGNED.type       = MPI_UNSIGNED;
  MPIX_UNSIGNED_LONG.type  = MPI_UNSIGNED_LONG;
  MPIX_FLOAT.type          = MPI_FLOAT;
  MPIX_DOUBLE.type         = MPI_DOUBLE;
  MPIX_LONG_DOUBLE.type    = MPI_LONG_DOUBLE;
  MPIX_BYTE.type           = MPI_BYTE;
  MPIX_PACKED.type         = MPI_PACKED;
  MPIX_SUM.op              = MPI_SUM;
  MPIX_MAX.op              = MPI_MAX;
  MPIX_MIN.op              = MPI_MIN;
  MPIX_PROD.op             = MPI_PROD;
  MPIX_LAND.op             = MPI_LAND;
  MPIX_BAND.op             = MPI_BAND;
  MPIX_LOR.op              = MPI_LOR;
  MPIX_BOR.op              = MPI_BOR;
  MPIX_LXOR.op             = MPI_LXOR;
  MPIX_BXOR.op             = MPI_BXOR;

  return (error);
}

int MPIX_Datatype::Struct(int count,int blocklens[],MPI_Aint displs[],
                          MPIX_Datatype oldtype[]) 
{
  int errno;
  MPI_Datatype *types = new MPI_Datatype[count];
  for (int i=0; i<count; i++) { types[i] = oldtype[i].type; }
  errno = MPI_Type_struct ( count, blocklens, displs, types, &type);
  delete types;
  return (errno);
}
