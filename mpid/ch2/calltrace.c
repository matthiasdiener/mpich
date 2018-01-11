
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "calltrace.h"

/* Declarations */
#ifdef DEBUG_TRACE
char *(TR_stack[TR_MAX_STACK]);
int  TR_stack_sp = 0, TR_stack_debug = 0;

void TR_stack_init( flag )
int flag;
{
    TR_stack_debug = flag;
}

/* Generate a stack trace */
void TR_stack_print( fp, dir )
FILE *fp;
int  dir;
{
    int i;

    if (dir == 1) {
	for (i=0; i<TR_stack_sp; i++) {
	    fprintf( fp, "(%d) %s\n", i, TR_stack[i] );
	}
    }
    else {
	for (i=TR_stack_sp-1; i>=0; i--) {
	    fprintf( fp, "(%d) %s\n", i, TR_stack[i] );
	}
    }
}

#else
void TR_stack_init( flag )
int flag;
{
}
#endif
