/*
 * nx_c2c.h
 */

/* 
static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_examples/client_server/nx_c2c.h,v 1.4 1996/03/21 23:32:41 geisler Exp $";
*/

#ifndef URL_FILE
#define URL_FILE "attach_url"
#endif

#ifdef BUILD_DEBUG
#define debug_printf(A) nexus_printf A
#else
#define debug_printf(A)
#endif

extern nexus_bool_t write_attach_file(char *url, char *file);
extern nexus_bool_t read_attach_file(char *url, char *file);

#define BROKER_QUERY_CLIENTS  0
#define BROKER_QUERY_SESSIONS 1

#define	BROKER_SHUTDOWN_HANDLER_NAME "broker_shutdown_handler"
#define	BROKER_SHUTDOWN_HANDLER_HASH 419

#define BROKER_QUERY_HANDLER_NAME "broker_query_handler"
#define BROKER_QUERY_HANDLER_HASH 93

#define BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_NAME \
	"bcontrol_query_clients_reply_handler"
#define BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_HASH 794

#define BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_NAME \
	"bcontrol_query_sessions_reply_handler"
#define BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_HASH 927

