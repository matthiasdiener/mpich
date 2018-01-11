/*
 * p4_args.c   Code that looks at the arguments, recognizes any that are
 *             for p4, uses the arguments, and removes them from the
 *             command line args.
 */
#include "p4.h"
#include "p4_sys.h"

/* Macro used to see if an arg is not following the correct format. */
#define bad_arg(a)    ( ((a)==NULL) || ((*(a)) == '-') )

static char pgm[100];		/* Used to keep argv[0] for the usage cmd. */

static P4VOID usage();
static P4VOID print_version_info();
static P4VOID strip_out_args();


P4VOID process_args(argc,argv)
int *argc;
char **argv;
{
    int i,c,nextarg;
    FILE *fp;
    char *s, **a;
    struct p4_procgroup_entry *pe;

    /* Put the name of the called program (according to the args) into pgm */
    s = (char *)  rindex(*argv, '/');
    if (s)
	strcpy(pgm, s + 1);
    else
	strcpy(pgm, *argv);

    /* Set all command line flags (except procgroup) to their defaults */
    debug_level = 0;
    remote_debug_level = 0;
    bm_outfile[0] = '\0';
    procgroup_file[0] = '\0';
    strcpy(local_domain, "");
    hand_start_remotes = FALSE;
    execer_starting_remotes = FALSE;
    execer_id[0] = '\0';
    execer_masthost[0] = '\0';
    execer_jobname[0] = '\0';
    execer_mynodenum = 0;
    execer_mastport = 0;
    execer_pg = NULL;

    /* Move to last argument, so that we can go backwards. */
    a = &argv[*argc - 1];

    /*
     * Loop backwards through arguments, catching the ones that start with
     * '-'.  Backwards is more efficient when you are stripping things out.
     */
    for (c = (*argc); c > 1; c--, a--)
    {
        /* p4_dprintfl(00,"p4: process_args:  :%s:",*a); */
	if (**a != '-')
	    continue;

        if (strcmp(*a, "-execer_id") == 0)
        {
            execer_starting_remotes = TRUE;
            strcpy(execer_id,*(a+1));
            strcpy(execer_masthost,*(a+3));
            strcpy(execer_myhost,*(a+5));
            execer_mynodenum = atoi(*(a+7));
            execer_mynumprocs = atoi(*(a+9));
	    execer_numtotnodes = atoi(*(a+11));
            strcpy(execer_jobname,*(a+13));
	    if (execer_mynodenum == 0)
	    {
		execer_pg = p4_alloc_procgroup();
		pe = execer_pg->entries;
		strcpy(pe->host_name,execer_myhost);
		pe->numslaves_in_group = execer_mynumprocs - 1;
		strcpy(pe->slave_full_pathname,argv[0]);
		pe->username[0] = '\0'; /* unused */
		execer_pg->num_entries++;
		nextarg = 15;
		for (i=0; i < (execer_numtotnodes-1); i++)
		{
		    pe++;
		    strcpy(pe->host_name,*(a+nextarg));
		    nextarg++;
		    nextarg++;  /* skip node num */
		    pe->numslaves_in_group = atoi(*(a+nextarg));
		    nextarg++;
		    strcpy(pe->slave_full_pathname,*(a+nextarg)); /* unused */
		    nextarg++;
		    pe->username[0] = '\0'; /* unused */
		    execer_pg->num_entries++;
		}
	    }
	    else
	    {
		execer_mastport = get_execer_port(execer_masthost);
	    }
            continue;
        }

	if (!strcmp(*a, "-pg")      ||
	    !strcmp(*a, "-pg")      ||
	    !strcmp(*a, "-dbg")     ||
	    !strcmp(*a, "-ssport")  ||
	    !strcmp(*a, "-rdbg")    ||
	    !strcmp(*a, "-gm")      ||
	    !strcmp(*a, "-dmn")     ||
	    !strcmp(*a, "-out")     ||
	    !strcmp(*a, "-rout")    ||
	    !strcmp(*a, "-log")     ||
	    !strcmp(*a, "-norem")   ||
	    !strcmp(*a, "-version") ||
	    !strcmp(*a, "-help"))
	{
	    printf("Warning: %s should be -p4%s\n",*a,(*a)+1);
	}


	if (!strcmp(*a, "-p4pg")  ||  !strcmp(*a, "-pg"))
	{
	    if (bad_arg(a[1]))
		usage();
	    strcpy(procgroup_file, a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4dbg") ||  !strcmp(*a, "-dbg"))
	{
	    if (bad_arg(a[1]))
		usage();
	    debug_level = atoi(a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4ssport") ||  !strcmp(*a, "-ssport"))
	{
	    if (bad_arg(a[1]))
		usage();
	    sserver_port = atoi(a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4rdbg") ||  !strcmp(*a, "-rdbg"))
	{
	    if (bad_arg(a[1]))
		usage();
	    remote_debug_level = atoi(a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4gm") ||  !strcmp(*a, "-gm"))
	{
	    if (bad_arg(a[1]))
		usage();
	    globmemsize = atoi(a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4dmn") ||  !strcmp(*a, "-dmn"))
	{
	    if (bad_arg(a[1]))
		usage();
	    strcpy(local_domain, a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4out") ||  !strcmp(*a, "-out"))
	{
	    if (bad_arg(a[1]))
		usage();
	    strcpy(bm_outfile, a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4rout") ||  !strcmp(*a, "-rout"))
	{
	    if (bad_arg(a[1]))
		usage();
	    strcpy(rm_outfile_head, a[1]);
	    strip_out_args(a, argc, &c, 2);
	    continue;
	}
	if (!strcmp(*a, "-p4log") ||  !strcmp(*a, "-log"))
	{
	    strip_out_args(a, argc, &c, 1);
	    logging_flag = TRUE;
	    continue;
	}
	if (!strcmp(*a, "-p4norem") ||  !strcmp(*a, "-norem"))
	{
	    strip_out_args(a, argc, &c, 1);
	    hand_start_remotes = TRUE;
	    continue;
	}
	if (!strcmp(*a, "-p4version") ||  !strcmp(*a, "-version"))
	{
	    strip_out_args(a, argc, &c, 1);
	    print_version_info();
	    continue;
	}
	if (!strcmp(*a, "-p4help") ||  !strcmp(*a, "-help"))
	    usage();
    }
    if (procgroup_file[0] == '\0')
    {
	strcpy(procgroup_file,argv[0]);
	strcat(procgroup_file,".pg");
	if ((fp = fopen(procgroup_file,"r")) == NULL)  /* pgm.pg not there */
	    strcpy(procgroup_file, "procgroup");
	else
	    fclose(fp);
    }
    p4_dprintfl(10,"using procgroup file %s\n",procgroup_file);
}

static P4VOID strip_out_args(argv, argc, c, num)
char **argv;
int *argc, *c, num;
{
    char **a;
    int i;

    /* Strip out the argument. */
    for (a = argv, i = (*c); i <= *argc; i++, a++)
	*a = (*(a + num));
    (*argc) -= num;
}

static P4VOID usage()
{
    print_version_info();
    printf("p4 usage: %s [p4 options]\n", pgm);
    printf("Valid p4 options:\n");
    printf("\t-p4help            get this message\n");
    printf("\t-p4pg      <file>  set procgroup file\n");
    printf("\t-p4dbg    <level>  set debug level\n");
    printf("\t-p4rdbg   <level>  set remote debug level\n");
    printf("\t-p4gm      <size>  set globmemsize\n");
    printf("\t-p4dmn   <domain>  set domainname\n");
    printf("\t-p4out     <file>  set output file for master\n");
    printf("\t-p4rout    <file>  set output file prefix for remote masters\n");
    printf("\t-p4ssport <port#>  set private port number for secure server\n");
    printf("\t-p4norem           don't start remote processes\n");
#ifdef ALOG_TRACE
    printf("\t-p4log             enable internal p4 logging by alog\n");
#endif
    printf("\t-p4version         print current p4 version number\n");
    printf("\n");
    exit(-1);

}

static P4VOID print_version_info()
{
        printf("\n");
        printf("p4  version number: %s\n",P4_PATCHLEVEL);
        printf("p4 date configured: %s\n",P4_CONFIGURED_TIME);
        printf("p4    machine type: %s\n",P4_MACHINE_TYPE);
#ifdef P4_DPRINTFL
        printf("   P4_DPRINTFL is:  on\n");
#else
        printf("   P4_DPRINTFL is:  off\n");
#endif
#ifdef ALOG_TRACE
        printf("    ALOG_TRACE is:  on\n");
#else
        printf("    ALOG_TRACE is:  off\n");
#endif
#if defined(SYSV_IPC)
        printf("      SYSV IPC is:  on\n");
#else
        printf("      SYSV IPC is:  off\n");
#endif
#if defined(VENDOR_IPC)
        printf("      VENDOR IPC is:  on\n");
#else
        printf("      VENDOR IPC is:  off\n");
#endif
        printf("\n");
}
