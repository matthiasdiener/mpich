/*
 * Author:      John W. Garnett
 *              California Institute of Technology
 *              Compositional C++ Group
 *              1994 May 11
 *
 * pr_pvm.c		- PVM 3.2.6 protocol module for Nexus 1.9
 *                (based on the pr_local and pr_tcp protocol modules
 *                 by Steve Tuecke and Robert Olson)
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_pvm.c,v 1.18 1996/10/07 04:40:09 tuecke Exp $";

#undef IPVM_PROTO_SINGLE_THREADED

#include "pr_pvm.h"

/*
   todo: test the single-threaded mode.
   todo: test propagation of shutdowns and aborts.
   todo: shutdown all pvmd processes once everyone has called pvm_exit()?
   todo: use ipvm_done flag to tell handler thread to shutdown.
   todo: make the thread-safe version work.
*/

/*
 * Thread is handler?
 *
 * Thread specific storage is used to keep track of whether the current
 * thread is a handler thread or not.
 */
static nexus_thread_key_t i_am_handler_thread_key;

#ifndef IPVM_PROTO_SINGLE_THREADED

static nexus_mutex_t ipvm_mutex;
static nexus_cond_t ipvm_cond;
static nexus_bool_t ipvm_in_use;
static int ipvm_threads_waiting;
static nexus_bool_t ipvm_done;
static nexus_bool_t handler_thread_done;
static nexus_mutex_t handler_thread_done_mutex;
static nexus_cond_t handler_thread_done_cond;

#endif /* IPVM_PROTO_SINGLE_THREADED */

static char ipvm_local_host[MAXHOSTNAMELEN];
static int ipvm_local_host_length;
static int ipvm_local_tid;
static int ipvm_creator_tid;
static int ipvm_local_pid;
static int ipvm_aborting = 0;

/*
 * A ipvm_buffer_t free list, to avoid malloc calls on the
 * main body of a message buffer.
 */
static ipvm_buffer_t *buffer_free_list = (ipvm_buffer_t *) NULL;

#ifndef IPVM_PROTO_SINGLE_THREADED
static nexus_mutex_t	buffer_free_list_mutex;
#endif

static ipvm_incoming_t *incoming_free_list = (ipvm_incoming_t *) NULL;

/* -- function tables -- */

static nexus_proto_funcs_t ipvm_proto_funcs =
{
    ipvm_process_arguments_init,
    ipvm_process_arguments,
    ipvm_usage_message,
    ipvm_new_process_params,
    ipvm_init,
    ipvm_shutdown,
    ipvm_abort,
    ipvm_poll,
    ipvm_init_remote_service_request,
    ipvm_destroy_proto,
    ipvm_get_my_mi_proto,
    ipvm_construct_from_mi_proto,
    ipvm_construct_creator_proto,
    ipvm_compare_protos,
    NULL /* ipvm_test_proto */,
};

static nexus_buffer_funcs_t ipvm_buffer_funcs =
{
    ipvm_set_buffer_size,
    ipvm_check_buffer_size,
    ipvm_send_remote_service_request,
    ipvm_send_urgent_remote_service_request, 
    ipvm_free_buffer,
    ipvm_stash_buffer,
    ipvm_free_stashed_buffer,
    ipvm_sizeof_float,
    ipvm_sizeof_double,
    ipvm_sizeof_short,
    ipvm_sizeof_u_short,
    ipvm_sizeof_int,
    ipvm_sizeof_u_int,
    ipvm_sizeof_long,
    ipvm_sizeof_u_long,
    ipvm_sizeof_char,
    ipvm_sizeof_u_char,
    ipvm_put_float,
    ipvm_put_double,
    ipvm_put_short,
    ipvm_put_u_short,
    ipvm_put_int,
    ipvm_put_u_int,
    ipvm_put_long,
    ipvm_put_u_long,
    ipvm_put_char,
    ipvm_put_u_char,
    ipvm_get_float,
    ipvm_get_double,
    ipvm_get_short,
    ipvm_get_u_short,
    ipvm_get_int,
    ipvm_get_u_int,
    ipvm_get_long,
    ipvm_get_u_long,
    ipvm_get_char,
    ipvm_get_u_char,
    ipvm_get_stashed_float,
    ipvm_get_stashed_double,
    ipvm_get_stashed_short,
    ipvm_get_stashed_u_short,
    ipvm_get_stashed_int,
    ipvm_get_stashed_u_int,
    ipvm_get_stashed_long,
    ipvm_get_stashed_u_long,
    ipvm_get_stashed_char,
    ipvm_get_stashed_u_char,
};

/* these need to be non-static so that st_pvm.c can use them */

void _nx_ipvm_enter(void)
{
	ipvm_enter();
}

void _nx_ipvm_exit(void)
{
	ipvm_exit();
}

/*
   nx_pvm_send_command():
     send 'message' as a command to every process that this process created.
*/

static void nx_pvm_send_command(int target, char *caller, int command)
{
	char flag = (char)command;
	int bufid, info;

	nexus_debug_printf(2, ("[nx_pvm_send_command]\n"));
	ipvm_enter();
	bufid = pvm_mkbuf(PvmDataRaw);
	IPVM_TEST(bufid, caller);
	info = pvm_setsbuf(bufid);
	IPVM_TEST(info, caller);
	info = pvm_pkbyte(&flag, 1, 1);
	IPVM_TEST(info, caller);
	info = pvm_send(target, COMMAND_TAG);
	IPVM_TEST(info, caller);
	info = pvm_setsbuf(0);
	IPVM_TEST(info, caller);
	info = pvm_freebuf(bufid);
	IPVM_TEST(info, caller);
	ipvm_exit();
}

