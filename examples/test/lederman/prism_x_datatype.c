/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   PRISM routine (version 2.1)

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

/* PURPOSE
   =======
   This routine returns the MPI datatype corresponding to a PRISM
   datatype to use for an MPI call.
*/

#if !PRISM_NX
/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"

MPI_Datatype prism_x_datatype(PRISM_E_DATATYPE e_datatype)

     /* PARAMETERS
	==========

	e_datatype: the PRISM datatype that you want the corresponding
	            MPI datatype
*/

{
  switch (e_datatype) {

  case prism_byte:
    return MPI_BYTE;

  case prism_char:
    return MPI_CHAR;

  case prism_double:
    return MPI_DOUBLE;

  case prism_float:
    return MPI_FLOAT;

  case prism_int:
    return MPI_INT;

  case prism_long:
    return MPI_LONG;

  case prism_long_double:
    return MPI_LONG_DOUBLE;

  case prism_short:
    return MPI_SHORT;

  case prism_unsigned:
    return MPI_UNSIGNED;

  case prism_unsigned_char:
    return MPI_UNSIGNED_CHAR;

  case prism_unsigned_long:
    return MPI_UNSIGNED_LONG;

  case prism_unsigned_short:
    return MPI_UNSIGNED_SHORT;
    
  default:
    prism_v_generror("prism_x_datatype: PRISM datatype received not known",
		     brief);
  } /* end switch */
}

#endif /* on !PRISM_NX */
