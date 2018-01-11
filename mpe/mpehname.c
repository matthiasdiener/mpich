#if defined(HAVE_UNAME)
#include <sys/utsname.h>
#endif
#if defined(HAVE_SYSINFO)
#include <sys/systeminfo.h>
#endif

void MPE_GetHostName( name, nlen )
int  nlen;
char *name;
{
#if defined(HAVE_UNAME)
  struct utsname utname;
  uname(&utname);
  strncpy( name, utname.nodename, nlen);
#elif defined(HAVE_GETHOSTNAME)
  gethostname( name, nlen );
#elif defined(HAVE_SYSINFO)
  sysinfo(SI_HOSTNAME, name, nlen);
#else 
  strncpy( name, "Unknown!", nlen );
#endif
/* See if this name includes the domain */
  if (!strchr(name,'.')) {
    int  l;
    l = strlen(name);
    name[l++] = '.';
    name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
#if defined(HAVE_SYSINFO)
    sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#elif defined(HAVE_GETDOMAINNAME)
    getdomainname( name+l, nlen - l );
#endif
  }
}