static void nx_get_string(nexus_buffer_t* buf, char *str)
{
	int str_length;

	ipvm_get_int(buf, &str_length, 1);
	ipvm_get_char(buf, str, str_length);
	str[str_length] = '\0';
}

static ipvm_buffer_t* make_recv_buffer(int bufferId)
{
	ipvm_buffer_t* buf;

	GetPvmBuffer(make_recv_buffer(), buf);
	buf->next = (ipvm_buffer_t*)0;
	buf->funcs = &ipvm_buffer_funcs;
	buf->buffer_type = RECV_BUFFER;
	buf->stashed = NEXUS_FALSE;
	buf->bufid = bufferId;
	return buf;
}

static ipvm_buffer_t* make_send_buffer(int xlate, nexus_global_pointer_t *gp)
{
	ipvm_buffer_t* buf;

	GetPvmBuffer(make_send_buffer(), buf);
	buf->next = (ipvm_buffer_t*)0;
	buf->funcs = &ipvm_buffer_funcs;
	buf->gp = gp;
	buf->buffer_type = SEND_BUFFER;
	buf->stashed = NEXUS_FALSE;
	ipvm_enter();
	/* todo: use PvmDataInPlace when supported (especially on the Cray T3D). */
	buf->bufid = pvm_mkbuf((xlate) ? PvmDataDefault : PvmDataRaw);
	IPVM_TEST(buf->bufid, "make_send_buffer");
	pvm_setsbuf(0);
	ipvm_exit();
	return buf;
}

/*
 * _nx_pvm_protocol_info()
 *
 * Return the proto_type and the nexus_proto_funcs_t for this protocol
 * module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the info it needs to use this protocol module.
 */
void _nx_pvm_protocol_info(nexus_proto_type_t *proto_type,
			   nexus_proto_funcs_t **funcs)
{
    *proto_type = NEXUS_PROTO_TYPE_PVM;
    *funcs = &(ipvm_proto_funcs);
}

/*
 * ipvm_process_arguments_init()
 *
 * Initialize any data structures that will hold command line arguments.
 */
static void ipvm_process_arguments_init(void)
{
	/* none needed: using pvm_parent() */
}

/*
 * ipvm_process_arguments()
 *
 * See if the 'current_arg' argument (of the package arguments)
 * is meant for us.  If so, then extract it, and return a
 * new 'current_arg' value.  If not, then just return 'current_arg'
 * unchanged.
 */
static int ipvm_process_arguments(int current_arg, int arg_count)
{
	/* none needed */
	return current_arg;
}

/*
 * ipvm_usage_message()
 */
static void ipvm_usage_message(void)
{
	printf("    No flags supported.\n");
}

/*
 * ipvm_new_process_params()
 */
static int ipvm_new_process_params(char *buf, int size)
{
	/* none needed */
	return 0;
}

static int select_and_read(void)
{
	int bufid;
	char command = 0;

	while (1) {
		ipvm_enter();
#if defined(IPVM_PROTO_SINGLE_THREADED) || !defined(NEXUS_THREAD_SAFE_PVM_RECV)
		if (ipvm_done) {
			command = CLOSE_SELF_SHUTDOWN_FLAG;
			break; /* exit the while loop */
		}
		bufid = pvm_nrecv(-1, -1); /* non-blocking receive */
		IPVM_TEST(bufid, "select_and_read(non-blocking)");
#else
		bufid = pvm_recv(-1, -1);  /* blocking receive (only if thread-safe) */
		IPVM_TEST(bufid, "select_and_read(blocking)");
#endif
		pvm_setrbuf(0);
		ipvm_exit();
		if (bufid > 0) {
			int bytes, msg_tag, tid;

			nexus_debug_printf(3, ("select_and_read: bufid = %d\n", bufid));
			ipvm_enter();
			pvm_bufinfo(bufid, &bytes, &msg_tag, &tid);
			ipvm_exit();
			nexus_debug_printf(3, ("select_and_read: msg_tag = %d\n", msg_tag));
			if (msg_tag == HANDLER_TAG) {
				char handler_name[NEXUS_MAX_HANDLER_NAME_LENGTH];
				unsigned long context, address;
				nexus_handler_type_t handler_type;
				nexus_handler_func_t handler_func;
				nexus_global_pointer_t gp;
				ipvm_buffer_t* buffer;
				nexus_buffer_t* nx_buf;
				int handler_id;
#ifdef BUILD_PROFILE
				int node_id, context_id;
#endif

				buffer = make_recv_buffer(bufid);
				nx_buf = (nexus_buffer_t*)&buffer;
				ipvm_get_u_long(nx_buf, &context, 1);
				ipvm_get_u_long(nx_buf, &address, 1);
#ifdef BUILD_PROFILE
				ipvm_get_int(nx_buf, &node_id, 1);
				ipvm_get_int(nx_buf, &context_id, 1);
#endif
				ipvm_get_int(nx_buf, &handler_id, 1);
				nx_get_string(nx_buf, handler_name);
				nexus_debug_printf(2, ("select_and_read: looking for %s\n",
					handler_name));
				_nx_lookup_handler(context,
					handler_name,
					handler_id,
					&handler_type,
					&handler_func);
				if (handler_type == NEXUS_HANDLER_TYPE_THREADED) {
					buffer->stashed = NEXUS_TRUE;
				}
				nexus_debug_printf(2,
("_nx_handle_message: name = %s, id = %d, type = %d, func = %d, context = %lu, address = %lu, buffer = %x\n",
		handler_name, handler_id, (int)handler_type, (int)handler_func, context, address, (int)buffer));
				_nx_handle_message(handler_name,
					handler_id,
					handler_type,
					handler_func,
					context,
					address,
#ifdef BUILD_PROFILE
					node_id,
					context_id,
					bytes,
#endif
					(void *) buffer);
				nexus_debug_printf(2, ("select_and_read: finished with %s\n",
					handler_name));
			} else if (msg_tag == COMMAND_TAG) {
				nexus_debug_printf(3, ("select_and_read: COMMAND_TAG\n"));
				ipvm_enter();
				pvm_setrbuf(bufid);
				pvm_upkbyte(&command, 1, 1);
				pvm_setrbuf(0);
				pvm_freebuf(bufid);
				ipvm_exit();
				break; /* exit the while(1) loop */
			}
		} else { /* bufid == 0 */
#ifdef IPVM_PROTO_SINGLE_THREADED
			break; /* exiting as soon as select failed */
#endif
		}
#ifndef IPVM_PROTO_SINGLE_THREADED
		nexus_thread_yield(); /* give the other threads a chance to run */
#endif
	}
	return (int)command;
}

