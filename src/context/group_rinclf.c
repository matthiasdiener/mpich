/* group_rincl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
/* 
   Here we have a tricky situation.  In Fortran, the ranges will be
   an array integer ranges(3,*).  If there are n elements, then this is
   just 3*n integers in the order ranges(1,1),ranges(2,1),ranges(3,1),
   ranges(1,2),... .

   Now, the C binding is for int ranges[][3].  Now, note that the size
   of int [a][b] is a*b*sizeof(int) (NOT a * sizeof(int*)).  Also note
   that int foo[][b] is NOT a valid declaration EXCEPT for an actual 
   parameter to a routine.  What does all of this mean?  It means that
   while ranges[k] is a pointer to a type that consists of an int with
   3 components, it is not an arbitrary pointer; rather, it is computed from
   the layout of the data for an 2-d array in C.  Thus, all we need to do
   is pass the Fortran ranges straight through to C.

   Some compilers may complain about this; if you want to avoid the error
   message, you'll need to copy the "ranges" array into a temporary.
 */

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_range_incl_ PMPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_incl_ pmpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_incl_ pmpi_group_range_incl
#else
#define mpi_group_range_incl_ pmpi_group_range_incl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_range_incl_ MPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_incl_ mpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_incl_ mpi_group_range_incl
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_range_incl_ ANSI_ARGS(( MPI_Group *, int *, int [][3],
				       MPI_Group *, int * ));

void mpi_group_range_incl_ ( group, n, ranges, newgroup, __ierr )
MPI_Group *group, *newgroup;
int       *n;
int       ranges[][3];
int       *__ierr;
{
    *__ierr = MPI_Group_range_incl(*group,*n,ranges,newgroup);
}
