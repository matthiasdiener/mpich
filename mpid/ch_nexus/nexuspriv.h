#ifndef _NEXUS_PRIV_H_
#define _NEXUS_PRIV_H_

#include "nexus.h"

extern nexus_startpoint_t *Nexus_nodes;
extern nexus_endpointattr_t default_ep_attr;
extern int *remote_formats;

#define SEND_CONTIG_HANDLER_ID    0
#define SSEND_DONE_ID             1
#define SEND_DATATYPE_HANDLER_ID  2
#define SSEND_DATATYPE_HANDLER_ID 3
#define INITIAL_NODES_HANDLER_ID  4
#define SEND_FORMATS_ID           5
#define RECEIVE_FORMATS_ID        6

#define HANDLER_TABLE_SIZE 7

#endif
