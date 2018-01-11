#include "mpid.h"

/* 
   This version implements a node name base on using the various routines 
   for getting the node name from the system.  MPPs will use their own
   routines.
 */

#if defined(HAVE_UNAME)
#include <sys/utsname.h>
#endif
#if defined(HAVE_SYSINFO)
#if defined(HAVE_SYSTEMINFO_H)
#include <sys/systeminfo.h>
#else
#ifdef HAVE_SYSINFO
#undef HAVE_SYSINFO
#endif
#endif
#endif

#include <string.h>

void MPID_Node_name( name, nlen )
char *name;
int  nlen;
{
#if defined(solaris) || defined(HAVE_UNAME)
    struct utsname utname;
    uname(&utname);
    /* We must NOT use strncpy because it will null pad to the full length
       (nlen).  If the user has not allocated MPI_MAX_PROCESSOR_NAME chars,
       then this will unnecessarily overwrite storage.
     */
    /* strncpy(name,utname.nodename,nlen); */
    {
	char *p_out = name;
	char *p_in  = utname.nodename;
	int  i;
	for (i=0; i<nlen-1 && *p_in; i++) 
	    *p_out++ = *p_in++;
	*p_out = 0;
    }
#elif defined(HAVE_GETHOSTNAME)
    gethostname(name, nlen);
#elif defined(HAVE_SYSINFO)
    sysinfo(SI_HOSTNAME, name, nlen);
#else 
    sprintf( name, "%d", MPID_MyWorldRank );
#endif
/* See if this name includes the domain */
    if (!strchr(name,'.')) {
	int  l;
	l = strlen(name);
	name[l++] = '.';
	name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
#if defined(solaris) || defined(HAVE_SYSINFO)
	sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#elif defined(HAVE_GETDOMAINNAME)
	getdomainname( name+l, nlen - l );
#endif
  }
}
