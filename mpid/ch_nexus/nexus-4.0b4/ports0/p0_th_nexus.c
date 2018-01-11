/*
 * p0_th_nexus.c
 *
 * This ports0 thread module just redirects all thread calls
 * to the equivalent Nexus routines.
 * This allows a native Nexus thread library to be used with the
 * rest of ports0.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_nexus.c,v 1.2 1995/10/04 14:49:45 geisler Exp $";


/*
 * _p0_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _p0_thread_usage_message(void)
{
} /* _p0_thread_usage_message() */


/*
 * _p0_thread_new_process_params()
 *
 * Write any new process parameters for this module into 'buf',
 * up to a maximum of 'size' characters.
 *
 * Return:	The number of characters written into 'buf'.
 */
int _p0_thread_new_process_params(char *buf, int size)
{
    return (0);
} /* _p0_thread_new_process_params() */


/*
 * _p0_thread_preinit()
 *
 * If you need to call a thread package initialization routine, then
 * do it here.  This is called as the very first thing in ports0_init().
 */
void _p0_thread_preinit( void )
{
} /* _p0_thread_preinit() */


/*
 * _p0_thread_init()
 */
void _p0_thread_init(void)
{
} /* _p0_thread_init() */


/*
 * _p0_thread_shutdown()
 */
int _p0_thread_shutdown(void)
{
    return 0;
} /* _p0_thread_shutdown() */


/*
 * _p0_thread_abort()
 */
void _p0_thread_abort(int return_code)
{
} /* _p0_thread_abort() */
