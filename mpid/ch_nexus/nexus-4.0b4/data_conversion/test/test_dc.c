/*
 * test_dc.c
 *
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "nexus_dc.h"

#define CRAY_FLOAT_CONVERSION_TOLERANCE		0.00001
#define CRAY_DOUBLE_CONVERSION_TOLERANCE	0.000000001

/*variables that are put into buffer*/
    unsigned char put_u_char[256];
    short put_short[10];
    unsigned short put_u_short[10];
    int put_int[10];
    unsigned  put_u_int[10];
    long put_long[20];
    unsigned long put_u_long[20];
    float put_float[500];
    double put_double[20];
    nexus_byte_t put_byte[256];
    char *put_char_str= "This is a test";
    char put_char[255];

void dc_puts(void);

/*
 * set_put_vars
 *
 * sets the variables that are put into the buffer, if you want
 * to modify the values of any of the variables, except put_char_str
 * you should only need to modify it here
 */
void set_put_vars(void)
{
    int i;

    for (i= -5; i<4; i++)
    {
	put_int[i+5]= i*200;
    }
    for (i= 0; i<9; i++)
    {
   	put_u_int[i]= i*1000;
    }
    put_u_int[9]= 3000000000;
    put_int[9]= 3000000;
    for (i= -5; i<5; i++)
    {
	put_short[i+5]= i*200;
    }
    for (i= 0; i<10; i++)
    {
   	put_u_short[i]= i*1000;
    }
    for (i= -10; i<10; i++)
    {
	put_long[i+10]= 10250* i;
    }
    for (i= 0; i<20; i++)
    {
    	put_u_long[i]= 2000* i;
    }
    put_u_long[19]= put_u_long[18];
    put_long[19]= put_long[18];;
    put_u_long[18]= 3000000000;
    put_long[18]= 3000000;
    for (i= -200; i<300; i++)
    {
	put_float[i+200]= i*i*0.75;
    }
    for (i= -10; i<10; i++)
    {
	put_double[i+10]= i* 0.99* 100000;
    }
    for (i= 0; i< 256; i++)
    {
	put_byte[i]= i;
    }
    for (i= 0; i< 256; i++)
    {
	put_u_char[i]= i;
    }
    for (i= -127; i <127; i++)
    {
	put_char[i+127]= i;
    }
}	/* set_put_vars */

/*
 * open_put_files
 *
 * this opens the appropriate .in file
 */
int open_put_files(nexus_byte_t *end, nexus_byte_t *start)
{
   FILE *fp;
   char *filename;
   int type;

   type= NEXUS_DC_FORMAT_LOCAL;

   switch (type)
   {
      case NEXUS_DC_FORMAT_32BIT_LE:
        filename= "32le.in";
        break;
      case NEXUS_DC_FORMAT_32BIT_BE:
	filename= "32be.in";
        break;
      case NEXUS_DC_FORMAT_64BIT_LE:
	filename= "64le.in";
	break;
      case NEXUS_DC_FORMAT_64BIT_BE:
	filename= "64be.in";
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	filename= "cray.in";
	break;
      default:
        printf("Error in switch\n");
        return 1;
   }
   if ((fp= fopen(filename, "w")))
   {
      while (start != end)
      {
         fprintf(fp, "%c", *start);
	 start= start+ 1;
      }
   }
   else
   {
      printf("Error in opening file for put buffer\n");
      return 1;
   }
   fclose(fp);
   return 0;
}	/* open_put_files */

/*
 * dc_puts
 *
 * does the sizing of the buffer and puts the items into the 
 * buffer
 */