static void ipvm_process_command(int caller, int command)
{
	switch (command) {
		case CLOSE_SHUTDOWN_FLAG :
			/* A "close" was recv'd from some other process. */
			nexus_debug_printf(1,
				("ipvm_process_command: close_shutdown from other.\n"));
			/* cause our ipvm_shutdown() to be called but don't propagate. */
			/* todo: change to NEXUS_FALSE... see nexus_exit() and
			   finish_shutdown().
			*/
			nexus_exit(0, NEXUS_TRUE);
			break;
		case CLOSE_SELF_SHUTDOWN_FLAG :
			nexus_debug_printf(1,
				("ipvm_process_command: close_shutdown from self.\n"));
			break;
		case CLOSE_SELF_ABNORMAL_FLAG :
			nexus_debug_printf(1,
				("ipvm_process_command: close_abnormal from self.\n"));
			if (!ipvm_aborting) {
				ipvm_aborting = 1;
				nexus_debug_printf(1,
					("ipvm_process_command: calling pvm_halt().\n"));
				pvm_halt(); /* burn off all pvmd's and pvm tasks */
			}
			break;
		case CLOSE_ABNORMAL_FLAG :
			nexus_debug_printf(1,
				("ipvm_process_command: close_abnormal from other.\n"));
			nexus_debug_printf(1,
					("ipvm_process_command: Calling nexus_silent_fatal()\n"));
			ipvm_aborting = 1;
			nexus_silent_fatal(); /* cause ipvm_abort() to be called. */
			break;
		default :
			if ((caller == IPVM_POLL) && (command != 0)) {
				nexus_debug_printf(1,
					("ipvm_process_command: unexpected exit.\n"));
			}
			break;
	}
}

/*
 * ipvm_poll()
 *
 * In a version of the PVM protocol module that is _not_ threaded,
 * all handlers are invoked from this routine.
 *
 * If threads are supported, then ipvm_poll() is a no-op, since
 * one or more threads can be created which just sit waiting
 * for remote service requests and invoking handlers as required.
 */
static void ipvm_poll(void)
{
	nexus_debug_printf(2, ("[ipvm_poll]\n"));
#ifdef IPVM_PROTO_SINGLE_THREADED
	int command;

	command = select_and_read();
	ipvm_process_command(IPVM_POLL, command);
	nexus_debug_printf(2, ("ipvm_poll: exiting\n"));
#endif /* IPVM_PROTO_SINGLE_THREADED */
}

#ifndef IPVM_PROTO_SINGLE_THREADED

/*
 * ipvm_handler_thread()
 *
 * In the multi-threaded version, this is the entry point
 * for the handler thread.
 */
static void *ipvm_handler_thread(void *arg)
{
	int command = 0;

	_nx_set_i_am_handler_thread();
#ifdef BUILD_DEBUG
	if (NexusDebug(1)) {
		nexus_bool_t i_am;

		_nx_i_am_handler_thread(&i_am);
		nexus_printf("ipvm_handler_thread: i_am_handler_thread = %d\n", i_am);
	}
#endif

	command = select_and_read();

	if (!ipvm_done) {
		nexus_debug_printf(2,
		("ipvm_handler_thread: unlocking handler_thread_done_mutex.\n"));
		nexus_mutex_lock(&handler_thread_done_mutex);
		handler_thread_done = NEXUS_TRUE;
		nexus_cond_signal(&handler_thread_done_cond);
		nexus_mutex_unlock(&handler_thread_done_mutex);
		nexus_debug_printf(2,
		("ipvm_handler_thread: handler_thread_done_mutex unlocked.\n"));
	}

	ipvm_process_command(IPVM_HANDLER_THREAD, command);
	return ((void *) NULL);
}

#endif /* #ifndef IPVM_PROTO_SINGLE_THREADED */

/*
 *  nx_start_pvmd(): start a local pvmd and block until it is started.
 *  Specify a search path of "/" and "$cwd".  This is a hack but is
 *  necessary because PVM does not seem to provide any way to start
 *  a given executable as specified by a fully qualified path.  Having
 *  "/" in the path lets us pvm_spawn() fully qualified executables
 *  since //bin/sh is the same as /bin/sh to UNIX.
 */
 
