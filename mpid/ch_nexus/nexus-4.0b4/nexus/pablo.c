/*
 * pablo.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pablo.c,v 1.16 1996/10/07 04:40:06 tuecke Exp $";

#ifdef BUILD_PROFILE

#include "internal.h"

static char *DEFAULT_FILE_BASE = "pablo";

static nexus_bool_t	arg_profile;
static int		arg_profile_rsr;
static char *		arg_file_base;

static FILE *		pablo_fp;

#ifdef BUILD_LITE
#define pablo_lock()
#define pablo_unlock()
#else  /* BUILD_LITE */
static nexus_mutex_t	pablo_mutex;
#define pablo_lock()    nexus_mutex_lock(&pablo_mutex)
#define pablo_unlock()  nexus_mutex_unlock(&pablo_mutex)
#endif /* BUILD_LITE */


/******************************************************************/

#define print_lognum(numtype,numval) \
	fprintf(pablo_fp,", %d",numval);

#define print_logstr(strtype,strval) \
	{ \
		int len=strlen(strval); \
		len++; \
		fprintf(pablo_fp,"\n     [%d]{\n     \"%s\"\n     }",len,strval); \
	}

#define print_logtime() \
	{\
		double dtime = nexus_wallclock(); \
		unsigned long sec=(unsigned long) dtime; \
		unsigned long usec=(unsigned long) ((dtime-sec)*1000000); \
		fprintf(pablo_fp,"\n     [2] {\n     %lu,\n     %lu\n     }, %f",sec, usec, dtime); \
	}

#define print_logend() \
	fprintf(pablo_fp," };;\n\n");

void print_logbegin(_nx_pablo_log_event_type eventtype)
{
     switch(eventtype) {
       case _NX_PABLO_NODE_CREATION: 
	      fprintf(pablo_fp,"\"Node Creation\" {"); break;
       case _NX_PABLO_NODE_DESTRUCTION:
	      fprintf(pablo_fp,"\"Node Destruction\" {"); break;
       case _NX_PABLO_NODE_COUNT:
	      fprintf(pablo_fp,"\"Node Count\" {"); break;
       case _NX_PABLO_CONTEXT_CREATION:
	      fprintf(pablo_fp,"\"Context Creation\" {"); break;
       case _NX_PABLO_CONTEXT_DESTRUCTION:
	      fprintf(pablo_fp,"\"Context Destruction\" {"); break;
       case _NX_PABLO_CONTEXT_COUNT:
	      fprintf(pablo_fp,"\"Context Count\" {"); break;
       case _NX_PABLO_THREAD_CREATION:
	      fprintf(pablo_fp,"\"Thread Creation\" {"); break;
       case _NX_PABLO_THREAD_DESTRUCTION:
	      fprintf(pablo_fp,"\"Thread Destruction\" {"); break;
       case _NX_PABLO_THREAD_COUNT:
	      fprintf(pablo_fp,"\"Thread Count\" {"); break;
       case _NX_PABLO_REMOTE_SERVICE_REQUEST_SEND:
	      fprintf(pablo_fp,"\"Remote Service Request Send\" {"); break;
       case _NX_PABLO_REMOTE_SERVICE_REQUEST_RECEIVE:
	      fprintf(pablo_fp,"\"Remote Service Request Receive\" {"); break;
       case _NX_PABLO_REMOTE_SERVICE_REQUEST_COUNT:
	      fprintf(pablo_fp,"\"Remote Service Request Count\" {"); break;
     }
}


/*
 * _nx_pablo_usage_message()
 */
void _nx_pablo_usage_message(void)
{
    printf("    -profile                  : Turn on profiling.\n");
    printf("    -prsr <integer>           : Method of remote service request profiling:\n");
    printf("                                  0 : log each rsr send and receive\n");
    printf("                                 -1 : keep cumulative profile, and dump\n");
    printf("                                      only at end (default)\n");
    printf("                                 >0 : keep cumulative profile, and dump\n");
    printf("                                      after every <integer> rsr receives\n");
    printf("    -pablofile <file>         : Dump SDDF data into file_#id.\n");
} /* _nx_pablo_usage_message() */


/*
 * _nx_pablo_new_process_params()
 */