void dc_puts()
{
    nexus_byte_t *buffer, *beginning;
    int size,count;
    nexus_byte_t type;

    type= (nexus_byte_t)(NEXUS_DC_FORMAT_LOCAL);
    size= nexus_dc_sizeof_byte(1);
    size+= nexus_dc_sizeof_int(1);
    size+= nexus_dc_sizeof_char(strlen(put_char_str));
    size+= nexus_dc_sizeof_u_char(256);
    size+= nexus_dc_sizeof_short(10);
    size+= nexus_dc_sizeof_u_short(10);
    size+= nexus_dc_sizeof_int(10);
    size+= nexus_dc_sizeof_u_int(10);
    size+= nexus_dc_sizeof_long(20);
    size+= nexus_dc_sizeof_u_long(20);
    size+= nexus_dc_sizeof_float(500);
    size+= nexus_dc_sizeof_double(20);
    size+= nexus_dc_sizeof_byte(256);
    size+= nexus_dc_sizeof_char(255);
    buffer= (nexus_byte_t *)malloc(size);
    beginning= buffer;
    nexus_dc_put_byte(&buffer, &type, 1);
    count= strlen(put_char_str);
    nexus_dc_put_int(&buffer, &count, 1);
    nexus_dc_put_char(&buffer, put_char_str, strlen(put_char_str));
    nexus_dc_put_u_char(&buffer, put_u_char, 256);
    nexus_dc_put_short(&buffer, put_short, 10);
    nexus_dc_put_u_short(&buffer, put_u_short, 10);
    nexus_dc_put_int(&buffer, put_int, 10);
    nexus_dc_put_u_int(&buffer, put_u_int, 10);
    nexus_dc_put_long(&buffer, put_long, 20);
    nexus_dc_put_u_long(&buffer, put_u_long, 20);
    nexus_dc_put_float(&buffer, put_float, 500);
    nexus_dc_put_double(&buffer, put_double, 20);
    nexus_dc_put_byte(&buffer, put_byte, 256);
    nexus_dc_put_char(&buffer, put_char, 255);
    open_put_files(buffer, beginning);
}	/* dc_puts */


/*
 * message
 *
 * prints whether there is an error in a type
 */
int message(int error, char type[])
{
    if (!error)
    {
	printf("no error in %s...\n", type);
    }
    else
    {
        printf("ERROR in %s\n", type);
    }
    return 0;
}

/*
 * dc_gets
 *
 * gets the variables out of the buffer and checks to see if
 * they are correct
 */
