#include <stdio.h>
/*
 * Since the Cray will generate HUGE numbers, we want all generated numbers to
 * be in a range that all architectures can handle, so copy /usr/include/float.h
 * and /usr/include/machine/limits.h from the FreeBSD collection.
 */

/*    
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *    
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of 
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software  
 *    without specific prior written permission.
 *   
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *    
 *      from: @(#)float.h       7.1 (Berkeley) 5/8/90
 *      $Id: generate.c,v 1.3 1996/10/07 04:16:30 tuecke Exp $
 */   
 
#ifndef _MACHINE_FLOAT_H_
#define _MACHINE_FLOAT_H_ 1
 
#define FLT_RADIX       2               /* b */ 
#define FLT_ROUNDS      1               /* FP addition rounds to nearest */
 
#define FLT_MANT_DIG    24              /* p */
#define FLT_EPSILON     1.19209290E-07F /* b**(1-p) */
#define FLT_DIG         6               /* floor((p-1)*log10(b))+(b == 10) */
#define FLT_MIN_EXP     (-125)          /* emin */
#define FLT_MIN         1.17549435E-38F /* b**(emin-1) */ 
#define FLT_MIN_10_EXP  (-37)           /* ceil(log10(b**(emin-1))) */  
#define FLT_MAX_EXP     128             /* emax */
#define FLT_MAX         3.40282347E+38F /* (1-b**(-p))*b**emax */ 
#define FLT_MAX_10_EXP  38              /* floor(log10((1-b**(-p))*b**emax)) */
 
#define DBL_MANT_DIG    53
#define DBL_EPSILON     2.2204460492503131E-16 
#define DBL_DIG         15 
#define DBL_MIN_EXP     (-1021)
#define DBL_MIN         2.2250738585072014E-308
#define DBL_MIN_10_EXP  (-307)
#define DBL_MAX_EXP     1024
#define DBL_MAX         1.7976931348623157E+308
#define DBL_MAX_10_EXP  308             

#define LDBL_MANT_DIG   DBL_MANT_DIG
#define LDBL_EPSILON    DBL_EPSILON     
#define LDBL_DIG        DBL_DIG
#define LDBL_MIN_EXP    DBL_MIN_EXP
#define LDBL_MIN        DBL_MIN
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#define LDBL_MAX_EXP    DBL_MAX_EXP
#define LDBL_MAX        DBL_MAX
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP
#endif /* _MACHINE_FLOAT_H_ */

#ifndef _MACHINE_LIMITS_H_
#define _MACHINE_LIMITS_H_
     
#define CHAR_BIT        8               /* number of bits in a char */
#define MB_LEN_MAX      6               /* Allow 31 bit UTF2 */
     
/*   
 * According to ANSI (section 2.2.4.2), the values below must be usable by
 * #if preprocessing directives.  Additionally, the expression must have the
 * same type as would an expression that is an object of the corresponding
 * type converted according to the integral promotions.  The subtraction for
 * INT_MIN and LONG_MIN is so the value is not unsigned; 2147483648 is an
 * unsigned int for 32-bit two's complement ANSI compilers (section 3.1.3.2).
 * These numbers work for pcc as well.  The UINT_MAX and ULONG_MAX values
 * are written as hex so that GCC will be quiet about large integer constants.  
 */  
#define SCHAR_MAX       127             /* min value for a signed char */
#define SCHAR_MIN       (-128)          /* max value for a signed char */
 
#define UCHAR_MAX       255             /* max value for an unsigned char */
#define CHAR_MAX        127             /* max value for a char */
#define CHAR_MIN        (-128)          /* min value for a char */

#define USHRT_MAX       65535           /* max value for an unsigned short */
#define SHRT_MAX        32767           /* max value for a short */ 
#define SHRT_MIN        (-32768)        /* min value for a short */
     
#define UINT_MAX        0xffffffff      /* max value for an unsigned int */
#define INT_MAX         2147483647      /* max value for an int */
#define INT_MIN         (-2147483647-1) /* min value for an int */
 
#define ULONG_MAX       0xffffffff      /* max value for an unsigned long */
#define LONG_MAX        2147483647      /* max value for a long */
#define LONG_MIN        (-2147483647-1) /* min value for a long */
 