static void nx_start_pvmd(void)
{
	FILE *fp;
	int info, rc;
	char *argv[1];
	char hostfile[MAX_PATH_LENGTH];
	char current_directory[MAX_PATH_LENGTH];

	if (getwd(current_directory) == 0) {
		strcpy(current_directory, ".");
	} else {
		_nx_strip_tmp_mnt_from_path(current_directory);
	}
	tmpnam(hostfile);
	fp = fopen(hostfile, "w");
	if (fp == NULL) {
		nexus_fatal("nx_start_pvmd: Unable to create a PVM hostfile: %s\n",
			hostfile);
	}
	nexus_stdio_lock();
	fprintf(fp, "# temporary PVM hostfile auto-generated by Nexus.\n");
	fprintf(fp, "* ep=/:%s\n", current_directory);
	nexus_stdio_unlock();
	nexus_debug_printf(2, ("nx_start_pvmd: * ep=/:%s\n", current_directory));
	fclose(fp);
	argv[0] = hostfile;
	argv[1] = (char *)NULL;
	/* turn off libpvm error reporting temporarily */
	rc = pvm_setopt(PvmAutoErr, 0);
	info = pvm_start_pvmd(1, argv, 1);
#ifdef BUILD_DEBUG
	if (info == PvmDupHost) {
		nexus_debug_printf(1, ("nx_start_pvmd: pvmd already running.\n"));
	}
#endif
	/* its ok if a pvmd is already running */
	if ((info < 0) && (info != PvmDupHost)) {
		IPVM_TEST(info, "nx_start_pvmd");
		nexus_fatal("nx_start_pvmd: Unable to start a PVM daemon (pvmd).\n");
	}
	/* reset libpvm error reporting to the way it was */
	pvm_setopt(PvmAutoErr, rc);
	/* todo: make sure this tmpfile is removed even if a crash occurs. */
	unlink(hostfile);
}

/*
 * ipvm_init()
 *
 * Note: no ipvm_enter() or ipvm_exit() locking is needed until _after_
 *       more than one thread exists (after the spawning of the
 *       message-handler).
 */
