#ifndef _MPI_PT2PT
#define _MPI_PT2PT

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#if defined(__STDC__) || defined(__cplusplus)
/* This SAME test must be used in pt2pt/mperror.c */
#define MPIR_USE_STDARG
#endif

int MPIR_Pack  ANSI_ARGS(( MPI_Comm, int, void *, int, MPI_Datatype, 
			   void *, int, int *));
int MPIR_Pack_size  ANSI_ARGS(( int, MPI_Datatype, MPI_Comm, int, int *));
#ifdef MPI_ADI2
int MPIR_Unpack ANSI_ARGS(( MPI_Comm, void *, int, int, MPI_Datatype, 
			    MPID_Msgrep_t, void *, int *, int * ));
#else
int MPIR_Unpack ANSI_ARGS(( MPI_Comm, void *, int, int, MPI_Datatype, 
			    int, void *, int *, int * ));
#endif
int MPIR_UnPackMessage ANSI_ARGS(( char *, int, MPI_Datatype, int, 
				   MPI_Request, int * ));
int MPIR_Type_free ANSI_ARGS(( MPI_Datatype * ));
void MPIR_Type_free_struct ANSI_ARGS(( MPI_Datatype ));
MPI_Datatype MPIR_Type_dup ANSI_ARGS(( MPI_Datatype ));
int MPIR_Type_permanent ANSI_ARGS(( MPI_Datatype ));
void MPIR_Free_perm_type ANSI_ARGS(( MPI_Datatype * ));
void MPIR_Type_get_limits ANSI_ARGS(( MPI_Datatype, MPI_Aint *, MPI_Aint *));
#ifndef MPI_ADI2
int MPIR_Send_init ANSI_ARGS(( void *, int, MPI_Datatype, int, int, 
			       MPI_Comm, MPI_Request, MPIR_Mode, int ));
int MPIR_Recv_init ANSI_ARGS(( void *, int, MPI_Datatype, int, int, 
			       MPI_Comm, MPI_Request, int ));
#endif
extern MPI_Handler_function MPIR_Errors_are_fatal;
extern MPI_Handler_function MPIR_Errors_return;
extern MPI_Handler_function MPIR_Errors_warn;
/* Since these are declared as handler functions, we do not
   redeclare them */
#ifdef MPIR_USE_STDARG
/* gcc requires an explicit declaration when checking for strict prototypes */
void MPIR_Errors_are_fatal ANSI_ARGS(( MPI_Comm *, int *, ... ));
void MPIR_Errors_return ANSI_ARGS(( MPI_Comm *, int *, ... ));
void MPIR_Errors_warn ANSI_ARGS(( MPI_Comm *, int *, ... ));
#else
#ifdef FOO
/* Otherwise, just accept the typedef declaration */
void MPIR_Errors_are_fatal ANSI_ARGS(( MPI_Comm *, int *, char *, 
				      char *, int *));
void MPIR_Errors_return ANSI_ARGS(( MPI_Comm *, int *, char *, char *, int *));
void MPIR_Errors_warn ANSI_ARGS(( MPI_Comm *, int *, char *, char *, int *));
#endif
#endif /* MPIR_USE_STDARG */

#endif
