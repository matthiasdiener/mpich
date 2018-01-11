#include <stdio.h>
#include "nexus_dc.h"
#include "UTP.h"

static void timer_init(int format, char *format_string);
static void read_infile(FILE *infile, int format, char *outfilename);
static void compare_files(char *comparefile, char *outfilename);

int main(int argc, char **argv)
{
    FILE *infile;
    FILE *comparefile;
    char outfilename[1024];
    int format;
    int iterations;
    int i;

    if (argc != 5)
    {
	printf("Usage:\n");
	printf("    %s <test_format> <test_file> <compare_file> <iterations>\n", argv[0]);
	printf("\nwhere <test_format> can be:\n\n");
	printf("          32le\n");
	printf("          32be\n");
	printf("          64le\n");
	printf("          64be\n");
	printf("          cray\n");
	printf("          java\n");
	printf("\n<test_file> is the location of the test file\n");
	printf("<compare_file> is the location of the comparison file\n");
	printf("and <iterations> is the number of times to repeat the experiment\n\n");
	return 1;
    }

    infile = fopen(argv[2], "r");
    if (!infile)
    {
	printf("Error reading %s\n", argv[2]);
	return 2;
    }

    comparefile = fopen(argv[3], "r");
    if (!comparefile)
    {
	printf("Error reading %s\n", argv[3]);
	return 4;
    }
    fclose(comparefile);

    if (strcmp(argv[1], "32le") == 0)
    {
	format = NEXUS_DC_FORMAT_32BIT_LE;
    }
    else if (strcmp(argv[1], "32be") == 0)
    {
	format = NEXUS_DC_FORMAT_32BIT_BE;
    }
    else if (strcmp(argv[1], "64le") == 0)
    {
	format = NEXUS_DC_FORMAT_64BIT_LE;
    }
    else if (strcmp(argv[1], "64be") == 0)
    {
	format = NEXUS_DC_FORMAT_64BIT_BE;
    }
    else if (strcmp(argv[1], "cray") == 0)
    {
	format = NEXUS_DC_FORMAT_CRAYC90;
    }
    else if (strcmp(argv[1], "java") == 0)
    {
	format = NEXUS_DC_FORMAT_JAVA;
    }
    else
    {
	printf("Invalid format.\n");
	return 3;
    }

    iterations = atoi(argv[4]);
    if (iterations <= 0)
    {
	printf("iterations must be > 0.\n");
	return 8;
    }

    timer_init(format, argv[1]);

    for (i = 0; i < iterations; i++)
    {
        read_infile(infile, format, outfilename);
	rewind(infile);
    }

    compare_files(argv[3], outfilename);
    UTP_shutdown();
    remove(outfilename);
    fclose(infile);
}