#if !defined(_ANSI_SOURCE)
#define SSIZE_MAX       INT_MAX         /* max value for a ssize_t */

#if !defined(_POSIX_SOURCE)
#define SIZE_T_MAX      UINT_MAX        /* max value for a size_t */

/* GCC requires that quad constants be written as expressions. */
#define UQUAD_MAX       ((u_quad_t)0-1) /* max value for a uquad_t */
                                        /* max value for a quad_t */
#define QUAD_MAX        ((quad_t)(UQUAD_MAX >> 1))
#define QUAD_MIN        (-QUAD_MAX-1)   /* min value for a quad_t */

#endif /* !_POSIX_SOURCE */
#endif /* !_ANSI_SOURCE */

#endif /* !_MACHINE_LIMITS_H_ */

/* end of copies from FreeBSD */

#include "nexus_dc.h"

#define MIN(a, b) \
    ((a) < (b) ? (a) : (b))

static void create_files(FILE *outfile,
			 FILE *comparefile,
			 int num_elements);

int main(int argc, char **argv)
{
    FILE *outfile;
    FILE *comparefile;
    int num_elements = 0;

    if (argc != 3 && argc != 4)
    {
	printf("Usage:\n");
	printf("    %s <generate_file> <compare_file> [<num_elements>]\n", argv[0]);
	printf("where <generate_file> is the location of the file to be generated\n");
	printf("<compare_file> is the location of the compare file\n");
	printf("and [optionally] <num_elements> is the number of elements\n");
	printf("   per type to generate.\n");
	return 1;
    }

    outfile = fopen(argv[1], "w");
    if (!outfile)
    {
	printf("Error opening %s for writing\n", argv[1]);
	return 2;
    }

    comparefile = fopen(argv[2], "w");
    if (!comparefile)
    {
	printf("Error opening %s for writing\n", argv[2]);
	return 3;
    }

    if (argc == 4)
    {
	num_elements = atoi(argv[3]);
    }

    create_files(outfile, comparefile, num_elements);

    fclose(outfile);
    fclose(comparefile);
}