void dc_gets(nexus_byte_t *buffer)
{
    int i,count;
    nexus_byte_t test_byte[256];
    char test_char_str[15];
    char test_char[255];
    unsigned char test_u_char[256];
    short test_short[10];
    unsigned short test_u_short[10];
    int test_int[10];
    unsigned test_u_int[10];
    long test_long[20];
    unsigned long test_u_long[20];
    float test_float[500];
    double test_double[20];
    nexus_byte_t bformat;
    int format;
    int error= 0;
    int craymachine= 0;   /*boolean- whether machine is a cray*/ 
    int checktol= 0;      /*boolean- whether to check the conversion tolerance*/
    int crayfile= 0;
    FILE *fp;

    nexus_dc_get_byte(&buffer, &bformat, 1, NEXUS_DC_FORMAT_LOCAL);
    format= (int)bformat;
    printf("in dc_gets: format= %i\n", format);
    if (NEXUS_DC_FORMAT_LOCAL ==  NEXUS_DC_FORMAT_CRAYC90)
    {
	craymachine= 1;
    }
    if (format== NEXUS_DC_FORMAT_CRAYC90)
    {
	crayfile= 1;
    }
    if ((craymachine && !crayfile) || (!craymachine && crayfile))
    {
   	checktol= 1;
        printf("crayfile= %i; craymachine= %i\n", crayfile, craymachine);
    }
    fp= fopen("error_file", "a");
    if (!fp)
    {
	printf("Failed to open error_file\n");
	exit(1);
    }
    fprintf(fp, "\n\nformat of following data = %i\n", format);

    nexus_dc_get_int(&buffer, &count, 1, format);
    printf("in dc_gets: count= %i\n",count);
    nexus_dc_get_char(&buffer, test_char_str, count, format);
    *(test_char_str+count)= '\0';
    printf("int dc_gets: string= %s\n",test_char_str);
    nexus_dc_get_u_char(&buffer, test_u_char, 256, format);
    for (i= 0; i<255; i++)
    {
        if (test_u_char[i]!= put_u_char[i])
        {
	    error= 1;
            fprintf(fp,"unsigned char[%i]: expected: %x   received: %x\n",
		    i, (unsigned int) put_u_char[i],
		    (unsigned int) test_u_char[i]);
	}
    }
    error= message(error, "u_chars");

    nexus_dc_get_short(&buffer, test_short, 10, format);
    for (i= 0; i<10; i++)
    {
        if (*(test_short+i)!= *(put_short+i))
        {
       	    error= 1;
            fprintf(fp,"short int[%i]: expected: %hx   received: %hx\n",
		    i, put_short[i], test_short[i]);
	}
    }
    error= message(error, "shorts");

    nexus_dc_get_u_short(&buffer, test_u_short, 10, format);
    for (i= 0; i<10; i++)
    {
        if (*(test_u_short+i)!=*(put_u_short+i))
        {
       	    error= 1;
            fprintf(fp,"unsigned short int[%i]: expected: %hx   received: %hx\n",
		i, put_u_short[i], test_u_short[i]);
	}
    }
    error= message(error, "unsigned shorts");

    nexus_dc_get_int(&buffer, test_int, 10, format);
    for (i= 0; i<10; i++)
    {
        if (*(test_int+i)!= *(put_int+i))
        {
       	    error= 1;
            fprintf(fp,"int[%i]: expected: %x   received: %x\n",
		i, put_int[i], test_int[i]);
	}
    }
    error= message(error, "ints");

    nexus_dc_get_u_int(&buffer, test_u_int, 10, format);
    for (i= 0; i<10; i++)
    {
        if (*(test_u_int+i)!=*(put_u_int+i))
        {
       	    error= 1;
            fprintf(fp,"unsigned int[%i]: expected: %x   received: %x\n",
		i, put_u_int[i], test_u_int[i]);
	}
    }
    error= message(error, "unsigned ints");

    nexus_dc_get_long(&buffer, test_long, 20, format);
    for (i= 0; i<20; i++)
    {   
        if (test_long[i]!= put_long[i]) 
        {
	    error= 1;
	    fprintf(fp,"long ints[%i]: expected: %lx; recieved: %lx\n",
		i, put_long[i], test_long[i]);
	}
    }
    error= message(error, "longs");

    nexus_dc_get_u_long(&buffer, test_u_long, 20, format);
    for (i= 0; i<20; i++)
    {   
        if (test_u_long[i]!= put_u_long[i])
        {
	    error= 1;
	    fprintf(fp, "u long ints[%i]: expected: %lx; recieved: %lx\n", 
		i, put_u_long[i], test_u_long[i]);
	}
    }
    error= message(error, "unsigned longs");

    nexus_dc_get_float(&buffer, test_float, 500, format);
    for (i= 0; i<500; i++)
    {
        if (test_float[i]!= put_float[i])
        {
	    if (checktol)
	    {
		if ((test_float[i]< put_float[i]-CRAY_FLOAT_CONVERSION_TOLERANCE)
	           || (test_float[i]> put_float[i]+CRAY_FLOAT_CONVERSION_TOLERANCE))
		{
	           error= 1;
		   printf("Tolerance check on %i: float not within acceptable limit\n",i);
		   fprintf(fp, "floats[%i]: expected value: %x; recieved value= %x\n", 
			i, put_float[i], test_float[i]);
		}
		else
		{
		   printf("Tolerance check: float not exact but within acceptable limits\n");
		}
	    }
	    else 
	    {
		error= 1;
		fprintf(fp, "floats[%i]: expected value: %x; recieved value= %x\n", 
		     i, put_float[i], test_float[i]);
	    }
	}
    }
    error= message(error, "floats");


    nexus_dc_get_double(&buffer, test_double, 20, format);
    for (i= 0; i<20; i++)
    {
        if (test_double[i]!= put_double[i])
        {
            if (checktol)
	    {
                if ((test_double[i]< put_double[i]-CRAY_DOUBLE_CONVERSION_TOLERANCE)
	          || (test_double[i]> put_double[i]+CRAY_DOUBLE_CONVERSION_TOLERANCE))
                {
                    error= 1;
		    printf("tolerance check on %i: double not within acceptable limit\n",i);
		    fprintf(fp, "float[%i]: expected value: %lx; recieved value= %lx\n", 
	                i, put_double[i], test_double[i]);
                }
	        else
	        {
		    printf("tolerance check: double not exact but within acceptable limit\n");
	        }
	    }
            else  
            {
                error= 1;
		fprintf(fp, "float[%i]: expected value: %lx; recieved value= %lx\n", 
	            i, put_double[i], test_double[i]);
            }
	}
    }
    error= message(error, "doubles");

    nexus_dc_get_byte(&buffer, test_byte, 256, format);
    for (i= 0; i<256; i++)
    {
        if (test_byte[i]!= put_byte[i])
        {
	    error= 1;
            fprintf(fp,"byte[%i]: expected: %x   received: %x\n",
		    i, (unsigned int) put_byte[i],
		    (unsigned int) test_byte[i]);
	}
    }
    error= message(error, "bytes");

    nexus_dc_get_char(&buffer, test_char, 255, format);
    for (i= 0; i<255; i++)
    {
        if (test_char[i]!= put_char[i])
        {
	    error= 1;
            fprintf(fp,"char[%i]: expected: %x   received: %x\n",
		i, (unsigned int) put_char[i], (unsigned int) test_char[i]);
	}
    }
    error= message(error, "chars");

    fclose(fp);
}	/* dc_gets */

