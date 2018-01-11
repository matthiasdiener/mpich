#include <stdio.h>
#include <varargs.h>
#include "p4.h"
#include "p4_sys.h"

#if defined(p4_dprintfl)
#undef p4_dprintfl
#endif

int p4_get_dbg_level()
{
    return(debug_level);
}

P4VOID p4_set_dbg_level(level)
int level;
{
    debug_level = level;
}

#if defined(DELTA)  ||  defined(NCUBE) || defined(LINUX)

P4VOID p4_dprintf(fmt, a, b, c, d, e, f, g, h, i)
{
    printf("%s: ",whoami_p4);
    printf(fmt,a,b,c,d,e,f,g,h,i);
    fflush(stdout);
}

#else

P4VOID p4_dprintf(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;

    printf("%s: ", whoami_p4);
    va_start(ap);
#ifdef VPRINTF
    vprintf(fmt, ap);
#else
    _doprnt(fmt, ap, stdout);
#endif
    va_end(ap);
    fflush(stdout);
}

#endif

#if defined(DELTA)  ||  defined(NCUBE)  ||  defined(LINUX)

P4VOID p4_dprintfl(level, fmt, a, b, c, d, e, f, g, h, i)
{
    if (level > debug_level)
        return;
    printf("%s: ",whoami_p4);
    printf(fmt,a,b,c,d,e,f,g,h,i);
    fflush(stdout);
}

#else

P4VOID p4_dprintfl(level, fmt, va_alist)
int level;
char *fmt;
va_dcl
{
    va_list ap;

    if (level > debug_level)
	return;
    printf("%d: %s: ", level, whoami_p4);
    va_start(ap);
#ifdef VPRINTF
    vprintf(fmt, ap);
#else
    _doprnt(fmt, ap, stdout);
#endif
    va_end(ap);
    fflush(stdout);
}

#endif 

P4VOID dump_global(level)
int level;
{
    int i;
    struct p4_global_data *g = p4_global;
    struct proc_info *p;

    if (level > debug_level)
	return;

    p4_dprintf("Dumping global data for process %d at %x\n", getpid(), g);

    for (i = 0, p = g->proctable; i < g->num_in_proctable; i++, p++)
    {
	p4_dprintf(" proctable entry %d: unix_id = %d host = %s\n",
		   i, p->unix_id, p->host_name);
	p4_dprintf("   port=%d group_id=%d switch_port=%d\n",
		   p->port, p->group_id, p->switch_port);
    }

    p4_dprintf("    listener_pid     = %d\n", g->listener_pid);
    p4_dprintf("    listener_port    = %d\n", g->listener_port);
    p4_dprintf("    local_slave_count= %d\n", g->local_slave_count);
    p4_dprintf("    my_host_name     = %s\n", g->my_host_name);
    p4_dprintf("    num_in_proctable = %d\n", g->num_in_proctable);
}

P4VOID dump_local(level)
int level;
{
    struct local_data *l = p4_local;
    int i;

    if (level > debug_level)
	return;

    p4_dprintf("Dumping local data for process %d at %x\n", getpid(), l);

    for (i = 0; i < p4_global->num_in_proctable; i++)
	p4_dprintf("     %d: conntab[%d]  type:%s    port %d\n", getpid(), i,
		   print_conn_type(p4_local->conntab[i].type),
		   p4_local->conntab[i].port);

    p4_dprintf("    listener_fd = %d\n", l->listener_fd);
    p4_dprintf("    my_id       = %d\n", l->my_id);
    p4_dprintf("    am_bm       = %d\n", l->am_bm);
}

char *print_conn_type(conn_type)
int conn_type;
{
    static char val[20];

    switch (conn_type)
    {
      case CONN_ME:
	return ("CONN_ME");
      case CONN_REMOTE_SWITCH:
	return ("CONN_REMOTE_SWITCH");
      case CONN_REMOTE_NON_EST:
	return ("CONN_REMOTE_NON_EST");
      case CONN_REMOTE_EST:
	return ("CONN_REMOTE_EST");
      case CONN_SHMEM:
	return ("CONN_SHMEM");
      case CONN_CUBE:
	return ("CONN_CUBE");
      case CONN_REMOTE_DYING:
	return ("CONN_REMOTE_DYING");
      default:
	sprintf(val, "invalid: %d  ", conn_type);
	return (val);
    }
}


P4VOID dump_listener(level)
int level;
{
    struct listener_data *l = listener_info;

    if (level > debug_level)
	return;

    p4_dprintf("Dumping listener data for process %d at %x\n", getpid(), l);
    p4_dprintf("    listening_fd = %d\n", l->listening_fd);
}

P4VOID dump_procgroup(procgroup, level)
struct p4_procgroup *procgroup;
int level;
{
    struct p4_procgroup_entry *pe;
    int i;

    if (level > debug_level)
	return;

    p4_dprintf("Procgroup:\n");
    for (pe = procgroup->entries, i = 0;
	 i < procgroup->num_entries;
	 pe++, i++)
	p4_dprintf("    entry %d: %s %d %s %s \n",
		   i,
		   pe->host_name,
		   pe->numslaves_in_group,
		   pe->slave_full_pathname,
		   pe->username);
}

P4VOID dump_tmsg(tmsg)
struct p4_msg *tmsg;
{
    p4_dprintf("type=%d, to=%d, from=%d, len=%d, ack_req=%x, msg=%s\n",
	       tmsg->type, tmsg->to, tmsg->from, tmsg->len, tmsg->ack_req,
	       &(tmsg->msg));
}

P4VOID dump_conntab(level)
int level;
{
    int i;

    if (level > debug_level)
	return;

    for (i = 0; i < p4_global->num_in_proctable; i++)
    {
	p4_dprintf("   %d: conntab[%d] type=%s port=%d switch_port=%d\n",
		   getpid(), i,
		   print_conn_type(p4_local->conntab[i].type),
		   p4_local->conntab[i].port,
		   p4_local->conntab[i].switch_port);
    }
}