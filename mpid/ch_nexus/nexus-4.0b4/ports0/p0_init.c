/*
 * p0_init.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_init.c,v 1.8 1996/01/29 22:47:25 patton Exp $";

#define PORTS0_DEFINE_GLOBALS

#include "p0_internal.h"

static ports0_bool_t preinit_called  = PORTS0_FALSE;
ports0_bool_t _p0_ports0_init_called = PORTS0_FALSE;

/*
 * ports0_preinit()
 */
void ports0_preinit(void)
{
    if (!preinit_called)
    {
#ifndef BUILD_LITE
	_p0_thread_preinit();
#endif /* BUILD_LITE */
	preinit_called = PORTS0_TRUE;
    }
} /* ports0_preinit() */


/*
 * ports0_init()
 *
 * Initialize ports0 and return.
 *
 * The modules should be initialized in the following order:
 *	1) machine dependent (md) module
 *	2) thread module
 *	3) protocal modules
 */
void ports0_init(int *argc,
		 char **argv[],
		 char *package_id)
{
    /*
     * This will bootstrap the thread package if necessary, so that
     * basic calls such as printf, malloc, etc can be called.
     */
    ports0_preinit();

    /*
     * Do some global initializations
     */
    _p0_stdout = stdout;

    /*
     * Register the package_id
     */
    ports0_set_package_id(package_id);
    
    /*
     * Initialize the various components
     */
    
    _p0_args_init(argc, argv);
    
#ifndef BUILD_LITE
    _p0_thread_init(argc, argv);
#endif /* BUILD_LITE */

#ifdef PORTS0_MALLOC_DEBUG    
    _p0_malloc_debug_init(argc, argv);
#endif

    _p0_ports0_init_called = PORTS0_TRUE;
    
} /* ports0_init() */


/*
 * ports0_shutdown()
 *
 * Shutdown Ports0 in this process, and also have it shutdown other
 * processes.
 */
int ports0_shutdown(void)
{
#ifdef BUILD_LITE
    return (0);
#else  /* BUILD_LITE */
    return(_p0_thread_shutdown());
#endif /* BUILD_LITE */
} /* ports0_shutdown() */
