/*
 * p0_sanity.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_sanity.c,v 1.3 1996/10/19 17:10:03 carl Exp $";

#include "p0_internal.h"
#ifndef BUILD_LITE

/*
 * _p0_interrogate_suspect()
 */
void _p0_interrogate_suspect( void *evidence,
			     _p0_structure_types suspect_type,
			     char *caller )
{
    switch( suspect_type )
    {
    case _P0_THREAD_T:
	break;
    case _P0_MUTEX_T:
    {
	ports0_mutex_t *m;

	m = (ports0_mutex_t *) evidence;
	if(!_P0_CHECK_START_MAGIC_COOKIE(m))
	{
	    char buf[100];
	    putchar(7);		/* beep */
	    sprintf(buf, "%s: Corrupted mutex (starting cookie = %x)",
		    caller, (unsigned int) _P0_EXTRACT_START_MAGIC_COOKIE(m));
	    _p0_imprison_thread( buf );
	}
	else if(!_P0_CHECK_END_MAGIC_COOKIE(m))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted mutex (ending cookie = %x)",
		    caller, (unsigned int) _P0_EXTRACT_END_MAGIC_COOKIE(m));
	    _p0_imprison_thread( buf );
	}
    }
	break;
    case _P0_COND_T:
    {
	ports0_cond_t *c;

	c = (ports0_cond_t *) evidence;
	if(!_P0_CHECK_START_MAGIC_COOKIE(c))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted condition (starting cookie = %x)",
		    caller, (unsigned int) _P0_EXTRACT_START_MAGIC_COOKIE(c));
	    _p0_imprison_thread( buf );
	}
	else if(!_P0_CHECK_END_MAGIC_COOKIE(c))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted condition (ending cookie = %x)",
		    caller, (unsigned int) _P0_EXTRACT_END_MAGIC_COOKIE(c));
	    _p0_imprison_thread( buf );
	}
	break;
    }
    case _P0_THREAD_FREELIST_T:
	break;
    }
} /* _p0_interrogate_suspect() */


/*
 * _p0_imprison_thread()
 */
void _p0_imprison_thread( char *psz_charges )
{
    int dummy_flag;
    int parole=PORTS0_FALSE;

    ports0_printf("Thread Imprisoned:\nCharged with: %s\n", psz_charges );

    while( !parole )
    {
	dummy_flag=0;
    }

    ports0_printf("Thread released on parole\n" );

} /* _p0_imprison_thread() */

#endif /* BUILD_LITE */
