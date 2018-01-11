/*
 * rdb_test.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/rdb/rdb_test.c,v 1.4 1996/08/19 19:58:53 geisler Exp $";

#include <stdio.h>
#include "rdb.h"

/*
 * main()
 */
int main(int argc, char *argv[])
{
    nexus_bool_t done = NEXUS_FALSE;
    nexus_list_t *names, *cur_name;
    char name[1024];
    char key[1024];
    char *value;
    int count;
    
    resource_database_init(&argc, &argv);

    printf("Testing resource_database_get_names()...\n");
    printf("Enter file to get names from: ");
    fflush(stdout);
    scanf("%s", name);
    {
        names = resource_database_get_names(name);
        for (cur_name = names; cur_name; cur_name = cur_name->next)
        {
            printf("Name %d: %s\n", count++, (char *)cur_name->value);
        }
        printf("Names done.\n\n\n");
    }

    printf("Testing resource_database_lookup()...\n");
    while (!done)
    {
	printf("Enter name and key: ");
	fflush(stdout);
	if (scanf("%s %s", name, key) == 2)
	{
	    value = resource_database_lookup(name, key);
	    if (value)
	    {
		printf("Lookup of \"%s\" and \"%s\" found \"%s\"\n",
		       name, key, value);
	    }
	    else
	    {
		printf("Lookup of \"%s\" and \"%s\" failed.\n",
		       name, key);
	    }
	}
	else
	{
	    done = NEXUS_TRUE;
	}
    }
    
    resource_database_shutdown();
    
} /* main() */
