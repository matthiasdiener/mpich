#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include "UTP.h"

main(int argc, char **argv)
{
    void *in_buf, *out_buf;
    XDR xdrs;
    int size;
    int count;
    int iterations;
    int i, j;

    if (argc != 3)
    {
	printf("Usage:\n");
	printf("    %s <array_size> <iterations>\n", argv[0]);
	printf("\nwhere <array_size> is the size of the arrays to be converted\n");
	printf("and <iterations> is the number of times the experiment\n");
	printf("    should be repeated\n\n");
	return 1;
    }

    count = atoi(argv[1]);
    count += size % 4; /* make sure size is a multiple of 4 */

    if (count <= 0)
    {
	printf("size must be > 0.\n");
	return 2;
    }

    iterations = atoi(argv[2]);
    if (iterations <= 0)
    {
	printf("iterations must be > 0.\n");
	return 3;
    }

    UTP_init(22, UTP_MODE_SHARED);
    UTP_set_filename("xdr_timers.utp");
    UTP_set_attribute("rcsid%s", "", "$Header: /nfs/globus1/src/master/nexus_dc/timings/xdr_times.c,v 1.2 1996/10/07 04:16:30 tuecke Exp $");

    UTP_name_timer(0, "put byte");
    UTP_name_timer(1, "put char");
    UTP_name_timer(2, "put u_char");
    UTP_name_timer(3, "put short");
    UTP_name_timer(4, "put u_short");
    UTP_name_timer(5, "put int");
    UTP_name_timer(6, "put u_int");
    UTP_name_timer(7, "put long");
    UTP_name_timer(8, "put u_long");
    UTP_name_timer(9, "put float");
    UTP_name_timer(10, "put double");

    UTP_name_timer(11, "get byte");
    UTP_name_timer(12, "get char");
    UTP_name_timer(13, "get u_char");
    UTP_name_timer(14, "get short");
    UTP_name_timer(15, "get u_short");
    UTP_name_timer(16, "get int");
    UTP_name_timer(17, "get u_int");
    UTP_name_timer(18, "get long");
    UTP_name_timer(19, "get u_long");
    UTP_name_timer(20, "get float");
    UTP_name_timer(21, "get double");

    for (j = 0; j < iterations; j++)
    {
        size = sizeof(unsigned char) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(0);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_u_char(&xdrs, (unsigned char *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(0);
    
	UTP_start_timer(11);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_char(&xdrs, (unsigned char *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(11);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(char) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(1);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_char(&xdrs, (char *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(1);
    
	UTP_start_timer(12);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
            xdr_char(&xdrs, (char *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(12);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(unsigned char) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(2);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_char(&xdrs, (unsigned char *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(2);
    
	UTP_start_timer(13);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
    	xdr_u_char(&xdrs, (unsigned char *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(13);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(short) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(3);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_short(&xdrs, (short *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(3);
    
	UTP_start_timer(14);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_short(&xdrs, (short *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(14);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(unsigned short) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(4);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_short(&xdrs, (unsigned short *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(4);
    
	UTP_start_timer(15);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
	for (i = 0; i < count; i++)
	{
            xdr_u_short(&xdrs, (unsigned short *)out_buf + i);
	}
        xdr_destroy(&xdrs);
        UTP_stop_timer(15);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(int) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(5);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_int(&xdrs, (int *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(5);
    
	UTP_start_timer(16);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_int(&xdrs, (int *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(16);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(unsigned int) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(6);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_int(&xdrs, (unsigned int *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(6);
    
	UTP_start_timer(17);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_int(&xdrs, (unsigned int *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(17);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(long) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(7);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_long(&xdrs, (long *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(7);
    
	UTP_start_timer(18);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
            xdr_long(&xdrs, (long *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(18);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(unsigned long) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(8);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_u_long(&xdrs, (unsigned long *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(8);
    
	UTP_start_timer(19);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_u_long(&xdrs, (unsigned long *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(19);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(float) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(9);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_float(&xdrs, (float *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(9);
    
	UTP_start_timer(20);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
    	    xdr_float(&xdrs, (float *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(20);
    
        free(out_buf);
        free(in_buf);
    
        size = sizeof(double) * count;
        out_buf = (void *)malloc(size);
        in_buf = (void *)malloc(size);
    
        UTP_start_timer(10);
        xdrmem_create(&xdrs, out_buf, size, XDR_ENCODE);
        for (i = 0; i < count; i++)
        {
            xdr_double(&xdrs, (double *)in_buf + i);
        }
        xdr_destroy(&xdrs);
	UTP_stop_timer(10);
    
	UTP_start_timer(21);
        xdrmem_create(&xdrs, in_buf, size, XDR_DECODE);
        for (i = 0; i < count; i++)
        {
            xdr_double(&xdrs, (double *)out_buf + i);
        }
        xdr_destroy(&xdrs);
        UTP_stop_timer(21);
    
        free(out_buf);
        free(in_buf);
    
    }
    UTP_shutdown();
}