int _nx_pablo_new_process_params(char *buf, int size)
{
    char tmp_buf1[1024];
    char tmp_buf2[1024];
    int n_added;
 
    tmp_buf1[0] = '\0';

    nexus_stdio_lock();
    
    if (arg_profile)
    {
	strcpy(tmp_buf1, "-profile ");
    }

    if (arg_profile_rsr > -2)
    {
	sprintf(tmp_buf2, "-prsr %d ", arg_profile_rsr);
	strcat(tmp_buf1, tmp_buf2);
    }
    
    if (arg_file_base != (char *) NULL)
    {
	sprintf(tmp_buf2, "-pablofile %s ", arg_file_base);
	strcat(tmp_buf1, tmp_buf2);
    }

    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
    nexus_fatal("_nx_pablo_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
 
    strcpy(buf, tmp_buf1);

    nexus_stdio_unlock();

    return (n_added);
 
} /* _nx_pablo_new_process_params() */


/*
 * _nx_pablo_init()
 */
void _nx_pablo_init(int *argc, char ***argv)
{
    char buf[1024];
    int node_id;
    int process_id;
    int arg_num;

    arg_profile = NEXUS_FALSE;
    arg_profile_rsr = -2;
    arg_file_base=NULL;

    if ((arg_num = nexus_find_argument(argc, argv, "profile", 2)) >= 0)
    {
	arg_profile = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "prsr", 2)) >= 0)
    {
        arg_profile_rsr = NEXUS_MAX(atoi((*argv)[arg_num + 1]), -2);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "pablofile", 2)) >= 0)
    {
        arg_file_base = (*argv)[arg_num + 1];
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }

    _nx_do_profile = arg_profile;

    if (arg_profile_rsr > -2)
	_nx_rsr_profile_accumulate = arg_profile_rsr;
    else
	_nx_rsr_profile_accumulate = -1;
    
    if (!_nx_do_profile)
	return;

#ifndef BUILD_LITE
    nexus_mutex_init(&pablo_mutex, (nexus_mutexattr_t *) NULL);
#endif /* BUILD_LITE */
    
    _nx_node_id(&node_id);
    process_id = _nx_md_getpid();
 
    if (arg_file_base == (char *) NULL)
    {
        arg_file_base = DEFAULT_FILE_BASE;
    }
 
    sprintf(buf, "%s_%d_%d.log", arg_file_base, node_id, process_id);
 
    if ((pablo_fp = fopen(buf, "w")) == (FILE *) NULL)
    {
        nexus_fatal("_nx_pablo_init(): Unable to open SDDF data file: %s\n",
		    buf);
    }
} /* _nx_pablo_init() */


/*
 * _nx_pablo_shutdown()
 */
void _nx_pablo_shutdown(void)
{
    if (!_nx_do_profile)
	return;

    fclose(pablo_fp);
} /* _nx_pablo_shutdown() */


/*****************************************************************/

void _nx_pablo_log_node_creation(int node_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "NodeCreate");
    fprintf(pablo_fp, " %d", _NX_PABLO_NODE_CREATION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_NODE_CREATION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_NODE_CREATION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_node_destruction(int node_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "NodeDest");
    fprintf(pablo_fp, " %d", _NX_PABLO_NODE_DESTRUCTION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_NODE_DESTRUCTION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_NODE_DESTRUCTION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_node_count(int count, int snapshot_id, 
			      int node_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "NodeCount");
    fprintf(pablo_fp, " %d", _NX_PABLO_NODE_COUNT);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", snapshot_id);
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", count);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_NODE_COUNT);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_NODE_COUNT);
    print_lognum(_NX_PABLO_SNAPSHOT_ID,snapshot_id);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_COUNT,count);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_context_creation(int node_id, int context_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "CntxtCreate");
    fprintf(pablo_fp, " %d", _NX_PABLO_CONTEXT_CREATION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_CONTEXT_CREATION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_CONTEXT_CREATION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_context_destruction(int node_id, int context_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "CntxtDest");
    fprintf(pablo_fp, " %d", _NX_PABLO_CONTEXT_DESTRUCTION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_CONTEXT_DESTRUCTION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_CONTEXT_DESTRUCTION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_context_count(int count, int snapshot_id, 
				 int node_id, int context_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "CntxtCount");
    fprintf(pablo_fp, " %d", _NX_PABLO_NODE_DESTRUCTION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", snapshot_id);
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, " %d", count);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_CONTEXT_COUNT);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_CONTEXT_COUNT);
    print_lognum(_NX_PABLO_SNAPSHOT_ID,snapshot_id);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_lognum(_NX_PABLO_COUNT,count);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_thread_creation(int node_id, int context_id,
				   int thread_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "ThreadCreate");
    fprintf(pablo_fp, " %d", _NX_PABLO_THREAD_CREATION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, " %d", thread_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_THREAD_CREATION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_THREAD_CREATION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_lognum(_NX_PABLO_THREAD_ID,thread_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_thread_destruction(int node_id,
				      int context_id, int thread_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "ThreadDest");
    fprintf(pablo_fp, " %d", _NX_PABLO_THREAD_DESTRUCTION);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, " %d", thread_id);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_THREAD_DESTRUCTION);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_THREAD_DESTRUCTION);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_lognum(_NX_PABLO_THREAD_ID,thread_id);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_thread_count(int count, int snapshot_id,
				int node_id, int context_id, int thread_id)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "ThreadCount");
    fprintf(pablo_fp, " %d", _NX_PABLO_THREAD_COUNT);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", snapshot_id);
    fprintf(pablo_fp, " %d", node_id);
    fprintf(pablo_fp, " %d", context_id);
    fprintf(pablo_fp, " %d", thread_id);
    fprintf(pablo_fp, " %d", count);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_THREAD_COUNT);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_THREAD_COUNT);
    print_lognum(_NX_PABLO_SNAPSHOT_ID,snapshot_id);
    print_lognum(_NX_PABLO_NODE_ID,node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,context_id);
    print_lognum(_NX_PABLO_THREAD_ID,thread_id);
    print_lognum(_NX_PABLO_COUNT,count);
    print_logend();
