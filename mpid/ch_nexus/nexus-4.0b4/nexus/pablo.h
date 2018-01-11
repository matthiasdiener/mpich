/*
 * pablo.h
 *
 * rcsid ="$Header: /nfs/globus1/src/master/nexus_source/pablo.h,v 1.10 1996/10/07 04:40:06 tuecke Exp $" 
 */

#ifndef _NEXUS_PABLO_H
#define _NEXUS_PABLO_H

NEXUS_GLOBAL nexus_bool_t	_nx_do_profile;
NEXUS_GLOBAL int	_nx_rsr_profile_accumulate;

#define _nx_pablo_ASCII      "SDDFA"
#define _nx_pablo_BINARY     "SDDFB"

typedef enum _nx_pablo_log_event_types {
  _NX_PABLO_NODE_CREATION,
  _NX_PABLO_NODE_DESTRUCTION,
  _NX_PABLO_NODE_COUNT,
  _NX_PABLO_CONTEXT_CREATION,
  _NX_PABLO_CONTEXT_DESTRUCTION,
  _NX_PABLO_CONTEXT_COUNT,
  _NX_PABLO_THREAD_CREATION,
  _NX_PABLO_THREAD_DESTRUCTION,
  _NX_PABLO_THREAD_COUNT,
  _NX_PABLO_REMOTE_SERVICE_REQUEST_SEND,
  _NX_PABLO_REMOTE_SERVICE_REQUEST_RECEIVE,
  _NX_PABLO_REMOTE_SERVICE_REQUEST_COUNT
} _nx_pablo_log_event_type;

typedef enum _nx_pablo_log_num_types {
  _NX_PABLO_EVENT_ID,
  _NX_PABLO_SNAPSHOT_ID,
  _NX_PABLO_NODE_ID,
  _NX_PABLO_CONTEXT_ID,
  _NX_PABLO_THREAD_ID,
  _NX_PABLO_HANDLER_ID,
  _NX_PABLO_MESSAGE_LEN,
  _NX_PABLO_COUNT
} _nx_pablo_log_num_type;

extern void _nx_pablo_usage_message(void);
extern int  _nx_pablo_new_process_params(char *buf, int size);
extern void _nx_pablo_init(int *argc, char ***argv);
extern void _nx_pablo_shutdown(void);
extern void _nx_pablo_log_node_creation(int node_id);
extern void _nx_pablo_log_node_destruction(int node_id);
extern void _nx_pablo_log_node_count(int count, int snapshot_id,
                  int node_id);
 
extern void _nx_pablo_log_context_creation(int node_id,
                  int context_id);
extern void _nx_pablo_log_context_destruction(int node_id,
                  int context_id);
extern void _nx_pablo_log_context_count(int count,
                  int snapshot_id, int node_id, int context_id);
 
extern void _nx_pablo_log_thread_creation(int node_id,
                  int context_id, int thread_id);
extern void _nx_pablo_log_thread_destruction(int node_id,
                  int context_id, int thread_id);
extern void _nx_pablo_log_thread_count(int count, int snapshot_id,
                  int node_id, int context_id, int thread_id);
 
extern void _nx_pablo_log_remote_service_request_send(int d_node_id,
                  int d_context_id, char * handler_name, int handler_id, 
                  int message_len);
extern void _nx_pablo_log_remote_service_request_receive(int s_node_id,
                  int s_context_id, char * handler_name, int handler_id, 
                  int message_len);
extern void _nx_pablo_log_remote_service_request_count(int count,
                  int snapshot_id, int s_node_id, int s_context_id,
                  int d_node_id, int d_context_id, char * handler_name,
                  int handler_id, int message_len);

#define _nx_pablo_count_remote_service_requests() \
    (_nx_do_profile && (_nx_rsr_profile_accumulate != 0))

#define _nx_pablo_should_dump_remote_service_request_counts(Count) \
    ((_nx_rsr_profile_accumulate > 0)&&((Count) >= _nx_rsr_profile_accumulate))

#endif
