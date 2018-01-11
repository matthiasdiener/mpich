#ifndef _P4_H_
#define _P4_H_

#include <ctype.h>
#include <stdio.h>

#if defined(RS6000)  ||  defined(SYMMETRY_PTX)
#include <sys/select.h>
#endif

/* for xdr  -  includes netinet/in.h and sys/types.h */
#include <rpc/rpc.h>      
/* Some systems DO NOT include netinet! */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif

#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <pwd.h>
#include <fcntl.h>

#include "p4_config.h"
#include "p4_MD.h"
#include "p4_mon.h"
#include "p4_sr.h"

#include "p4_funcs.h"

#ifndef P4_DPRINTFL
#define p4_dprintfl
#endif

#include "alog.h"

#include "usc.h"
#define p4_ustimer() usc_clock()
#define p4_usrollover() usc_MD_rollover_val

#define HOSTNAME_LEN 64

struct p4_procgroup_entry {
    char host_name[HOSTNAME_LEN];
    int numslaves_in_group;
    char slave_full_pathname[100];
    char username[10];
};

#define P4_MAX_PROCGROUP_ENTRIES 256
struct p4_procgroup {
    struct p4_procgroup_entry entries[P4_MAX_PROCGROUP_ENTRIES];
    int num_entries;
};

#endif