#endif
    pablo_unlock();
}


void _nx_pablo_log_remote_service_request_send(int d_node_id, 
	int d_context_id, char * handler_name, int handler_id, int message_len)
{
    if (!_nx_do_profile || (_nx_rsr_profile_accumulate != 0))
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "RSRSend");
    fprintf(pablo_fp, " %d", _NX_PABLO_REMOTE_SERVICE_REQUEST_SEND);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", d_node_id);
    fprintf(pablo_fp, " %d", d_context_id);
    fprintf(pablo_fp, " %d %s", strlen(handler_name), handler_name);
    fprintf(pablo_fp, " %d", handler_id);
    fprintf(pablo_fp, " %d", message_len);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_REMOTE_SERVICE_REQUEST_SEND);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_REMOTE_SERVICE_REQUEST_SEND);
    print_lognum(_NX_PABLO_NODE_ID,d_node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,d_context_id);
    print_logstr(_NX_PABLO_HANDLER_NAME,handler_name);
    print_lognum(_NX_PABLO_HANDLER_ID,handler_id);
    print_lognum(_NX_PABLO_MESSAGE_LEN,message_len);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_remote_service_request_receive(int s_node_id, 
	int s_context_id, char * handler_name, int handler_id, int message_len)
{
    if (!_nx_do_profile || (_nx_rsr_profile_accumulate != 0))
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "RSRRecv");
    fprintf(pablo_fp, " %d", _NX_PABLO_REMOTE_SERVICE_REQUEST_RECEIVE);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", s_node_id);
    fprintf(pablo_fp, " %d", s_context_id);
    fprintf(pablo_fp, " %d %s", strlen(handler_name), handler_name);
    fprintf(pablo_fp, " %d", handler_id);
    fprintf(pablo_fp, " %d", message_len);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_REMOTE_SERVICE_REQUEST_RECEIVE);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_REMOTE_SERVICE_REQUEST_RECEIVE);
    print_lognum(_NX_PABLO_NODE_ID,s_node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,s_context_id);
    print_logstr(_NX_PABLO_HANDLER_NAME,handler_name);
    print_lognum(_NX_PABLO_HANDLER_ID,handler_id);
    print_lognum(_NX_PABLO_MESSAGE_LEN,message_len);
    print_logend();
#endif
    pablo_unlock();
}

void _nx_pablo_log_remote_service_request_count(int count, 
	int snapshot_id, int s_node_id, int s_context_id, int d_node_id, 
	int d_context_id, char * handler_name, int handler_id, int message_len)
{
    if (!_nx_do_profile)
	return;
    pablo_lock();
#ifdef JMPAVS
    fprintf(pablo_fp, "RSRSend");
    fprintf(pablo_fp, " %d", _NX_PABLO_REMOTE_SERVICE_REQUEST_SEND);
    fprintf(pablo_fp, " %f", nexus_wallclock());
    fprintf(pablo_fp, " %d", snapshot_id);
    fprintf(pablo_fp, " %d", s_node_id);
    fprintf(pablo_fp, " %d", s_context_id);
    fprintf(pablo_fp, " %d", d_node_id);
    fprintf(pablo_fp, " %d", d_context_id);
    fprintf(pablo_fp, " %d %s", strlen(handler_name), handler_name);
    fprintf(pablo_fp, " %d", handler_id);
    fprintf(pablo_fp, " %d", message_len);
    fprintf(pablo_fp, " %d", count);
    fprintf(pablo_fp, "\n");
#else
    print_logbegin(_NX_PABLO_REMOTE_SERVICE_REQUEST_COUNT);
    print_logtime();
    print_lognum(_NX_PABLO_EVENT_ID,_NX_PABLO_REMOTE_SERVICE_REQUEST_COUNT);
    print_lognum(_NX_PABLO_SNAPSHOT_ID,snapshot_id);
    print_lognum(_NX_PABLO_NODE_ID,s_node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,s_context_id);
    print_lognum(_NX_PABLO_NODE_ID,d_node_id);
    print_lognum(_NX_PABLO_CONTEXT_ID,d_context_id);
    print_logstr(_NX_PABLO_HANDLER_NAME,handler_name);
    print_lognum(_NX_PABLO_HANDLER_ID,handler_id);
    print_lognum(_NX_PABLO_MESSAGE_LEN,message_len);
    print_lognum(_NX_PABLO_COUNT,count);
    print_logend();
#endif
    pablo_unlock();
}

#endif /* BUILD_PROFILE */
