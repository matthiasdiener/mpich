/* comm_make.c */
/* Fortran interface file for sun4 */
#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif
#include "mpiimpl.h"
 void mpi_comm_create_ ( comm, group, comm_out, __ierr )
MPI_Comm comm;
MPI_Group group;
MPI_Comm *comm_out;
int *__ierr;
{
*__ierr = MPI_Comm_create(
	(MPI_Comm)*((int*)comm),
	(MPI_Group)*((int*)group),comm_out);
}
