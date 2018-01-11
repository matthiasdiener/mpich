/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

/* PURPOSE
   =======
   to copy the data input to output pointer
*/

/* INCLUDE FILES */
#include "stdeig.h"
#include "mm.h"

/* CONDITIONAL COMPILATION */
#if PRISM_BLAS_DCOPY
/* use the BLAS routine dcopy to copy full double words.
   If not, then just do with pointers.  dcopy might be faster since it
   is often an assembly routine on machines, but also requires the BLAS
   for linking.
*/
#endif

#if PRISM_NX
void prism_v_copy(void * p_v_input, void * p_v_output, int i_elements,
		  PRISM_E_DATATYPE e_datatype)
#else
void prism_v_copy(void * p_v_input, void * p_v_output, int i_elements,
		  MPI_Datatype x_datatype)
#endif  

/* PARAMETERS
   ==========

   p_v_input: pointer to linear array to copy

   p_v_output: pointer to linear array to copy into (values modified)

   i_elements: # elements to copy

#if PRISM_NX
   e_datatype: PRISM datatype of p_v_send which gives units of
               i_elements (input)
#else
   x_datatype: MPI datatype of p_v_send which gives units of i_elements (input)
#endif
*/

{

  /* --------------- */
  /* Local variables */
  /* --------------- */

  int
    i_bytes, /* length of copy in bytes */
    i_l1,
    i_n
      ;

#if !PRISM_NX
  MPI_Aint
    i_extent
      ;
#endif

#if !PRISM_BLAS_DCOPY
  double
    *p_d_ohold, /* temporary pointer for index calculation on recv pointer */
    *p_d_ihold  /* temporary pointer for index calculation on send pointer */
      ;
#endif

  char
    *p_c_ohold, /* temporary pointer for index calculation on recv pointer */
    *p_c_ihold  /* temporary pointer for index calculation on send pointer */
      ;

  /* ------------------ */
  /* External functions */
  /* ------------------ */
#if PRISM_BLAS_DCOPY
#endif

#if PRISM_NX
  i_bytes = prism_i_datatype_size(e_datatype) * i_elements;
#else
  MPI_Type_extent(x_datatype, &i_extent);
  i_bytes = (int)i_extent * i_elements;
#endif  

  /* copy */
  /* first do for bytes that are complete double words */
  i_n = i_bytes / sizeof(double);
#if PRISM_BLAS_DCOPY
  dcopy_((fint *) &i_n, (double *)p_v_input, (fint *) &i_one, 
	 (double *)p_v_output, (fint *) &i_one);
#else
  p_d_ohold = (double *)p_v_output;
  p_d_ihold = (double *)p_v_input;
  for (i_l1 = 0; i_l1 < i_n; i_l1++) {
    *p_d_ohold = *p_d_ihold;
    p_d_ohold++;
    p_d_ihold++;
  }
#endif
  /* now do the bytes that might be left over */
  p_c_ohold = (char *)p_v_output + i_n * sizeof(double);
  p_c_ihold = (char *)p_v_input + i_n * sizeof(double);
  for (i_l1 = 0; i_l1 < i_bytes - i_n * sizeof(double); i_l1++) {
    *p_c_ohold = *p_c_ihold;
    p_c_ohold++;
    p_c_ihold++;
  }
}

