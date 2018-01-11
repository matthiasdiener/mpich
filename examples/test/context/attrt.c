/*

  Exercise communicator routines.

  This C version derived from a Fortran test program from ....

 */
#include <stdio.h>
#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
MPI_Init( &argc, &argv );
test_communicators();
MPI_Finalize();
return 0;
}

int copy_fn(oldcomm, keyval, extra_state,
                      attribute_val_in, attribute_val_out, flag)
MPI_Comm *oldcomm;
int *keyval;
void *extra_state, *attribute_val_in, **attribute_val_out;
int *flag;
{
*(int *)attribute_val_out = (MPI_Aint)attribute_val_in;
*flag = 1;
return MPI_SUCCESS;
}

int delete_fn(comm, keyval, attribute_val, extra_state)
MPI_Comm *comm;
int *keyval;
void *attribute_val, *extra_state;
{
int world_rank;
MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
if ((MPI_Aint)attribute_val != (MPI_Aint)world_rank) {
    printf( "incorrect attribute value %d\n", *(int*)attribute_val );
    MPI_Abort(MPI_COMM_WORLD, 1005 );
    }
return MPI_SUCCESS;
}

int test_communicators()
{
MPI_Comm dup_comm_world, lo_comm, rev_comm, dup_comm, split_comm, world_comm;
MPI_Group world_group, lo_group, rev_group;
int ranges[1][3];
int flag, world_rank, world_size, rank, size, n, key_1, key_3, value;

/*      integer n, result,
     .        key_2
     .        color, key

  */

MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
MPI_Comm_size( MPI_COMM_WORLD, &world_size );
if (world_rank == 0) {
    printf( "*** Communicators ***\n" );
    }

MPI_Comm_dup( MPI_COMM_WORLD, &dup_comm_world );

/*
     Exercise Comm_create by creating an equivalent to dup_comm_world
     (sans attributes) and a half-world communicator.
 */

if (world_rank == 0) 
    printf( "    Comm_create\n" );

MPI_Comm_group( dup_comm_world, &world_group );
MPI_Comm_create( dup_comm_world, world_group, &world_comm );
MPI_Comm_rank( world_comm, &rank );
if (rank != world_rank) {
    printf( "incorrect rank in world comm: %d\n", rank );
    MPI_Abort(MPI_COMM_WORLD, 3001 );
    }

n = world_size / 2;

ranges[0][0] = 0;
ranges[0][1] = (world_size - n) - 1;
ranges[0][2] = 1;

MPI_Group_range_incl(world_group, 1, ranges, &lo_group );
MPI_Comm_create(world_comm, lo_group, &lo_comm );
MPI_Group_free( &lo_group );

if (world_rank < (world_size - n)) {
    MPI_Comm_rank(lo_comm, &rank );
    if (rank == MPI_UNDEFINED) {
	printf( "incorrect lo group rank: %d\n", rank );
	MPI_Abort(MPI_COMM_WORLD, 3002 );
	}
    else {
	MPI_Barrier(lo_comm );
	}
    }
else {
    if (lo_comm != MPI_COMM_NULL) {
	printf( "incorrect lo comm:\n" );
	MPI_Abort(MPI_COMM_WORLD, 3003 );
	}
    }
      
MPI_Barrier(world_comm );
/*
     Check Comm_dup by adding attributes to lo_comm & duplicating
 */
if (world_rank == 0) 
    printf( "    Comm_dup\n" );

if (lo_comm != MPI_COMM_NULL) {
    value = 9;
    MPI_Keyval_create(copy_fn,     delete_fn,   &key_1, &value );
    value = 8;
/*     MPI_Keyval_create(MPI_DUP_FN,  MPI_NULL_DELETE_FN,
                              &key_2, &value );  */
    value = 7;
    MPI_Keyval_create(MPI_NULL_COPY_FN, MPI_NULL_DELETE_FN,
                               &key_3, &value ); 

    /* This may generate a compilation warning; it is, however, an
       easy way to cache a value instead of a pointer */
    MPI_Attr_put(lo_comm, key_1, (void *)world_rank );
/*         MPI_Attr_put(lo_comm, key_2, world_size ) */
    MPI_Attr_put(lo_comm, key_3, (void *)0 );

    MPI_Comm_dup(lo_comm, &dup_comm );

    MPI_Attr_get(dup_comm, key_1, (void **)&value, &flag );

    if (! flag) {
	printf( "dup_comm key_1 not found on %d\n", world_rank );
	MPI_Abort(MPI_COMM_WORLD, 3004 );
	}

    if (value != world_rank) {
	printf( "dup_comm key_1 value incorrect: %d\n", value );
	MPI_Abort(MPI_COMM_WORLD, 3005 );
	}

/*         MPI_Attr_get(dup_comm, key_2, (int *)&value, &flag ); */
/*
        if (! flag) {
           printf( "dup_comm key_2 not found\n" );
           MPI_Abort(MPI_COMM_WORLD, 3006 );
           }

       if (value != world_size) {
           printf( "dup_comm key_2 value incorrect: %d\n", value );
           MPI_Abort(MPI_COMM_WORLD, 3007 );
	   }
 */
    MPI_Attr_get(dup_comm, key_3, (void **)&value, &flag );
    if (flag) {
        printf( "dup_comm key_3 found!\n" );
	MPI_Abort(MPI_COMM_WORLD, 3008 );
	}
    MPI_Keyval_free(&key_1 );
/* 
c        MPI_Keyval_free(&key_2 )
 */
    MPI_Keyval_free(&key_3 );
    }
#ifdef FOO
/* 
     Split the world into even & odd communicators with reversed ranks.
 */
      if (world_rank == 0) then
         print *, '    Comm_split'
         end if

      color = MOD(world_rank, 2)
      key   = world_size - world_rank

      MPI_Comm_split(dup_comm_world, color, key, split_comm )  
      MPI_Comm_size(split_comm, size )
      MPI_Comm_rank(split_comm, rank )
      if (rank .ne. ((size - world_rank/2) - 1)) then
         print *, 'incorrect split rank: ', rank
         MPI_Abort(MPI_COMM_WORLD, 3009 )
         end if

      MPI_Barrier(split_comm )
c
c     Test each possible Comm_compare result
c
c     if (world_rank == 0) then
c        print *, '    Comm_compare'
c        end if

c     MPI_Comm_compare(world_comm, world_comm, result )
c     if (result .ne. MPI_IDENT) then
c        print *, 'incorrect ident result: ', result
c        MPI_Abort(MPI_COMM_WORLD, 3010 )
c        end if

c     if (lo_comm .ne. MPI_COMM_NULL) then
c        MPI_Comm_compare(lo_comm, dup_comm, result )
c        if (result .ne. MPI_CONGRUENT) then
c           print *, 'incorrect congruent result: ', result
c           MPI_Abort(MPI_COMM_WORLD, 3011 )
c           end if
c        end if

c     ranges(1,1) = world_size - 1
c     ranges(2,1) = 0
c     ranges(3,1) = -1

c     MPI_Group_range_incl(world_group, 1, ranges, rev_group )  
c     MPI_Comm_create(world_comm, rev_group, rev_comm )
c     MPI_Comm_compare(world_comm, rev_comm, result )
c     if (result .ne. MPI_SIMILAR) then
c        print *, 'incorrect similar result: ', result
c        MPI_Abort(MPI_COMM_WORLD, 3012 )
c        end if

c     MPI_Comm_compare(world_comm, lo_comm, result )
c     if (result .ne. MPI_UNEQUAL) then
c        print *, 'incorrect unequal result: ', result
c        MPI_Abort(MPI_COMM_WORLD, 3013 )
c        end if
c
c     Free all communicators created
c
#endif
    if (world_rank == 0) 
	printf( "    Comm_free\n" );

MPI_Comm_free(&world_comm );
/* c     MPI_Comm_free(rev_comm ) */
/*       MPI_Comm_free(split_comm ) */

MPI_Group_free(&world_group );
/* 
c     MPI_Group_free(rev_group )

c     if (lo_comm .ne. MPI_COMM_NULL) then
c        MPI_Comm_free(lo_comm )
c        MPI_Comm_free(dup_comm )
c        end if
 */
}

