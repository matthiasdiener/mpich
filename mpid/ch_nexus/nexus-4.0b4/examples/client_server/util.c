/*
 * util.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_examples/client_server/util.c,v 1.3 1996/02/06 19:39:58 geisler Exp $";

#include "nexus.h"
#include "nx_c2c.h"


/*
 * write_attach_file()
 */
bool_t write_attach_file(char *url, char *file)
{
    FILE *fp;
    nexus_stdio_lock();
    printf("Attachment URL: %s\n", url);
    if ((fp = fopen(file, "w")) == (FILE *) NULL)
    {
	nexus_stdio_unlock();
	return(NEXUS_FALSE);
    }
    fprintf(fp, "%s\n", url);
    fclose(fp);
    nexus_stdio_unlock();
    return(NEXUS_TRUE);
} /* write_attach_file() */


/*
 * read_attach_file()
 */
bool_t read_attach_file(char *url, char *file)
{
    FILE *fp;
    nexus_stdio_lock();
    if ((fp = fopen(file, "r")) == (FILE *) NULL)
    {
	nexus_stdio_unlock();
	return(NEXUS_FALSE);
    }
    fscanf(fp, "%s", url);
    fclose(fp);
    nexus_stdio_unlock();
    return(NEXUS_TRUE);
} /* read_attach_file() */