static void create_files(FILE *outfile,
			 FILE *comparefile,
			 int num_elements)
{
    int tmp;
    int count;
    long i;
    float k;
    double j;
    long linc;
    float finc;
    double dinc;
    unsigned char *ptr = (unsigned char *)&i;

    if (num_elements)
    {
	tmp = MIN(UCHAR_MAX, num_elements);
    }
    else
    {
        tmp = UCHAR_MAX;
    }
    linc = UCHAR_MAX / tmp;
    count = UCHAR_MAX / linc + 1;
    fprintf(outfile, "%d %d %d",
        1, count, count * sizeof(unsigned char));
    for (i = 0; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
	fprintf(outfile, " %d", ptr[sizeof(long)-1]);
#else
	fprintf(outfile, " %d", ptr[0]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");


    if (num_elements)
    {
	tmp = MIN(CHAR_MAX - CHAR_MIN, num_elements);
    }
    else
    {
        tmp = CHAR_MAX - CHAR_MIN;
    }
    linc = (CHAR_MAX - CHAR_MIN) / tmp;
    count = (CHAR_MAX - CHAR_MIN) / linc + 1;
    fprintf(outfile, "%d %d %d", 2, count, count * sizeof(char));
    for (i = CHAR_MIN; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
	fprintf(outfile, " %d", ptr[sizeof(long)-1]);
#else
	fprintf(outfile, " %d", ptr[0]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = MIN(UCHAR_MAX, num_elements);
    }
    else
    {
        tmp = UCHAR_MAX;
    }
    linc = UCHAR_MAX / tmp;
    count = UCHAR_MAX / linc + 1;
    fprintf(outfile, "%d %d %d",
        3, count, count * sizeof(unsigned char));
    for (i = 0; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
	fprintf(outfile, " %d", ptr[sizeof(long)-1]);
#else
	fprintf(outfile, " %d", ptr[0]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = MIN(SHRT_MAX - SHRT_MIN, num_elements);
    }
    else
    {
	tmp = SHRT_MAX - SHRT_MIN;
    }
    linc = (SHRT_MAX - SHRT_MIN) / tmp;
    count = (SHRT_MAX - SHRT_MIN) / linc + 1;
    fprintf(outfile, "%d %d %d", 4, count, count * sizeof(short));
    for (i = SHRT_MIN; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d",
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d",
	    ptr[0],
	    ptr[1]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = MIN(USHRT_MAX, num_elements);
    }
    else
    {
	tmp = USHRT_MAX;
    }
    linc = USHRT_MAX / tmp;
    count = USHRT_MAX / linc + 1;
    fprintf(outfile, "%d %d %d",
        5, count, count * sizeof(unsigned short));
    for (i = 0; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d",
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d",
	    ptr[0],
	    ptr[1]);
#endif
	fprintf(comparefile, "%hu ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = MIN(UINT_MAX, num_elements);
    }
    else
    {
	tmp = UINT_MAX;
    }
    linc = UINT_MAX / tmp;
    count = UINT_MAX / linc + 1;
    fprintf(outfile, "%d %d %d", 6, count, count * sizeof(int));
    for (i = INT_MIN; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[sizeof(long)-4],
	    ptr[sizeof(long)-3],
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = MIN(UINT_MAX, num_elements);
    }
    else
    {
	tmp = UINT_MAX;
    }
    linc = UINT_MAX / tmp;
    count = UINT_MAX / linc + 1;
    fprintf(outfile, "%d %d %d",
        7, count, count * sizeof(unsigned int));
    for (i = 0; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[sizeof(long)-4],
	    ptr[sizeof(long)-3],
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4]);
#endif
	fprintf(comparefile, "%u ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = num_elements;
    }
    else
    {
	tmp = ULONG_MAX;
    }
    linc = ULONG_MAX / tmp;
    count = ULONG_MAX / linc + 1;
    fprintf(outfile, "%d %d %d", 8, count, count * sizeof(long));
    for (i = LONG_MIN; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[sizeof(long)-4],
	    ptr[sizeof(long)-3],
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3]);
#endif
	fprintf(comparefile, "%d ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    if (num_elements)
    {
	tmp = num_elements;
    }
    else
    {
	tmp = ULONG_MAX;
    }
    linc = ULONG_MAX / tmp;
    count = ULONG_MAX / linc + 1;
    fprintf(outfile, "%d %d %d",
        9, count, count * sizeof(unsigned long));
    for (i = 0; count--; i += linc)
    {
#if BIG_ENDIAN == BYTE_ORDER
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[sizeof(long)-4],
	    ptr[sizeof(long)-3],
	    ptr[sizeof(long)-2],
	    ptr[sizeof(long)-1]);
#endif
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3]);
#endif
	fprintf(comparefile, "%u ", i);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    ptr = (unsigned char *)&k;
    if (num_elements)
    {
	count = num_elements;
    }
    else
    {
	count = 1024;
    }
    finc = (FLT_MAX * 2.0) / (float)tmp;
    fprintf(outfile, "%d %d %d", 10, tmp, tmp * sizeof(float));
    for (k = -FLT_MAX; count--; k += finc)
    {
#ifdef CRAY
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
#else
	fprintf(outfile, " %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3]);
#endif
	fprintf(comparefile, "%13.6e ", k);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");

    ptr = (unsigned char *)&j;
    if (num_elements)
    {
	count = num_elements;
    }
    else
    {
        count = 1024;
    }
    dinc = (DBL_MAX * 2.0) / (double)tmp;
    fprintf(outfile, "%d %d %d", 11, tmp, tmp * sizeof(double));
    for (j = -DBL_MAX; count--; j += dinc)
    {
	fprintf(outfile, " %d %d %d %d %d %d %d %d",
	    ptr[0],
	    ptr[1],
	    ptr[2],
	    ptr[3],
	    ptr[4],
	    ptr[5],
	    ptr[6],
	    ptr[7]);
	fprintf(comparefile, "%13.6e ", j);
    }
    fprintf(outfile, "\n");
    fprintf(comparefile, "\n");
}