/*
 * open_and_get
 *
 * opens the file to read the buffer from and calls dc_gets
 */
int open_and_get(char *filename)
{
    FILE *fp;
    nexus_byte_t *buffer;    
    int filesize, i= 0;
    nexus_byte_t temp;

    if (!(fp= fopen(filename, "r")))
    {
       return 1;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer= (nexus_byte_t *)malloc(filesize*sizeof(nexus_byte_t));
    fscanf(fp, "%c", &temp);
    while (!feof(fp))
    {
       *(buffer+i++)=temp;
       fscanf(fp, "%c", &temp);
    }
    fclose(fp);    
    dc_gets(buffer);
    return 0;
}	/* open_and_get */

/*
 * get_all
 *
 * if no command line arguments are entered then the program gets
 * all of the files and tests them
 */
void get_all(void)
{
    char *filename[] = 
      { "32be.in",
        "32le.in",
        "64le.in",
        "64be.in",
        "cray.in" };
    int i, rc;

    for (i= 0; i< 5; i++)
    {
	printf("filename= %s\n",filename[i]);
        rc= open_and_get(filename[i]);
        if (rc != 0)
        {
            printf("Error in handling file %s\n",filename[i]);
        } 
    }  
}	/* get_all */


/*
 * MAIN 
 */
int main (int argc, char **argv)
{
    int          i= 2, rc;
    char	*filename;

    set_put_vars();
    if (argc == 1)
    {
	/* the user didn't specify any parameters, assume some defaults */
	printf("Usage: %s -put\n", argv[0]);
	printf("   or: %s -get <file>\n", argv[0]);
    }
    else
    {
	/* the user specified something, so let's go with that */
	if (!strcmp (argv[1], "-put"))
	{
 	    dc_puts();
	}
	else
	{
	    if (!strcmp (argv[1], "-get"))
	    {
		if (argc < 3)
		{
		    printf("ERROR: argument expected.\n");
		}
		else
		{
		    while (argc > i)
		    {
			filename= argv[i];
 			printf("filename= %s\n",filename);
			rc= open_and_get(filename);
			if (rc != 0)
			{
			    printf("Error in handling file %s\n",filename);
			}
			i++;
		    }
		}
	    }
	    else
	    {
		get_all();
	    }
	}
    }
    return(0);
} /* main() */
