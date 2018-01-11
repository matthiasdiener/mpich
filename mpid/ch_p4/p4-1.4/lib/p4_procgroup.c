#include "p4.h"
#include "p4_sys.h"

#ifdef TCMP
extern struct tc_globmem *tcglob;
#endif

struct p4_procgroup *p4_alloc_procgroup()
{
    struct p4_procgroup *pg;

    if (!(pg = (struct p4_procgroup *) p4_malloc(sizeof(struct p4_procgroup))))
	p4_error("p4_alloc_procgroup: p4_malloc failed",
		 sizeof(struct p4_procgroup));

    p4_dprintfl(90, "p4_alloc_procgroup: allocing %d bytes\n",
		sizeof(struct p4_procgroup));

    pg->num_entries = 0;
    return (pg);
}

struct p4_procgroup *read_procgroup()
{
    FILE *fp;
    char buf[1024], *s;
    struct p4_procgroup_entry *pe;
    int i, group_id, pt_index, n;
    struct p4_procgroup *pg;
    struct passwd *pwent;
    char *logname; 


    p4_dprintfl(90,"entering read_procgroup pgfname=%s\n",procgroup_file);

    pg = p4_alloc_procgroup();

#   if defined(CM5)  ||  defined(NCUBE)
    logname = '\0';
#   else
    logname = (char *) getlogin();
#   endif

    if ((fp = fopen(procgroup_file, "r")) == NULL)
	p4_error("open error on procgroup file",NULL);

    pe = pg->entries;

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
	for (s = buf; isspace(*s); s++)
	    ;

	if (*s == '#' || *s == '\0')	/* Ignore comments & blanks */
	    continue;

	n = sscanf(buf, "%s %d %s %s",
		   pe->host_name,
		   &pe->numslaves_in_group,
		   pe->slave_full_pathname,
		   pe->username);

	if (n == 3)
	{
	    if (logname != NULL && logname[0] != '\0')
		strcpy(pe->username, logname);
	    else
	    {
#               if defined(CM5)  ||  defined(NCUBE)
                strcpy(pe->username, "cube-user");
#               else
		if ((pwent = getpwuid(getuid())) == NULL)
		    p4_error("create_procgroup: getpwuid failed", 0);
		strcpy(pe->username, pwent->pw_name);
#               endif
	    }
	}
	pe++;
	pg->num_entries++;
	if (pg->num_entries > P4_MAX_PROCGROUP_ENTRIES)
	    p4_error("read procgroup: exceeded max # of procgroup entries",
		     P4_MAX_PROCGROUP_ENTRIES);
    }

    dump_procgroup(pg,50);
    return (pg);
}				/* read_procgroup */


int install_in_proctable(group_id,port,unix_id,host_name,
			 slv_idx,machine_type,switch_port)
int group_id;
int port;
int unix_id;
char host_name[64];
int slv_idx;
char machine_type[];
int switch_port;
{
    struct p4_global_data *g;
    struct proc_info *pi;

    g = p4_global;
    pi = &g->proctable[g->num_installed];
    pi->group_id = group_id;
    pi->port = port;
    pi->unix_id = unix_id;
    strcpy(pi->host_name, host_name);
    strcpy(pi->machine_type,machine_type);
    pi->slave_idx = slv_idx;
    pi->switch_port = switch_port;
    g->num_installed++;
    p4_dprintfl(50, "installed in proctable num=%d port=%d host=%s unix=%d slav=%d grp=%d swport=%d\n",
		g->num_installed, port, host_name, unix_id, slv_idx, pi->group_id,pi->switch_port);
    return (g->num_installed - 1);
}

