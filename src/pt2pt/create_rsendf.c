/* create_rsend.c */
/* Custom Fortran interface file for sun4 */
#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif
#include "mpiimpl.h"
 void mpi_rsend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int           *count;
MPI_Datatype  datatype;
int           *dest;
int           *tag;
MPI_Comm      comm;
MPI_Request   *request;
int *__ierr;
{
*__ierr = MPI_Rsend_init(buf,*count,
	(MPI_Datatype)*((int*)datatype),*dest,*tag,
	(MPI_Comm)*((int*)comm),request);
}
