#include <stdio.h>
#include "mpi.h"

/* 
   Test the error handers (based on a Fortran test program)
 */
int main(argc, argv)
int argc;
char **argv;
{
int    err = 0;
int host, tagub, hasio;
void *v;
int  flag;
int  vval;
int  rank, size;

MPI_Init( &argc, &argv );

Test_errorhandling();

MPI_Finalize();
return 0;
}

static int a_errors, b_errors;

int Test_errorhandling()
{
char errstring[MPI_MAX_ERROR_STRING];
MPI_Comm dup_comm_world, dummy;
MPI_Errhandler errhandler_a, errhandler_b, errhandler, old_handler;
void handler_a(), handler_b(), error_handler();
int  err, world_rank, class, resultlen;

#ifdef FOO
      logical test_default, test_abort
#endif

MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
MPI_Comm_dup( MPI_COMM_WORLD, &dup_comm_world );

if (world_rank == 0) {
    printf( "*** Error Handling ***\n" );
    }

/*
     Exercise save/restore of user error handlers.
 */
a_errors = 0;
MPI_Errhandler_create(handler_a, &errhandler_a);

MPI_Errhandler_set(dup_comm_world, errhandler_a);
MPI_Errhandler_free(&errhandler_a);

MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
if (a_errors != 1) {
    printf( "    error handler A not invoked\n" );
    }
b_errors = 0;
MPI_Errhandler_create(handler_b, &errhandler_b);
MPI_Errhandler_get(dup_comm_world, &old_handler);
MPI_Errhandler_set(dup_comm_world, errhandler_b);
MPI_Errhandler_free(&errhandler_b);
MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
if (b_errors != 1) {
    printf( "    error handler B not invoked\n" );
    }

MPI_Errhandler_set(dup_comm_world, old_handler);
MPI_Errhandler_free(&old_handler);
MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
if (a_errors != 2) {
    printf( "    error handler A not re-invoked\n" );
    }
/*
      Exercise class & string interrogation.
 */
MPI_Errhandler_set(dup_comm_world, MPI_ERRORS_ARE_FATAL);

printf( "    Three error messages (from two errors) are expected\n\
which should both show an error class of %d\n", MPI_ERR_GROUP );

MPI_Errhandler_set(dup_comm_world, MPI_ERRORS_RETURN);
err = MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
if (err != MPI_SUCCESS) {
   MPI_Error_class(err, &class);
   MPI_Error_string(err, errstring, &resultlen);
   printf( "(first) %d : %s\n", class, errstring );
   }

MPI_Errhandler_create(error_handler, &errhandler);
MPI_Errhandler_set(dup_comm_world, errhandler);
MPI_Errhandler_free(&errhandler);
err = MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
if (err != MPI_SUCCESS) {
   MPI_Error_class(err, &class);
   MPI_Error_string(err, errstring, &resultlen);
   printf( "(second) %d : %s\n", class, errstring );
   }

MPI_Errhandler_set(dup_comm_world, MPI_ERRORS_ARE_FATAL);

#ifdef FOO
if (test_default) then
   print *, 'Forcing error for default handler...'
   MPI_Comm_create(dup_comm_world,
                        MPI_GROUP_NULL, dummy) 
   end if

if (test_abort) then
         print *, 'Calling MPI_Abort...'
         MPI_Abort(MPI_COMM_WORLD, 123456768)
         end if
#endif
}


/*

  Trivial error handler.  Note that FORTRAN error handlers can't
  deal with the varargs stuff the C handlers can.
 
 */
void error_handler(comm, err)
MPI_Comm *comm;
int *err;
{
int class;
int resultlen;
char string[MPI_MAX_ERROR_STRING];

MPI_Error_class(*err, &class);
MPI_Error_string(*err, string, &resultlen);
printf( "(errhandler) %d : %s\n", class, string );
}
/* 
  Error handler A, used for save/restore testing.
 */

void handler_a(comm, err)
MPI_Comm *comm; 
int      *err;
{
int class;

MPI_Error_class(*err, &class);
if (class != MPI_ERR_GROUP) {
    printf( "handler_a: incorrect error class %d\n", class );
    }
*err = MPI_SUCCESS;
a_errors++;
}

/* 
  Error handler B, used for save/restore testing.
 */

void handler_b(comm, err)
MPI_Comm *comm;
int      *err;
{
int class;

MPI_Error_class(*err, &class);
if (class != MPI_ERR_GROUP) {
    printf( "handler_b: incorrect error class %d\n", class );
    }
*err = MPI_SUCCESS;
b_errors++;
}
