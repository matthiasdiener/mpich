/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"

int prism_i_datatype_size(PRISM_E_DATATYPE e_datatype)

     /* 
	COPYRIGHT U.S. GOVERNMENT 
   
	This software is distributed without charge and comes with no
	warranty.

	Please feel free to send questions, comments, and problem
	reports to prism@super.org.
	*/

     /* PURPOSE
	=======
	This routine decodes the PRISM datatype received.  It returns
	the number of bytes in the datatype.
	*/

     /* PARAMETERS
	==========

	e_datatype: the PRISM datatype that you want the sizeof in
	bytes returned.
	*/
{
  switch (e_datatype) {

  case prism_byte:
    return (int)1;

  case prism_char:
    return (int)sizeof(signed char);

  case prism_double:
    return (int)sizeof(double);

  case prism_float:
    return (int)sizeof(float);

  case prism_int:
    return (int)sizeof(signed int);

  case prism_long:
    return (int)sizeof(signed long int);

  case prism_long_double:
    return (int)sizeof(long double);

  case prism_short:
    return (int)sizeof(short int);

  case prism_unsigned:
    return (int)sizeof(unsigned int);

  case prism_unsigned_char:
    return (int)sizeof(unsigned char);

  case prism_unsigned_long:
    return (int)sizeof(unsigned long int);

  case prism_unsigned_short:
    return (int)sizeof(unsigned short int);
    
  default:
    prism_v_generror("prism_i_datatype_size: PRISM datatype received not known",
		     brief);
  } /* end switch */
}
