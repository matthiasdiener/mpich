/*
 * listener.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/listener.c,v 1.19 1996/10/07 04:39:57 tuecke Exp $";

#include "server.h"

#include "nexus_config.h"

#ifdef TARGET_ARCH_AIX
#define _ALL_SOURCE
#endif

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef TARGET_ARCH_NEXTSTEP
#include <libc.h>
#endif

#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_AXP) || \
	defined(TARGET_ARCH_LINUX) || defined(TARGET_ARCH_CRAYC90) || \
	defined(TARGET_ARCH_PARAGON)
#include <unistd.h>
#endif

#ifdef TARGET_ARCH_AXP
#include <stdlib.h>
#include <sys/param.h>
#endif

#ifdef TARGET_ARCH_SUNOS41
#include <malloc.h>
#include <unistd.h>
#include <sys/param.h>
#endif

#if defined(TARGET_ARCH_AIX)
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/access.h>
#include <sys/select.h>
#endif

#if defined(TARGET_ARCH_LINUX) || defined(TARGET_ARCH_FREEBSD)
#include <sys/param.h>
#endif

#ifdef TARGET_ARCH_PARAGON
#include <sys/param.h>
#include <sys/access.h>
#include <sys/errno.h>
#define NEXUS_ARGS "NexusArgs" /* Environment variable */
#endif

#ifdef TARGET_ARCH_SGI
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#endif

#ifdef TARGET_ARCH_HPUX
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#endif

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/time.h>

#define DEBUG_LEVEL 0
#if defined(BUILD_DEBUG) || defined(NEXUS_LISTENER_DEBUG)
#define nexus_dbgss_printf( operation, level, message ) \
    if (level <= DEBUG_LEVEL) \
    { \
	printf message ; \
    }
#else
#define nexus_dbgss_printf( operation, level, message )
#endif /* BUILD_DEBUG */

#define nexus_stdio_lock()
#define nexus_stdio_unlock()

#define SS_Fork() fork()
#define NEXUS_FALSE 0
#define NEXUS_TRUE  1

#include "ss_start.h"

/*
 * main()
 */
int main(int argc, char **argv)
{
    char **environment;
    char *message;
    int n_env;
    int i;

    n_env = atoi(argv[8]);
    environment = (char **)malloc(sizeof(char *) * (n_env + 1));

    for (i = 0; i < n_env; i++)
    {
	environment[i] = (char *)malloc(sizeof(char) * 256);
	strcpy(environment[i], argv[9 + i]);
    }
    environment[i] = (char *)NULL;

    if ((message = start_secure_remote_node(argv[1],
					    atoi(argv[2]),
					    argv[3],
					    argv[4],
					    environment,
					    n_env,
					    argv[5],
					    argv[6],
					    argv[7])) != (char *) NULL)
    {
	printf("listener: Failed to start node: %s\n", message);
	return(1);
    }
    else
    {
	return(0);
    }
} /* main() */
