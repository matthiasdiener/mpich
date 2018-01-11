#include <mpi.h>
#include <stdio.h>

main(argc, argv)
     int argc;
     char **argv;
{
  /* should be int by p. 68 */
  MPI_Aint
    i_size
      ;
    
  MPI_Init(&argc, &argv);


  MPI_Type_extent(MPI_INT, &i_size);
  printf("MPI_Type_extent (MPI_INT) = %d\n", i_size);

  MPI_Type_size(MPI_INT, &i_size);
  printf("MPI_Type_size (MPI_INT) = %d\n", i_size);


  MPI_Type_extent(MPI_UNSIGNED, &i_size);
  printf("MPI_Type_extent (MPI_UNSIGNED) = %d\n", i_size);

  MPI_Type_size(MPI_UNSIGNED, &i_size);
  printf("MPI_Type_size (MPI_UNSIGNED) = %d\n", i_size);

#if defined(__STDC__) && !defined(MPI_rs6000)
  MPI_Type_extent(MPI_LONG_DOUBLE, &i_size);
  printf("MPI_Type_extent (MPI_LONG_DOUBLE) = %d\n", i_size);

  MPI_Type_size(MPI_LONG_DOUBLE, &i_size);
  printf("MPI_Type_size (MPI_LONG_DOUBLE) = %d\n", i_size);
#endif

  MPI_Finalize();
}