static void ipvm_init(void)
{
#ifndef IPVM_PROTO_SINGLE_THREADED
	nexus_thread_t thread;
#endif
	int info;

	nexus_debug_printf(2, ("[ipvm_init]\n"));
	ipvm_in_use = NEXUS_FALSE;
	ipvm_done = NEXUS_FALSE;
	ipvm_in_use = NEXUS_FALSE;
	handler_thread_done = NEXUS_FALSE;
	ipvm_threads_waiting = 0;
	nexus_thread_key_create(&i_am_handler_thread_key, NULL);
	nexus_mutex_init(&ipvm_mutex, (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&ipvm_cond, (nexus_condattr_t *) NULL);
	nexus_mutex_init(&handler_thread_done_mutex, (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&handler_thread_done_cond, (nexus_condattr_t *) NULL);
	nexus_mutex_init(&buffer_free_list_mutex, (nexus_mutexattr_t *) NULL);

	/* if we are the master node, then fire up a PVM daemon */
	if (nexus_master_node()) {
		nx_start_pvmd();
	}
#ifdef BUILD_DEBUG
	/* tell libpvm to print messages when errors occur */
	info = pvm_setopt(PvmAutoErr, 1);
#else
	/* squelch libpvm-generated error messages (rely on IPVM_TEST()) */
	/* todo: allow this to be turned on via an environment variable? */
	info = pvm_setopt(PvmAutoErr, 0);
#endif
	IPVM_TEST(info, "ipvm_init");
	ipvm_local_tid = pvm_mytid(); /* get my PVM task id.  */
	IPVM_TEST(ipvm_local_tid, "ipvm_init(pvm_mytid)");
	ipvm_creator_tid = pvm_parent();
	if (ipvm_creator_tid < 0) {
		ipvm_creator_tid = ipvm_local_tid;
	}

	/* get my Internet hostname and pid. */
	_nx_md_gethostname(ipvm_local_host, MAXHOSTNAMELEN);
	ipvm_local_pid = _nx_md_getpid();
	ipvm_local_host_length = strlen(ipvm_local_host);

#ifndef IPVM_PROTO_SINGLE_THREADED
	/* Spawn the handler thread. */
	nexus_thread_create(&thread, (nexus_thread_attr_t *) NULL,
		ipvm_handler_thread, (void *) NULL);
#endif /* IPVM_PROTO_SINGLE_THREADED */
}

static void nx_pvm_tasks(int *nt, int **ts)
{
	struct taskinfo* taskp;
	int *tids;
	int ntasks;
	int j;

	/* Space for the taskp array is allocated by pvm_tasks();
	   the man page doesn't say so but the PVM source code does.
	   The source also seems to reclaim the space the next
	   time that pvm_tasks() is called (scary).
	*/
	pvm_tasks(0, &ntasks, &taskp);
	NexusMalloc(nx_pvm_tasks(), tids, int *, ntasks);
	for (j = 0; j < ntasks; j++) {
		tids[j] = taskp[j].ti_tid;
	}
	*nt = ntasks;
	*ts = tids;
}

static void nx_pvm_broadcast_command(char *caller, int command)
{
	char flag = (char)command;
	int bufid, info;
	int ntasks;
	int *tids;

	nexus_debug_printf(1, ("[nx_pvm_broadcast_command]\n"));
	ipvm_enter();
	nx_pvm_tasks(&ntasks, &tids);
	bufid = pvm_mkbuf(PvmDataRaw);
	IPVM_TEST(bufid, caller);
	info = pvm_setsbuf(bufid);
	IPVM_TEST(info, caller);
	info = pvm_pkbyte(&flag, 1, 1);
	IPVM_TEST(info, caller);
	info = pvm_mcast(tids, ntasks, COMMAND_TAG);
	IPVM_TEST(info, caller);
	info = pvm_setsbuf(0);
	IPVM_TEST(info, caller);
	info = pvm_freebuf(bufid);
	IPVM_TEST(info, caller);
	ipvm_exit();
	NexusFree(tids);
}

/*
 * ipvm_shutdown()
 *
 * This routine is called during _normal_ shutdown of a process.
 *
 * todo: finish this.
 */

static void ipvm_shutdown(nexus_bool_t shutdown_others)
{
#ifndef IPVM_PROTO_SINGLE_THREADED
	nexus_bool_t i_am_handler_thread;
#endif

	nexus_debug_printf(1, ("[ipvm_shutdown]\n"));

#ifndef IPVM_PROTO_SINGLE_THREADED
    _nx_i_am_handler_thread(&i_am_handler_thread);
    if (!i_am_handler_thread) {
		nexus_debug_printf(1, ("ipvm_shutdown: sending self-shutdown.\n"));
		nx_pvm_send_command(ipvm_local_tid, "ipvm_shutdown",
			CLOSE_SELF_SHUTDOWN_FLAG);
		/* wait for handler thread to close. */
		nexus_debug_printf(1,
			("ipvm_shutdown: waiting for handler-thread to shutdown.\n"));
		nexus_mutex_lock(&handler_thread_done_mutex);
		while (!handler_thread_done) {
			nexus_cond_wait(&handler_thread_done_cond,
				&handler_thread_done_mutex);
		}
		nexus_mutex_unlock(&handler_thread_done_mutex);
	} else {
		ipvm_done = NEXUS_TRUE;
	}
	nexus_mutex_destroy(&handler_thread_done_mutex);
	nexus_cond_destroy(&handler_thread_done_cond);
	nexus_debug_printf(1,
		("ipvm_shutdown: finished waiting on handler-thread.\n"));
#endif /* IPVM_PROTO_SINGLE_THREADED */

	if (shutdown_others) {
		nx_pvm_broadcast_command("ipvm_shutdown", CLOSE_SHUTDOWN_FLAG);
	}
	ipvm_enter();
	pvm_exit();
	ipvm_exit();
}

/*
 * ipvm_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void ipvm_abort(void)
{
	nexus_debug_printf(1, ("[ipvm_abort]\n"));
	if (!ipvm_aborting) {
		nx_pvm_broadcast_command("ipvm_abort", CLOSE_ABNORMAL_FLAG);
		nexus_debug_printf(1, ("ipvm_abort: sending self-abort.\n"));
		nx_pvm_send_command(ipvm_local_tid, "ipvm_abort",
			CLOSE_SELF_ABNORMAL_FLAG);
	}
}

/* nx_pvm_get_tid(): extract PVM task id from global pointer */

static int nx_pvm_get_tid(nexus_global_pointer_t* gp)
{
	ipvm_proto_t* p;

    NexusAssert2((gp->proto->type == NEXUS_PROTO_TYPE_PVM),
("nx_pvm_get_tid(): Internal error: proto_type is not NEXUS_PROTO_TYPE_PVM\n"));

	p = (ipvm_proto_t *)gp->proto;
	return p->tid;
}

/*
 * ipvm_get_my_mi_proto()
 *
 * Return the machine independent PVM protocol information for
 * this protocol (i.e. the PVM task id == tid).
 */
static nexus_mi_proto_t *ipvm_get_my_mi_proto(void)
{
	nexus_mi_proto_t* mi_proto;
	int size;

	size = sizeof(nexus_mi_proto_t) + sizeof(int);
	NexusMalloc(ipvm_get_my_mi_proto(), mi_proto, nexus_mi_proto_t*, size);
	mi_proto->proto_type = NEXUS_PROTO_TYPE_PVM;
	mi_proto->n_ints = 1;
	mi_proto->n_strings = 0;
	mi_proto->ints = (int *)(mi_proto + 1);
	mi_proto->ints[0] = ipvm_local_tid;
	mi_proto->string_lengths = (int *)0;
	mi_proto->strings = (char **)0;
	return mi_proto;
}

/* todo: reference count protos? */

static ipvm_proto_t* construct_proto(int task_id)
{
	ipvm_proto_t* proto;

	NexusMalloc(construct_proto(), proto, ipvm_proto_t *, sizeof(ipvm_proto_t));
	proto->type = NEXUS_PROTO_TYPE_PVM;
	proto->funcs = &ipvm_proto_funcs;
	proto->tid = task_id;
	return proto;
}

/*
 * ipvm_construct_from_mi_proto()
 *
 * The passed machine independent protocol, 'mi_proto', should
 * be a PVM protocol that I can use to connect to that node:
 *  - If it is not a PVM protocol, then fatal out.
 *  - If it is a PVM protocol:
 *  - If I cannot use this protocol to attach to the node, then
 *      return NEXUS_FALSE.  (This option is useful if two nodes
 *      both speak a particular protocol, but they cannot
 *      talk to each other via that protocol.)
 *  - If this tcp protocol points to myself, then set
 *      *proto=_nx_local_proto, and return NEXUS_TRUE.
 *  - Otherwise, construct a tcp protocol object for this mi_proto
 *      and put it in *proto.  Then return NEXUS_TRUE.
 */
static nexus_bool_t
ipvm_construct_from_mi_proto(nexus_proto_t **proto, nexus_mi_proto_t *mi_proto)
{
	int tid;

	NexusAssert2((mi_proto->proto_type == NEXUS_PROTO_TYPE_PVM), (
"ipvm_construct_from_mi_proto(): Internal error: Was given a non-pvm mi_proto\n"
	));
	/*
	 * Test to see if this proto points to myself.
	 * If it does, then return the _nx_local_proto.
	 */
	tid = mi_proto->ints[0];
	if (tid == ipvm_local_tid) {
		*proto = _nx_local_proto;
	} else {
		*proto = (nexus_proto_t *) construct_proto(tid);
	}
	return (NEXUS_TRUE);
}

/*
 * ipvm_construct_creator_proto()
 *
 * Use the stored command line arguments to construct a proto
 * to my creator.
 */
static void ipvm_construct_creator_proto(nexus_proto_t **proto)
{
	*proto = (nexus_proto_t *)construct_proto(ipvm_creator_tid);
}

/*
 * ipvm_compare_protos()
 */
static int ipvm_compare_protos(nexus_proto_t *proto1, nexus_proto_t *proto2)
{
	ipvm_proto_t *p1 = (ipvm_proto_t*)proto1;
	ipvm_proto_t *p2 = (ipvm_proto_t*)proto2;

	return (p1->tid == p2->tid);
}

/*
 * ipvm_destroy_proto()
 *
 * todo: use reference counting for protos?
 */
static void ipvm_destroy_proto(nexus_proto_t *nexus_proto)
{
	nexus_debug_printf(4, ("[ipvm_destroy_proto]\n"));
	NexusFree(nexus_proto);
}

/*
 * ipvm_init_remote_service_request()
 *
 * Initiate a remote service request to the node and context specified
 * in the gp, to the handler specified by handler_id and handler_name.
 *
 * Return: Fill in 'buffer' with a nexus_buffer_t.
 */
static void ipvm_init_remote_service_request(nexus_buffer_t *buffer,
	nexus_global_pointer_t *gp, char *handler_name,
	int handler_id)
{
	ipvm_buffer_t* ipvm_buffer;
	int handler_name_length;
#ifdef BUILD_PROFILE
	int node_id, context_id;
#endif

	nexus_debug_printf(2, ("[ipvm_init_remote_service_request]\n"));
	ipvm_buffer = make_send_buffer(NEXUS_FALSE, gp);
	*buffer = (nexus_buffer_t) ipvm_buffer;
	nexus_debug_printf(2,
		("ipvm_init_remote_service_request: handler_name = %s, context = %lu, address = %lu\n", handler_name, gp->context, gp->address));
	nexus_put_u_long(buffer, &(gp->context), 1);
	nexus_put_u_long(buffer, &(gp->address), 1);
#ifdef BUILD_PROFILE
	ipvm_buffer->handler_name = handler_name;
	ipvm_buffer->handler_id = handler_id;
	_nx_node_id(&node_id);
	_nx_context_id(&context_id);
	nexus_put_int(buffer, &node_id, 1);
	nexus_put_int(buffer, &context_id, 1);
#endif
	ipvm_put_int(buffer, &handler_id, 1);
	handler_name_length = strlen(handler_name);
	ipvm_put_int(buffer, &handler_name_length, 1);
	ipvm_put_char(buffer, handler_name, handler_name_length);
}

/*
 * ipvm_send_remote_service_request()
 *
 * Generate a remote service request message to the node and context
 * saved in the 'nexus_buffer'.
 */
static int ipvm_send_remote_service_request(nexus_buffer_t *nexus_buffer)
{
	ipvm_buffer_t* buf;
#ifdef BUILD_PROFILE
	int info, msg_size, msg_tag, tid;
#endif

	nexus_debug_printf(2, ("[ipvm_send_remote_service_request]\n"));
	NexusBufferMagicCheck(ipvm_send_remote_service_request(), nexus_buffer);
	buf = (ipvm_buffer_t *) *nexus_buffer;

	NexusAssert2((buf->buffer_type == SEND_BUFFER), (
"ipvm_send_remote_service_request(): Internal error: Expected a send buffer\n")
	);

#ifdef BUILD_PROFILE
	ipvm_enter();
	/* get the size of the buffer in bytes */
	info = pvm_bufinfo(buf->bufid, &msg_size, &msg_tag, &tid);
	ipvm_exit();
	_nx_pablo_log_remote_service_request_send(buf->gp->node_id,
		buf->gp->context_id, buf->handler_name, buf->handler_id, msg_size);
#endif
	ipvm_enter();
	pvm_setsbuf(buf->bufid);
	pvm_send(nx_pvm_get_tid(buf->gp), HANDLER_TAG);
	ipvm_exit();

	return(0);
}

/*
 * ipvm_send_urgent_remote_service_request()
 */
static int ipvm_send_urgent_remote_service_request(nexus_buffer_t *buffer)
{
	/* todo: how are urgent requests different from non-urgent requests? */
    return(ipvm_send_remote_service_request(buffer));
}

/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

/*
 * ipvm_set_buffer_size()
 */
static void ipvm_set_buffer_size(nexus_buffer_t *buffer,
				int size, int n_elements)
{
	/* not needed in PVM protocol module */
}

/*
 * ipvm_check_buffer_size()
 */
static int ipvm_check_buffer_size(nexus_buffer_t *buffer,
				 int slack, int increment)
{
	/* not needed in PVM protocol module */
	return NEXUS_TRUE; /* indicates no resizing was needed. */
}

/*
 * ipvm_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since ipvm_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void ipvm_free_buffer(nexus_buffer_t *buffer)
{
	ipvm_buffer_t *ipvm_buffer;

	NexusAssert2((buffer),
		 ("ipvm_free_buffer(): Passed a NULL nexus_buffer_t *\n") );

	/* If the buffer was stashed, *buffer will have been set to NULL */
	if (!(*buffer)) {
		return;
	}
	NexusBufferMagicCheck(ipvm_free_buffer, buffer);
	ipvm_buffer = (ipvm_buffer_t *) *buffer;

	NexusAssert2((ipvm_buffer->buffer_type == RECV_BUFFER),
		 ("ipvm_free_buffer(): Expected a receive buffer\n"));
	NexusAssert2((!ipvm_buffer->stashed),
		 ("ipvm_free_buffer(): Expected a non-stashed buffer\n"));

	FreePvmBuffer(ipvm_buffer);
	*buffer = (nexus_buffer_t) NULL;
}

/*
 * ipvm_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void ipvm_stash_buffer(nexus_buffer_t *buffer,
			     nexus_stashed_buffer_t *stashed_buffer)
{
	ipvm_buffer_t *ipvm_buffer;

	NexusBufferMagicCheck(ipvm_stash_buffer, buffer);
	ipvm_buffer = (ipvm_buffer_t *) *buffer;

	NexusAssert2((ipvm_buffer->buffer_type == RECV_BUFFER),
	 ("ipvm_stash_buffer(): Expected a receive buffer\n"));
	NexusAssert2((!ipvm_buffer->stashed),
	 ("ipvm_stash_buffer(): Expected an un-stashed buffer\n"));

	ipvm_buffer->stashed = NEXUS_TRUE;
	*stashed_buffer = (nexus_stashed_buffer_t) *buffer;
	*buffer = (nexus_buffer_t) NULL;
}

/*
 * ipvm_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by ipvm_stash_buffer().
 */
static void ipvm_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
	ipvm_buffer_t *ipvm_buffer;

	NexusAssert2((stashed_buffer),
	 ("ipvm_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n"));

	NexusBufferMagicCheck(ipvm_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
	ipvm_buffer = (ipvm_buffer_t *) *stashed_buffer;

	NexusAssert2((ipvm_buffer->buffer_type == RECV_BUFFER),
		 ("ipvm_free_stashed_buffer(): Expected a receive buffer\n"));
	NexusAssert2((ipvm_buffer->stashed),
		 ("ipvm_free_stashed_buffer(): Expected a stashed buffer\n"));

	FreePvmBuffer(ipvm_buffer);
	*stashed_buffer = (nexus_stashed_buffer_t) NULL;
}

/* -- SIZEOF, PUT, GET, etc -- */

/*
 * ipvm_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int ipvm_sizeof_float(nexus_buffer_t *buffer, int count)
{
    return(sizeof(float) * count);
}

static int ipvm_sizeof_double(nexus_buffer_t *buffer, int count)
{
    return(sizeof(double) * count);
}

static int ipvm_sizeof_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(short) * count);
}

static int ipvm_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned short) * count);
}

static int ipvm_sizeof_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(int) * count);
}

static int ipvm_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned int) * count);
}

static int ipvm_sizeof_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(long) * count);
}

static int ipvm_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned long) * count);
}

static int ipvm_sizeof_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(char) * count);
}

static int ipvm_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned char) * count);
}

/*
 * ipvm_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */

#ifdef BUILD_DEBUG
#define OVERFLOW_CHECK(TYPE) \
do { \
	NexusBufferMagicCheck(ipvm_put_ ## TYPE, buf); \
	nexus_debug_printf(4, ("ipvm_put_" #TYPE "()\n")); \
} while (0)
#else /* BUILD_DEBUG */
#define OVERFLOW_CHECK(TYPE)
#endif /* BUILD_DEBUG */

#define PUT(TYPE) \
do { \
	int info; \
	ipvm_buffer_t *ipvm_buf = (ipvm_buffer_t *) *buf; \
	OVERFLOW_CHECK(TYPE); \
	ipvm_enter(); \
	pvm_setsbuf(ipvm_buf->bufid); \
	info = pvm_pk ## TYPE (data, count, 1); \
	IPVM_TEST(info, "ipvm_put_*"); \
	pvm_setsbuf(0); \
	ipvm_exit(); \
} while (0)

static void ipvm_put_float(nexus_buffer_t *buf, float *data, int count)
{
    PUT(float);
}

static void ipvm_put_double(nexus_buffer_t *buf, double *data, int count)
{
    PUT(double);
}

static void ipvm_put_short(nexus_buffer_t *buf, short *data, int count)
{
    PUT(short);
}

static void ipvm_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    PUT(ushort);
}

static void ipvm_put_int(nexus_buffer_t *buf, int *data, int count)
{
	PUT(int);
}

static void ipvm_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    PUT(uint);
}

static void ipvm_put_long(nexus_buffer_t *buf, long *data, int count)
{
    PUT(long);
}

static void ipvm_put_u_long(nexus_buffer_t *buf, unsigned long *data, int count)
{
    PUT(ulong);
}

static void ipvm_put_char(nexus_buffer_t *buf, char *data, int count)
{
    PUT(byte);
}

static void ipvm_put_u_char(nexus_buffer_t *buf, unsigned char *d, int count)
{
	char *data = (char *)d;

    PUT(byte);
}

/*
 * ipvm_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_ASSERTIONS(TYPE) \
do { \
	NexusBufferMagicCheck(ipvm_get_ ## TYPE, buf); \
	NexusAssert2(!ipvm_buf->stashed, \
		 ("ipvm_get_*(): Expected a non-stashed buffer.\n")); \
	if (info < 0) { \
		if (info == PvmNoData) { \
			ipvm_fatal("nexus_get_" #TYPE ": Buffer overrun.\n"); \
		} else if (info == PvmBadMsg) { \
			ipvm_fatal("nexus_get_" #TYPE ": PvmBadMsg.\n"); \
		} else if (info == PvmNoBuf) { \
			ipvm_fatal("nexus_get_" #TYPE ": No active receive buffer.\n"); \
		} else { \
			ipvm_fatal("nexus_get_" #TYPE ": error in pvm_unpk%s.\n", #TYPE); \
		} \
	} else nexus_debug_printf(4, ("ipvm_get_" #TYPE "()\n")); \
} while (0)

#else /* BUILD_DEBUG */
#define DO_GET_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#define GET(TYPE) \
do { \
	int info; \
	ipvm_buffer_t *ipvm_buf = (ipvm_buffer_t *) *buf; \
	ipvm_enter(); \
	pvm_setrbuf(ipvm_buf->bufid);  \
	info = pvm_upk ## TYPE (dest, count, 1); \
	IPVM_TEST(info, "ipvm_get_*"); \
	pvm_setrbuf(0); \
	ipvm_exit(); \
	DO_GET_ASSERTIONS(TYPE); \
} while (0)

static void ipvm_get_float(nexus_buffer_t *buf, float *dest, int count)
{
    GET(float);
}

static void ipvm_get_double(nexus_buffer_t *buf, double *dest, int count)
{
    GET(double);
}

static void ipvm_get_short(nexus_buffer_t *buf, short *dest, int count)
{
    GET(short);
}

static void ipvm_get_u_short(nexus_buffer_t *buf, unsigned short *dest,
			    int count)
{
    GET(ushort);
}

static void ipvm_get_int(nexus_buffer_t *buf, int *dest, int count)
{
    GET(int);
}

static void ipvm_get_u_int(nexus_buffer_t *buf, unsigned int *dest, int count)
{
    GET(uint);
}

static void ipvm_get_long(nexus_buffer_t *buf, long *dest, int count)
{
    GET(long);
}

static void ipvm_get_u_long(nexus_buffer_t *buf, unsigned long *dest, int count)
{
    GET(ulong);
}

static void ipvm_get_char(nexus_buffer_t *buf, char *dest, int count)
{
    GET(byte);
}

static void ipvm_get_u_char(nexus_buffer_t *buf, unsigned char *d, int count)
{
	char *dest = (char *)d;

    GET(byte);
}

/*
 * ipvm_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_STASHED_ASSERTIONS(TYPE) \
do { \
	NexusBufferMagicCheck(ipvm_get_stashed_ ## TYPE, (nexus_buffer_t*)buf); \
	NexusAssert2(ipvm_buf->stashed, \
		 ("ipvm_get_*(): Expected a stashed buffer.\n")); \
} while (0)
#else /* BUILD_DEBUG */
#define DO_GET_STASHED_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#define GET_STASHED(TYPE) \
do { \
	int info; \
	ipvm_buffer_t *ipvm_buf = (ipvm_buffer_t *) *buf; \
	DO_GET_STASHED_ASSERTIONS(TYPE); \
	ipvm_enter(); \
	pvm_setrbuf(ipvm_buf->bufid); \
	info = pvm_upk ## TYPE (dest, count, 1); \
	pvm_setrbuf(0); \
	ipvm_exit(); \
} while (0)

static void ipvm_get_stashed_float(nexus_stashed_buffer_t *buf,
				  float *dest, int count)
{
    GET_STASHED(float);
}

static void ipvm_get_stashed_double(nexus_stashed_buffer_t *buf,
				   double *dest, int count)
{
    GET_STASHED(double);
}

static void ipvm_get_stashed_short(nexus_stashed_buffer_t *buf,
				  short *dest, int count)
{
    GET_STASHED(short);
}

static void ipvm_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				    unsigned short *dest, int count)
{
    GET_STASHED(ushort);
}

static void ipvm_get_stashed_int(nexus_stashed_buffer_t *buf,
				int *dest, int count)
{
    GET_STASHED(int);
}

static void ipvm_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				  unsigned int *dest, int count)
{
    GET_STASHED(uint);
}

static void ipvm_get_stashed_long(nexus_stashed_buffer_t *buf,
				 long *dest, int count)
{
    GET_STASHED(long);
}

static void ipvm_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				   unsigned long *dest, int count)
{
    GET_STASHED(ulong);
}

static void ipvm_get_stashed_char(nexus_stashed_buffer_t *buf,
				 char *dest, int count)
{
    GET_STASHED(byte);
}

static void ipvm_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				   unsigned char *d, int count)
{
	char *dest = (char*)d;
    GET_STASHED(byte);
}
