#include "nexus.h"
#include <stdio.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	NEXUS_EZ_HANDLER_ID 0

typedef struct
{
  nexus_endpoint_t endpoint;
  nexus_mutex_t mutex;
  nexus_cond_t cond;
  nexus_bool_t done;
  char *reply_format;
  struct s_node_t *node;
} nexus_ez_rpchandle_t;

#ifdef __STDC__
int nexus_ez_rpc_unpack_1sided(nexus_buffer_t *buffer, char *format, ...);
#else
int nexus_ez_rpc_unpack_1sided();
#endif

#ifdef __STDC__
int nexus_ez_rpc_1sided(nexus_startpoint_t *sp, int handler_id, 
			nexus_bool_t called_from_nonthread_h,
			char *format, ...);
#else
int nexus_ez_rpc_1sided();
#endif

#ifdef __STDC__
int nexus_ez_asynch(nexus_startpoint_t *sp, int handler_id, 
			nexus_ez_rpchandle_t *rpchandle, 
			nexus_bool_t called_from_nonthread_h,
			char *format1,  ...); 
#else
int nexus_ez_asynch();
#endif

#ifdef __STDC__
int nexus_ez_rpc_unpack(nexus_buffer_t *buffer,
		        nexus_startpoint_t *reply_sp, 
			char *format, ...); 
#else
int nexus_ez_rpc_unpack();
#endif

int nexus_ez_rpc_wait(nexus_ez_rpchandle_t *rpchandle);

int nexus_ez_rpc_probe(nexus_ez_rpchandle_t rpchandle, int *status);

#ifdef __STDC__
int nexus_ez_rpc(nexus_startpoint_t *sp, int handler_id, 
			nexus_bool_t called_from_nonthread_h,
			char *format1, ...); 
#else
int nexus_ez_rpc();
#endif

void nexus_ez_destroy_rpchandle(nexus_ez_rpchandle_t *rpchandle);

