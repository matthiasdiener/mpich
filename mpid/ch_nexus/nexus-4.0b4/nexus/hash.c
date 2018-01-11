/*
 * Compute the Nexus handler hash value for the command line arguments.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/hash.c,v 1.5 1996/10/07 04:39:56 tuecke Exp $";

#define NEXUS_HANDLER_HASH_TABLE_SIZE 1021

int main(int argc, char **argv)
{
    int i;
    int hash;
    char *s, *name;

    for (i = 1; i < argc; i++)
    {
	name = argv[i];
	
	hash = 0;
	for (s = name; *s != '\0'; s++)
	    hash += (int) *s;
	hash = hash % NEXUS_HANDLER_HASH_TABLE_SIZE;

	printf("%d\t%s\n", hash, name);
    }
    
    return (0);
}
