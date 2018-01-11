#ifndef _MPIR_DMPI_INC
#define _MPIR_DMPI_INC

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#ifdef MPI_ADI2
void MPID_BSwap_N_inplace ANSI_ARGS(( unsigned char *, int, int ));
void MPID_BSwap_short_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_int_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_long_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_float_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_double_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_long_double_inplace ANSI_ARGS(( unsigned char *, int ));
void MPID_BSwap_N_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				   int, int ));
void MPID_BSwap_short_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				       int ));
void MPID_BSwap_int_copy ANSI_ARGS(( unsigned char *, unsigned char *, int ));
void MPID_BSwap_long_copy ANSI_ARGS(( unsigned char *, unsigned char *, int ));
void MPID_BSwap_float_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				       int ));
void MPID_BSwap_double_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
					int ));
void MPID_BSwap_long_double_copy ANSI_ARGS(( unsigned char *, 
					     unsigned char *, int ));

int MPID_Type_swap_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				    MPI_Datatype, int, void * ));
void MPID_Type_swap_inplace ANSI_ARGS(( unsigned char *, MPI_Datatype, int ));
int MPID_Type_XDR_encode ANSI_ARGS(( unsigned char *, unsigned char *, 
				     MPI_Datatype, int, void * ));
int MPID_Type_XDR_decode ANSI_ARGS(( unsigned char *, int, MPI_Datatype, int, 
				     unsigned char *, int, int *, int *, 
				     void * ));
int MPID_Type_convert_copy ANSI_ARGS(( MPI_Comm, void *, int, void *, 
				       MPI_Datatype, int, int, int * ));
#ifdef MPI_ADI2
int MPID_Mem_convert_len ANSI_ARGS(( MPID_Msgrep_t, MPI_Datatype, int ));
#else
int MPID_Mem_convert_len ANSI_ARGS(( int, MPI_Datatype, int ));
#endif
int MPID_Mem_XDR_Len ANSI_ARGS(( MPI_Datatype, int ));
#else
void MPIR_BSwap_N_inplace ANSI_ARGS(( unsigned char *, int, int ));
void MPIR_BSwap_short_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_int_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_long_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_float_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_double_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_long_double_inplace ANSI_ARGS(( unsigned char *, int ));
void MPIR_BSwap_N_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				   int, int ));
void MPIR_BSwap_short_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				       int ));
void MPIR_BSwap_int_copy ANSI_ARGS(( unsigned char *, unsigned char *, int ));
void MPIR_BSwap_long_copy ANSI_ARGS(( unsigned char *, unsigned char *, int ));
void MPIR_BSwap_float_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				       int ));
void MPIR_BSwap_double_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
					int ));
void MPIR_BSwap_long_double_copy ANSI_ARGS(( unsigned char *, 
					     unsigned char *, int ));

int MPIR_Type_swap_copy ANSI_ARGS(( unsigned char *, unsigned char *, 
				    MPI_Datatype, int, void * ));
void MPIR_Type_swap_inplace ANSI_ARGS(( unsigned char *, MPI_Datatype, int ));
int MPIR_Type_XDR_encode ANSI_ARGS(( unsigned char *, unsigned char *, 
				     MPI_Datatype, int, void * ));
int MPIR_Type_XDR_decode ANSI_ARGS(( unsigned char *, int, MPI_Datatype, int, 
				     unsigned char *, int, int *, int *, 
				     void * ));
int MPIR_Type_convert_copy ANSI_ARGS(( MPI_Comm, void *, int, void *, 
				       MPI_Datatype, int, int, int * ));
int MPIR_Mem_convert_len ANSI_ARGS(( int, MPI_Datatype, int ));
int MPIR_Mem_XDR_Len ANSI_ARGS(( MPI_Datatype, int ));
#endif

int MPIR_Comm_needs_conversion ANSI_ARGS(( MPI_Comm ));
int MPIR_Dest_needs_converstion ANSI_ARGS(( int ));
void MPIR_Pack_Hvector ANSI_ARGS(( MPI_Comm, char *, int, MPI_Datatype, 
				   int, char * ));
void MPIR_UnPack_Hvector ANSI_ARGS(( char *, int, MPI_Datatype, int, char * ));
int MPIR_HvectorLen ANSI_ARGS(( int, MPI_Datatype ));
int MPIR_PackMessage ANSI_ARGS(( char *, int, MPI_Datatype, int, int, 
				 MPI_Request ));
int MPIR_EndPackMessage ANSI_ARGS(( MPI_Request ));
int MPIR_SetupUnPackMessage ANSI_ARGS(( char *, int, MPI_Datatype, int, 
					MPI_Request ));
int MPIR_Receive_setup ANSI_ARGS(( MPI_Request * ));
int MPIR_Send_setup ANSI_ARGS(( MPI_Request * ));
int MPIR_SendBufferFree ANSI_ARGS(( MPI_Request ));

int MPIR_Elementcnt ANSI_ARGS(( unsigned char *, int, MPI_Datatype, int, 
		     unsigned char *, int, int *, int *, void * ));
void DMPI_msg_arrived ANSI_ARGS(( int, int, MPIR_CONTEXT, MPIR_RHANDLE **, 
				  int * ));
void DMPI_free_unexpected ANSI_ARGS(( MPIR_RHANDLE * ));

/*
int MPIR_Pack  ANSI_ARGS(( MPI_Comm, int, void *, int, MPI_Datatype, 
			   void *, int, int * ));
int MPIR_Pack_size ANSI_ARGS(( int, MPI_Datatype, MPI_Comm, int, int * ));
*/
int MPIR_Pack2 ANSI_ARGS(( char *, int, MPI_Datatype, 
		int  (*) (unsigned char *, unsigned char *, 
				     MPI_Datatype, int, 
				     void *), void *, char *, int *, int * ));
int MPIR_Unpack2 ANSI_ARGS(( char *, int, MPI_Datatype, 
		   int  (*)(unsigned char *, int, MPI_Datatype, int,
			   unsigned char *, int, int *, int *, void *),
		   void *, char *, int, int *, int * ));
#ifdef MPI_ADI2
int MPIR_Unpack ANSI_ARGS(( MPI_Comm, void *, int, int, MPI_Datatype, 
			    MPID_Msgrep_t, 
			    void *, int *, int * ));
#else
int MPIR_Unpack ANSI_ARGS(( MPI_Comm, void *, int, int, MPI_Datatype, int, 
			    void *, int *, int * ));
#endif

int MPIR_Printcontig ANSI_ARGS(( unsigned char *, unsigned char *, 
				 MPI_Datatype, int, void *));
int MPIR_Printcontig2 ANSI_ARGS(( char *, int, MPI_Datatype, int, char *, 
				  void * ));
int MPIR_Printcontig2a ANSI_ARGS(( unsigned char *, int, MPI_Datatype, int, 
			unsigned char *, int, int *, int *, void * ));

#ifdef MPID_INCLUDE_STDIO
int MPIR_PrintDatatypePack ANSI_ARGS(( FILE *, int, MPI_Datatype, int, int ));
int MPIR_PrintDatatypeUnpack ANSI_ARGS(( FILE *, int, MPI_Datatype, 
					 int, int ));
#endif

#endif
