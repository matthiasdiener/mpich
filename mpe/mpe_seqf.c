/* mpe_seq.c */
/* Fortran interface file for sun4 */
#include <stdio.h>
#include "mpe.h"
extern void *MPIR_ToPointer(); extern int MPIR_FromPointer();
 void  mpe_seq_begin_( comm, ng, __ierr )
MPI_Comm comm;
int*ng;
int *__ierr;
{
MPE_Seq_begin(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*ng);
}
 void  mpe_seq_end_( comm, ng, __ierr )
MPI_Comm comm;
int*ng;
int *__ierr;
{
MPE_Seq_end(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*ng);
}