static void read_infile(FILE *infile, int format, char *outfilename)
{
    FILE *outfile;
    char filename[1024];

    sprintf(outfilename, "/tmp/nexus_dc_test_%d_%d", format, getpid());
    outfile = fopen(outfilename, "w");
    if (!outfile)
    {
	printf("Unable to open %s as a temp file.\n");
	exit(5);
    }
    /* 
     * format of file is:
     *
     *   type count num_elements elements
     *   type count num_elements elements
     *   :
     *   .
     */
    while(1)
    {
	unsigned char *in_buf;
	void *out_buf;
	int type;
	int count;
	int num_elements;
	int position;
	int rc;
	int i;

	rc = fscanf(infile, "%d %d %d", &type, &count, &num_elements);
	if (rc != 3)
	{
	    break;
	}

	in_buf = (unsigned char *)malloc(num_elements);
	for (i = 0; i < num_elements; i++)
	{
	    int tmp;

	    fscanf(infile, "%d", &tmp);
	    in_buf[i] = (unsigned char)tmp;
	}

	position = 0;
	switch(type)
	{
	  case 1:
	    out_buf = (void *)malloc(nexus_dc_sizeof_byte(count));
	    UTP_start_timer(0);
	    nexus_dc_put_byte((char *)out_buf, &position, (unsigned char *)in_buf, count);
	    UTP_stop_timer(0);
	    position = 0;
	    UTP_start_timer(11);
	    nexus_dc_get_byte(in_buf, &position, format, (unsigned char *)out_buf, count);
	    UTP_stop_timer(11);
	    for (i = 0; i < count; i++)
	    {
		fprintf(outfile, "%u ", ((unsigned char *)out_buf)[i]);
	    }
	    break;
	  case 2:
	    out_buf = (void *)malloc(nexus_dc_sizeof_char(count));
	    UTP_start_timer(1);
	    nexus_dc_put_char((char *)out_buf, &position, (char *)in_buf, count);
	    UTP_stop_timer(1);
	    position = 0;
	    UTP_start_timer(12);
	    nexus_dc_get_char(in_buf, &position, format, (char *)out_buf, count);
	    UTP_stop_timer(12);
	    for (i = 0; i < count; i++)
	    {
		fprintf(outfile, "%d ", ((char *)out_buf)[i]);
	    }
	    break;
	  case 3:
	    out_buf = (void *)malloc(nexus_dc_sizeof_u_char(count));
	    UTP_start_timer(2);
	    nexus_dc_put_u_char((char *)out_buf, &position, (unsigned char *)in_buf, count);
	    UTP_stop_timer(2);
	    position = 0;
	    UTP_start_timer(13);
	    nexus_dc_get_u_char(in_buf, &position, format, (unsigned char *)out_buf, count);
	    UTP_stop_timer(13);
	    for (i = 0; i < count; i++)
	    {
		fprintf(outfile, "%u ", ((unsigned char *)out_buf)[i]);
	    }
	    break;
	  case 4:
	    out_buf = (void *)malloc(nexus_dc_sizeof_short(count));
	    UTP_start_timer(3);
	    nexus_dc_put_short((char *)out_buf, &position, (short *)in_buf, count);
	    UTP_stop_timer(3);
	    position = 0;
	    UTP_start_timer(14);
	    nexus_dc_get_short(in_buf, &position, format, (short *)out_buf, count);
	    UTP_stop_timer(14);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%hd ", ((short *)out_buf)[i]);
	    }
	    break;
	  case 5:
	    out_buf = (void *)malloc(nexus_dc_sizeof_u_short(count));
	    UTP_start_timer(4);
	    nexus_dc_put_u_short((char *)out_buf, &position, (unsigned short *)in_buf, count);
	    UTP_stop_timer(4);
	    position = 0;
	    UTP_start_timer(15);
	    nexus_dc_get_u_short(in_buf, &position, format, (unsigned short *)out_buf, count);
	    UTP_stop_timer(15);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%hu ", ((unsigned short *)out_buf)[i]);
	    }
	    break;
	  case 6:
	    out_buf = (void *)malloc(nexus_dc_sizeof_int(count));
	    UTP_start_timer(5);
	    nexus_dc_put_int((char *)out_buf, &position, (int *)in_buf, count);
	    UTP_stop_timer(5);
	    position = 0;
	    UTP_start_timer(16);
	    nexus_dc_get_int(in_buf, &position, format, (int *)out_buf, count);
	    UTP_stop_timer(16);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%d ", ((int *)out_buf)[i]);
	    }
	    break;
	  case 7:
	    out_buf = (void *)malloc(nexus_dc_sizeof_u_int(count));
	    UTP_start_timer(6);
	    nexus_dc_put_int((char *)out_buf, &position, (unsigned int *)in_buf, count);
	    UTP_stop_timer(6);
	    position = 0;
	    UTP_start_timer(17);
	    nexus_dc_get_u_int(in_buf, &position, format, (unsigned int *)out_buf, count);
	    UTP_stop_timer(17);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%u ", ((unsigned int *)out_buf)[i]);
	    }
	    break;
	  case 8:
	    out_buf = (void *)malloc(nexus_dc_sizeof_long(count));
	    UTP_start_timer(7);
	    nexus_dc_put_long((char *)out_buf, &position, (long *)in_buf, count);
	    UTP_stop_timer(7);
	    position = 0;
	    UTP_start_timer(18);
	    nexus_dc_get_long(in_buf, &position, format, (long *)out_buf, count);
	    UTP_stop_timer(18);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%ld ", ((long *)out_buf)[i]);
	    }
	    break;
	  case 9:
	    out_buf = (void *)malloc(nexus_dc_sizeof_u_long(count));
	    UTP_start_timer(8);
	    nexus_dc_put_u_long((char *)out_buf, &position, (unsigned long *)in_buf, count);
	    UTP_stop_timer(8);
	    position = 0;
	    UTP_start_timer(19);
	    nexus_dc_get_u_long(in_buf, &position, format, (unsigned long *)out_buf, count);
	    UTP_stop_timer(19);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%lu ", ((unsigned long *)out_buf)[i]);
	    }
	    break;
	  case 10:
	    out_buf = (void *)malloc(nexus_dc_sizeof_float(count));
	    UTP_start_timer(9);
	    nexus_dc_put_float((char *)out_buf, &position, (float *)in_buf, count);
	    UTP_stop_timer(9);
	    position = 0;
	    UTP_start_timer(20);
	    nexus_dc_get_float(in_buf, &position, format, (float *)out_buf, count);
	    UTP_stop_timer(20);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%13.6e ", ((float *)out_buf)[i]);
	    }
	    break;
	  case 11:
	    out_buf = (void *)malloc(nexus_dc_sizeof_double(count));
	    UTP_start_timer(10);
	    nexus_dc_put_double((char *)out_buf, &position, (double *)in_buf, count);
	    UTP_stop_timer(10);
	    position = 0;
	    UTP_start_timer(21);
	    nexus_dc_get_double(in_buf, &position, format, (double *)out_buf, count);
	    UTP_stop_timer(21);
	    for (i = 0; i < count; i++)
	    {
	        fprintf(outfile, "%13.6e ", ((double *)out_buf)[i]);
	    }
	    break;
	  default:
	    printf("Unknown type (%d) in data file\n", type);
	    exit(6);
	}
	fprintf(outfile, "\n");
    }
    fclose(outfile);
}

