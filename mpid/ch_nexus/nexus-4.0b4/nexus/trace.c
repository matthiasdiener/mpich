/*
 * trace.c
 *
 * Code for printing stack tracebacks.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/trace.c,v 1.11 1996/10/07 04:40:22 tuecke Exp $";

#include "internal.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

static void num_to_string(int n, char *str)
{
    int done = 0;
    int ones;
    int neg;
    char tmp[30], *s, *t;

    if (n < 0)
    {
	neg = 1;
	n = -n;
    }
    else
    {
	neg = 0;
    }

    s = tmp;
    while (!done)
    {
	ones = n % 10;
	n /= 10;

	*s++ = ones + 48;
	done = (n == 0);
    }
    s--;
    t = str;
    if (neg)
    {
	*t++ = '-';
    }
    while (s >= tmp)
    {
	*t = *s;
	t++;
	s--;
    }
    *t = '\0';
	
} /* num_to_string() */


/*
 * _nx_traceback()
 *
 * Put together a string to prefix to any output, and then call
 * the ports0 traceback routine.
 */
void _nx_traceback(void)
{
    char hostname[MAXHOSTNAMELEN];
    char output_prefix[1024];
    char num[30];
    int node_id, context_id, thread_id;

    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    _nx_thread_id(&thread_id);

    gethostname(hostname, MAXHOSTNAMELEN);

    strcpy(output_prefix, hostname);
    strcat(output_prefix, ":p");
    num_to_string(getpid(), num);
    strcat(output_prefix, num);

    strcat(output_prefix, ":n");
    num_to_string(node_id, num);
    strcat(output_prefix, num);

    strcat(output_prefix, ":c");
    num_to_string(context_id, num);
    strcat(output_prefix, num);

    strcat(output_prefix, ":t");
    num_to_string(thread_id, num);
    strcat(output_prefix, num);
    strcat(output_prefix, ": ");

    ports0_traceback(output_prefix);
    
} /* _nx_traceback() */
