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
  Subroutine to check message received for values set by prism_v_init_msg
  Since don't expect errors, generates one message per word (byte) 
  and continues
  See prism_v_init_msg for how initialized
*/

/* INCLUDE FILES */
#include "stdeig.h"
#include <stdio.h>
#include "mm.h"
#if PRISM_SPECIAL_INIT
#include <math.h>
#endif  

void prism_v_chk_msg(void *p_v_msg, int i_len_elements,
		     PRISM_E_DATATYPE e_datatype, int i_node, FILE *outfile)

/*
  PARAMTERS RECEIVED:
  -------------------

  p_v_msg: pointer to the message received

  i_len_elements: # of bytes in array (as linear)

  e_datatype: prism datatype of p_v_msg and what elements means

  i_node: node # who sent or other value desired

  outfile: file pointer for messages
*/
 
{
  /* --------------- */
  /* Local variables */
  /* --------------- */
  int
    i_world_id, /* id in MPI_COMM_WORLD */
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

  E_BOOLEAN
    e_error
      ;

/* check msg */
  e_error = false;

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
      if (*p_c != (char)(i_junk & 0377)) {
	e_error = true;
	/* print as int so won't get funny characters */
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(char *)p_v_msg, (i_junk & 0377));
      }
      p_c++;
      break;

    case prism_double:
      if (*p_d != (double)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %f but expect %f\n", i_l1, *(double *)p_v_msg, (double)(-i_junk));
      }
      p_d++;
      break;

    case prism_float:
      if (*p_f != (float)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %f but expect %f\n", i_l1, *(float *)p_v_msg, (float)(-i_junk));
      }
      p_f++;
      break;

    case prism_int:
      if (*p_i != (int)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(int *)p_v_msg, (int)(-i_junk));
      }
      p_i++;
      break;

    case prism_long:
      if (*p_li != (long)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(long *)p_v_msg, (long)(-i_junk));
      }
      p_li++;
      break;

    case prism_long_double:
      if (*p_ld != (long double)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %f but expect %f\n", i_l1, *(long double *)p_v_msg, (long double)(-i_junk));
      }
      p_ld++;
      break;

    case prism_short:
      if (*p_si != (short int)(-i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(short int *)p_v_msg, (short int)(-i_junk));
      }
      p_si++;
      break;

    case prism_unsigned:
      if (*p_ui != (unsigned int)(i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(unsigned int *)p_v_msg, (unsigned int)(i_junk));
      }
      p_ui++;
      break;

    case prism_unsigned_long:
      if (*p_uli != (unsigned long int)(i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1,
		*(unsigned long int *)p_v_msg, (unsigned long int)(i_junk));
      }
      p_uli++;
      break;

    case prism_unsigned_short:
      if (*p_usi != (unsigned short int)(i_junk)) {
	e_error = true;
	fprintf(outfile, "***error***prism_v_chk_msg: msg at loc = %d with value %d but expect %d\n", i_l1, *(unsigned short int *)p_v_msg,
		(unsigned short int)(i_junk));
      }
      p_usi++;
      break;
    
    default:
      prism_v_generror("prism_v_chk_msg: PRISM datatype received not known",
		       brief);
    }
  }

  if (e_error == true) {
#if PRISM_NX
    i_world_id = prism_i_nx_id;
#else
    MPI_Comm_rank(MPI_COMM_WORLD, &i_world_id);
#endif
    fprintf(stderr, "***error***prism_v_chk_msg: world  node = %d has errors in received values - see output file\n", i_world_id);
  }
}
