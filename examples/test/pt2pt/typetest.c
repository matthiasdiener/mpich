/* 
 * Patrick Bridges * bridges@mcs.anl.gov * patrick@CS.MsState.Edu 
 */

#include <stdio.h>
#include "test.h"
#include "mpi.h"
#include <string.h>
#include <memory.h>

struct struct1 {
    double d1;
    char   c1[8];
};


struct struct2 {
    double d1;
    double d2;
    char   c1[8];
    char   c2[8];
    double d3;
    char   c3[8];
    double d4;
    char   c4[8];
};

struct struct3 {
    double d1[2];
    char  c1[2][8];
    struct struct1 s1[2];
};


int main(argc, argv)
int argc;
char **argv;
{
    int rank, size, ret; 
    char *Current_Test = NULL;
    MPI_Status Status;
    MPI_Datatype struct1_t, struct2_t, struct3_t,
	astruct1_t, carray_t;
    static int block1[2] = {1, 1};
    static int block2[6] = {2, 2, 1, 1, 1, 1};
    static int block3[3] = {2, 2, 1};
    MPI_Aint disp1[2], disp2[6], disp3[6];
    MPI_Datatype type1[2], type2[6], type3[3];
    struct struct1 dummy1;
    struct struct2 dummy2;
    struct struct3 dummy3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    Test_Init("typetest", rank);

    /* Create some types to try out */

    /* A simple array of characters */ 
    MPI_Type_contiguous(8, MPI_CHAR, &carray_t); 
    ret = MPI_Type_commit(&carray_t);
    if (ret != MPI_SUCCESS) {
	fprintf(stderr, "Could not make char array type."), fflush(stderr); 
	exit(1);
    }

    /* A fairly simple structure */
    disp1[0] = 0;
    disp1[1] = (MPI_Aint)(char *)&dummy1.c1[0] - (MPI_Aint)(char *)&dummy1;
    type1[0] = MPI_DOUBLE;
    type1[1] = carray_t;
    MPI_Type_struct(2, block1, disp1, type1, &struct1_t);
    ret = MPI_Type_commit(&struct1_t);
    if (ret != MPI_SUCCESS) {
	fprintf(stderr, "Could not make struct 1."), fflush(stderr), exit(1);
	exit(1);
    }

    /* And a short array of this type */
    MPI_Type_contiguous(2, struct1_t, &astruct1_t);
    ret = MPI_Type_commit(&astruct1_t);
    if (ret != MPI_SUCCESS) {
	fprintf(stderr, "Could not make struct 1 array."), fflush(stderr);
	exit(1);
    }

    
    /* A more complex structure */
    disp2[0] = 0;                    
    disp2[1] = (MPI_Aint)(char *)&dummy2.c1[0] - (MPI_Aint)(char *)&dummy2;
    disp2[2] = (MPI_Aint)(char *)&dummy2.d3    - (MPI_Aint)(char *)&dummy2; 
    disp2[3] = (MPI_Aint)(char *)&dummy2.c3[0] - (MPI_Aint)(char *)&dummy2;
    disp2[4] = (MPI_Aint)(char *)&dummy2.d4    - (MPI_Aint)(char *)&dummy2; 
    disp2[5] = (MPI_Aint)(char *)&dummy2.c4[0] - (MPI_Aint)(char *)&dummy2;

    type2[0] = MPI_DOUBLE; type2[1] = carray_t; type2[2] = MPI_DOUBLE;
    type2[3] = carray_t; type2[4] = MPI_DOUBLE; type2[5] = carray_t;
    MPI_Type_struct(6, block2, disp2, type2, &struct2_t);
    ret = MPI_Type_commit(&struct2_t);
    if (ret != MPI_SUCCESS) {
	fprintf(stderr, "Could not make struct 2."), fflush(stderr);
	exit(1);
    }

    /* Another (hopefully compatible) complex structure */
    disp3[0] = 0; 
    disp3[1] = (MPI_Aint)(char *)&dummy3.c1[0][0] - (MPI_Aint)(char *)&dummy3; 
    disp3[2] = (MPI_Aint)(char *)&dummy3.s1[0] - (MPI_Aint)(char *)&dummy3;
    
    type3[0] = MPI_DOUBLE; type3[1] = carray_t; type3[2] = astruct1_t;
    MPI_Type_struct(3, block3, disp3, type3, &struct3_t);
    ret = MPI_Type_commit(&struct3_t);
    if (ret != MPI_SUCCESS) {
	fprintf(stderr, "Could not make struct 3."), fflush(stderr);
	exit(1);
    }

    /* Wait for everyone to be ready */
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) { 	

	/* Fill up the type */
	dummy2.d1 = 11.0; dummy2.d2 = 12.0; dummy2.d3 = 13.0; dummy2.d4 = 14.0;
	strncpy(dummy2.c1, "two", 8);
	strncpy(dummy2.c2, "four", 8);
	strncpy(dummy2.c3, "six", 8);
	strncpy(dummy2.c4, "eight", 8);
	
	/* Send the type */
	MPI_Send(&dummy2, 1, struct2_t, 1, 2000, MPI_COMM_WORLD);
	/* Clear out the type */
	memset(&dummy2, 0, sizeof(dummy2));
	/* And receive it back */
	MPI_Recv(&dummy2, 1, struct2_t, 1, 2000, MPI_COMM_WORLD, &Status);
	
	/* Did it make it OK? */
	if ((dummy2.d1 != 11.0) || (dummy2.d2 != 12.0) || 
	    (dummy2.d3 != 13.0) || (dummy2.d4 != 14.0) || 
	    strncmp(dummy2.c1, "two", 8) || strncmp(dummy2.c2, "four", 8) || 
	    strncmp(dummy2.c3, "six", 8) || strncmp(dummy2.c4, "eight", 8)) {
	    Test_Failed("Complex Type Round Trip Test");
	} else {
	    Test_Passed("Complex Type Round Trip Test");
	}


	/* Fill up the type again */
	dummy2.d1 = 11.0; dummy2.d2 = 12.0; dummy2.d3 = 13.0; dummy2.d4 = 14.0;
	strncpy(dummy2.c1, "two", 8);
	strncpy(dummy2.c2, "four", 8);
	strncpy(dummy2.c3, "six", 8);
	strncpy(dummy2.c4, "eight", 8);
	
	/* Send the type */
	MPI_Send(&dummy2, 1, struct2_t, 1, 2000, MPI_COMM_WORLD);
	/* Clear out the type */
	memset(&dummy2, 0, sizeof(dummy2));
	/* And receive it back */
	MPI_Recv(&dummy2, 1, struct2_t, 1, 2000, MPI_COMM_WORLD, &Status);
	
	/* Did it make it OK? */
	if ((dummy2.d1 != 11.0) || (dummy2.d2 != 12.0) || 
	    (dummy2.d3 != 13.0) || (dummy2.d4 != 14.0) || 
	    strncmp(dummy2.c1, "two", 8) || strncmp(dummy2.c2, "four", 8) || 
	    strncmp(dummy2.c3, "six", 8) || strncmp(dummy2.c4, "eight", 8))
	    Test_Failed("Compatible Complex Type Round Trip Test");
	else
	    Test_Passed("Compatible Complex Type Round Trip Test");

	if ((MPI_Type_free(&struct3_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&struct1_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&struct2_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&astruct1_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&carray_t) != MPI_SUCCESS))
	    Test_Failed("Type Free test");
	else
	    Test_Passed("Type Free test");
	
	MPI_Finalize();
    } else {
	MPI_Recv(&dummy2, 1, struct2_t, 0, 2000, MPI_COMM_WORLD, &Status);
	MPI_Send(&dummy2, 1, struct2_t, 0, 2000, MPI_COMM_WORLD);

	MPI_Recv(&dummy3, 1, struct3_t, 0, 2000, MPI_COMM_WORLD, &Status);
	if ((dummy3.d1[0] != 11.0) || (dummy3.d1[1] != 12.0) || 
	    (dummy3.s1[0].d1 != 13.0) || (dummy3.s1[1].d1 != 14.0) || 
	    strncmp(dummy3.c1[0], "two", 8) || 
	    strncmp(dummy3.c1[1], "four", 8) || 
	    strncmp(dummy3.s1[0].c1, "six", 8) || 
	    strncmp(dummy3.s1[1].c1, "eight", 8)) {

	    /* Kill dummy3 so it will die after it's sent back */
	    memset(&dummy3, 0, sizeof(dummy3));
	    Test_Message("Message didn't convert properly. Hosing \
return message.");
	}
	MPI_Send(&dummy3, 1, struct3_t, 0, 2000, MPI_COMM_WORLD);

	if ((MPI_Type_free(&struct3_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&struct1_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&struct2_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&astruct1_t) != MPI_SUCCESS) ||
	    (MPI_Type_free(&carray_t) != MPI_SUCCESS))
	    Test_Failed("Type Free test");
	else
	    Test_Passed("Type Free test");
	
	MPI_Finalize();
    }
    (void)Summarize_Test_Results();
    Test_Finalize();

}



