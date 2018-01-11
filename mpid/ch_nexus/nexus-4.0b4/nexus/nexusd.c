/*
 * nexusd.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nexusd.c,v 1.14 1996/10/07 04:40:04 tuecke Exp $";

#include "nexus.h"
#include <stdio.h>
#include <ctype.h>
#include "nx_c_hdr.h"

#ifdef NEXUS_ARCH_PVM
extern NEXUS_PROTOCOL_INFO_TYPE _nx_pvm_protocol_info \
    (NEXUS_PROTOCOL_INFO_ARGS);
nexus_protocol_list_t nexus_protocol_list[] =
{
    _nx_pvm_protocol_info,
    NULL
};

#elif defined (NEXUS_ARCH_PARAGON)
extern NEXUS_PROTOCOL_INFO_TYPE _nx_inx_protocol_info \
    (NEXUS_PROTOCOL_INFO_ARGS);
nexus_protocol_list_t nexus_protocol_list[] =
{
    _nx_inx_protocol_info,
    NULL
};

#else /* NEXUS_ARCH_PVM */

extern NEXUS_PROTOCOL_INFO_TYPE _nx_tcp_protocol_info \
    (NEXUS_PROTOCOL_INFO_ARGS);
nexus_protocol_list_t nexus_protocol_list[] =
{
    _nx_tcp_protocol_info,
    NULL
};
#endif /* NEXUS_ARCH_PVM */

/*
 * NexusExit()
 */
void NexusExit(void)
{
/*    nexus_printf("NexusExit(): entering\n"); */
/*    nexus_printf("NexusExit(): exiting\n"); */
} /* NexusExit() */


/*
 * NexusUnknownHandler()
 */
void NexusUnknownHandler(void *address, nexus_buffer_t *buffer,
			 char *handler_name, int handler_id)
{
    nexus_printf("NexusUnknownHandler(): handler_name=%s, handler_id=%d\n",
		 handler_name, handler_id);
    nexus_printf("NexusUnknownHandler(): exiting\n");
} /* NexusUnknownHandler() */


static nexus_handler_t system_handlers[] =
{ \
  {"NexusExit",
       NEXUS_EXIT_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NexusExit},
  {"NexusUnknownHandler",
       NEXUS_UNKNOWN_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NexusUnknownHandler},
  {(char *) NULL,
       0,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};

/*
 * NexusBoot()
 */
int NexusBoot(void)
{
    int rc;
    
/*    nexus_printf("NexusBoot(): entering\n"); */
    
    nexus_register_handlers(system_handlers);

    rc = 0;
    
/*    nexus_printf("NexusBoot(): exiting, returning %d\n", rc); */

    return (rc);
} /* NexusBoot() */


/*****************************************************************
 *		MAIN
 *****************************************************************/

/*
 * main()
 */
void main(int argc, char **argv)
{
    nexus_node_t *nodes;
    int n_nodes;
    int iwait4debug=1;
    int dopolling=1;
    
    nexus_init( (char *) NULL, /* args_env_variable */
	       (char *) NULL, /* package_designator */
	       (void (*)(void)) NULL, /* package_args_init_func */
	       (int (*)(int,int)) NULL, /* package_args_func */
	       (void (*)(void)) NULL, /* usage_message_func */
	       (int (*)(char *,int)) NULL /*new proc params func */,
	       &nodes,
	       &n_nodes
	       );

    nexus_start();

    while( dopolling ) nexus_poll( NEXUS_TRUE );

/*    nexus_printf("main(): Calling nexus_exit()\n"); */
    nexus_exit(0, NEXUS_TRUE);

} /* main() */
