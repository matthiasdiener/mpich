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
   Subroutine to initalize the message.
   By default, uses sequence of random values.
   If set PRISM_SPECIAL_INIT then set to:
     Set to be 10^12 + 10^8 * i_node + location in array for complete double
     words
   Sets extra bytes at end to char of i_node % 10 + '0'
   See prism_v_chk_msg for routine to verify after receipt
*/

/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"
#if PRISM_SPECIAL_INIT
#include <math.h>
#endif  

/* CONDITIONAL COMPILATION */

#if PRISM_SPECIAL_INIT
/*
Normally use a random number generator with a certain seed to set the
values in the array.  Sometimes, for debugging, it is helpful to use a
more intuitive value.  This sets the value to such a value given the size
of the datatype
*/
#endif

void prism_v_init_msg(void *p_v_msg, int i_len_elements,
		      PRISM_E_DATATYPE e_datatype, int i_node)

/*
  PARAMTERS RECEIVED:
  -------------------

  p_v_msg: pointer to the message to be sent

  i_len_elements: # of elements in p_v_msg

  e_datatype: prism datatype of p_v_msg and what elements means

  i_node: seed for rand generation
          node # or other value desired for PRISM_SPECIAL_INIT */
 
{
  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
    i_junk,
    i_l1 /* loop counter */
      ;

  /* Some systems do not let you do operations on void pointers.
     Thus, create a typed pointer for each pointer type needed */

  char
    *p_c
      ;
  double
    *p_d
      ;
  float
    *p_f
      ;
  int
    *p_i
      ;
  long int
    *p_li
      ;
  long double
    *p_ld
      ;
  short int
    *p_si
      ;
  unsigned int
    *p_ui
      ;
  unsigned long int
    *p_uli
      ;
  unsigned short int
    *p_usi
      ;

  /* external functions */
#if !PRISM_SPECIAL_INIT
  int prism_rand();
  void prism_srand(unsigned int);
#endif

/* initialize msg */

#if !PRISM_SPECIAL_INIT
  /* set seed to use in prism_srand.  use i_node as initial seed and
     then cycle generator i_node times to place somewhere unique for
     each node (hopefully!) */
  prism_srand((unsigned int)i_node);
  for (i_l1 = 0; i_l1 < i_node; i_l1++) {
    i_junk = prism_rand();
  }
#endif

  /* initialize pointer for appropriate type */

    switch (e_datatype) {

    case prism_byte: case prism_char: case prism_unsigned_char:
      p_c = (char *)p_v_msg;
      break;

    case prism_double:
      p_d = (double *)p_v_msg;
      break;

    case prism_float:
      p_f = (float *)p_v_msg;
      break;

    case prism_int:
      p_i = (int *)p_v_msg;
      break;

    case prism_long:
      p_li = (long *)p_v_msg;
      break;

    case prism_long_double:
      p_ld = (long double *)p_v_msg;
      break;

    case prism_short:
      p_si = (short int *)p_v_msg;
      break;

    case prism_unsigned:
      p_ui = (unsigned int *)p_v_msg;
      break;

    case prism_unsigned_long:
      p_uli = (unsigned long int *)p_v_msg;
      break;

    case prism_unsigned_short:
      p_usi = (unsigned short int *)p_v_msg;
      break;
    
    default:
      prism_v_generror("prism_v_init_msg: PRISM datatype received not known",
		       brief);
    }

  for (i_l1 = 0; i_l1 < i_len_elements; i_l1++) {
#if PRISM_SPECIAL_INIT
    i_junk = (int)(i_node * pow(10,4) + i_l1);
#else
    i_junk = prism_rand();
#endif

    switch (e_datatype) {

    case prism_byte: case prism_char: case prism_unsigned_char:
      *p_c = (char)(i_junk & 0377);
      p_c++;
      break;

    case prism_double:
      *p_d = (double)(-i_junk);
      p_d++;
      break;

    case prism_float:
      *p_f = (float)(-i_junk);
      p_f++;
      break;

    case prism_int:
      *p_i = (int)(-i_junk);
      p_i++;
      break;

    case prism_long:
      *p_li = (long int)(-i_junk);
      p_li++;
      break;

    case prism_long_double:
      *p_ld = (long double)(-i_junk);
      p_ld++;
      break;

    case prism_short:
      *p_si = (short int)(-i_junk);
      p_si++;
      break;

    case prism_unsigned:
      *p_ui = (unsigned int)(i_junk);
      p_ui++;
      break;

    case prism_unsigned_long:
      *p_uli = (unsigned long int)(i_junk);
      p_uli++;
      break;

    case prism_unsigned_short:
      *p_usi = (unsigned short int)(i_junk);
      p_usi++;
      break;
    
    default:
      prism_v_generror("prism_v_init_msg: PRISM datatype received not known",
		       brief);
    }
  }
}
