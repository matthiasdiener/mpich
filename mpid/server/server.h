/* server.h.  Generated automatically by configure.  */
/* 
 * rcsid = $Header: /home/MPI/servers/master/server/server.h.in,v 1.3 1996/12/04 16:51:04 gropp Exp $
 */
#ifndef _SECURE_SERVER
#define _SECURE_SERVER

/* 
 * Variables defined by configure
 */
#define HAVE_TERMIOS_H 1
#define HAVE_FCNTL_H 1
#define HAVE_INDEX 1
#define HAVE_RINDEX 1
/* #undef HAVE_KERBEROS */
#define HAVE_AFS 1
/* #undef HAVE_SSL */
/* #undef IWAY */
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_GETOPT 1
/* #undef HAVE_STRERROR */
/* #undef HAVE_UNION_WAIT */
/* #undef HAVE_WAIT3 */
/* #undef HAVE_BSD_SIGNAL */
#define HAVE_SETPGRP 1
/* #undef HAVE_SETRESUID */
/* #undef HAVE_SYS_SELECT */
#define HAVE_UNISTD_H 1

/* 
 * old defines for compatibility with the Nexus secure server.  These
 * can be removed when v3.0 is stopped being supported.
 */
#define SERVER_CD_NOTIFIER "\0"
#define SERVER_ENV_NOTIFIER "\1"

#endif /* _SECURE_SERVER */
