/*
 * p0_util.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_util.c,v 1.2 1995/03/30 18:34:29 tuecke Exp $";

#include "p0_internal.h"

/*
 * _p0_copy_string()
 *
 * Copy the string into malloced space and return it.
 *
 * Reminder: This function returns a pointer to malloced memory, so
 * don't forget to use Ports0Free() on it...
 */
char *_p0_copy_string(char *s)
{
    char *rc;

    Ports0Malloc(_p0_copy_string(), rc, char *, (strlen(s) + 1));
    strcpy(rc, s);
    return (rc);
} /* _p0_copy_string() */