static void compare_files(char *comparefile, char *outfile)
{
    char command[1024];
    int rc;

    sprintf(command, "diff %s %s", comparefile, outfile);
    rc = system(command);
    if (rc)
    {
	printf("Fundamental difference in output when comparing %s with %s\n", comparefile, outfile);
	UTP_shutdown();
	exit(7);
    }
}

static void timer_init(int format, char *format_string)
{
    UTP_init(22, UTP_MODE_SHARED);
    UTP_set_filename("compare.utp");
    UTP_set_attribute("rcsid == %s", "", "$Header: /nfs/globus1/src/master/nexus_dc/timings/compare.c,v 1.3 1996/10/07 04:16:28 tuecke Exp $");

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

    UTP_name_timer(11, "get byte from %s--%d", format_string, format);
    UTP_name_timer(12, "get char from %s--%d", format_string, format);
    UTP_name_timer(13, "get u_char from %s--%d", format_string, format);
    UTP_name_timer(14, "get short from %s--%d", format_string, format);
    UTP_name_timer(15, "get u_short from %s--%d", format_string, format);
    UTP_name_timer(16, "get int from %s--%d", format_string, format);
    UTP_name_timer(17, "get u_int from %s--%d", format_string, format);
    UTP_name_timer(18, "get long from %s--%d", format_string, format);
    UTP_name_timer(19, "get u_long from %s--%d", format_string, format);
    UTP_name_timer(20, "get float from %s--%d", format_string, format);
    UTP_name_timer(21, "get double from %s--%d", format_string, format);
}
