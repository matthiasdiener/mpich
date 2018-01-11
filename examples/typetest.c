#include "mpi.h"

main( argc, argv )
int argc;
char **argv;
{
    MPI_DATATYPE contig;
    MPI_DATATYPE vector;
    MPI_DATATYPE hvector;
    MPI_DATATYPE indexed;
    MPI_DATATYPE hindexed;
    MPI_DATATYPE structype;

    static int index_array[4]         = { 1,  2,  3,  4 };
    static int blocklen_array[4]      = { 8, 12, 16, 20 };
    MPI_DATATYPE type_array[4];

    MPIR_FDTEL *fdte, **tailptr;
    int disp;

    MPI_Init( &argc, &argv );

    printf("testing flattening...\n");
    disp = 0;
    MPIR_flatten_dte ( MPI_INT, &fdte, &tailptr, &disp );
    *tailptr = NULL;
    MPIR_dump_flat_dte( fdte );

    disp = 0;
    MPIR_flatten_dte ( MPI_FLOAT, &fdte, &tailptr, &disp );
    *tailptr = NULL;
    MPIR_dump_flat_dte( fdte );

    printf("\n4 floats:\n");
    MPI_Type_Contiguous( 4, MPI_FLOAT, &contig );
    MPIR_dump_datatype( contig );

    disp = 0;
    MPIR_flatten_dte ( contig, &fdte, &tailptr, &disp );
    *tailptr = NULL;
    MPIR_dump_flat_dte( fdte );

/*
    printf("Basic datatypes:\n");
    MPIR_dump_datatype( MPI_INT );
    MPIR_dump_datatype( MPI_FLOAT );
    MPIR_dump_datatype( MPI_DOUBLE );
    MPIR_dump_datatype( MPI_LONG );
    MPIR_dump_datatype( MPI_SHORT );
    MPIR_dump_datatype( MPI_CHAR );
    MPIR_dump_datatype( MPI_BYTE );

    printf("\nvector of 4 blocks of integers with stride 5 and blocklen 8:\n");
    MPI_Type_Vector( 4, MPI_INT, 5, 8, &vector );
    MPIR_dump_datatype( vector );

    printf("\nhindexed of 4 blocks of above vectors:\n");
    MPI_Type_Hindexed( 4, vector, index_array, blocklen_array, &hindexed );
    MPIR_dump_datatype( hindexed );

    printf("\nstruct of all of the above types:\n");
    type_array[0] = contig;
    type_array[1] = vector;
    type_array[2] = hindexed;
    type_array[3] = contig;
    MPI_Type_Struct( 4, type_array, index_array, blocklen_array, &structype );
    MPIR_dump_datatype( structype );
*/
    MPI_Finalize();
}
