/*
 * p0_mac_func.c
 *
 * Many of the PORTS0 functions can be efficiently implemented
 * as macros.  However, in some situations you may not want to use
 * the macros (i.e. when debugging).  If the user
 * defines USE_MACROS before including ports0.h, then the
 * macro versions will be used.
 *
 * This file holds the function versions of these various macros.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_mac_func.c,v 1.6 1996/02/02 01:13:08 patton Exp $";

#include "p0_internal.h"


#undef ports0_reentrant_lock
int ports0_reentrant_lock(void)
{
    return(ports0_macro_reentrant_lock());
} /* ports0_reentrant_lock() */

#undef ports0_reentrant_unlock
int ports0_reentrant_unlock(void)
{
    return(ports0_macro_reentrant_unlock());
} /* ports0_reentrant_unlock() */

#undef ports0_malloc
void *ports0_malloc(size_t bytes)
{
    return(ports0_macro_malloc(bytes));
} /* ports0_malloc() */

#undef ports0_realloc
void *ports0_realloc(void *ptr,
		     size_t bytes)
{
    return(ports0_macro_realloc(ptr,bytes));
} /* ports0_realloc() */

#undef ports0_calloc
void *ports0_calloc(size_t nobj, 
		    size_t bytes)
{
    return(ports0_macro_calloc(nobj,bytes));
} /* ports0_calloc() */

#undef ports0_free
void ports0_free(void *ptr)
{
    ports0_macro_free(ptr);
} /* ports0_free() */

#undef ports0_open
int ports0_open(char *path,
		int flags,
		int mode)
{
    return(ports0_macro_open(path, flags, mode));
} /* ports0_open() */

#undef ports0_close
int ports0_close(int fd)
{
    return(ports0_macro_close(fd));
} /* ports0_close() */

#undef ports0_read
int ports0_read(int fd,
		char *buf,
		int nbytes)
{
    return(ports0_macro_read(fd, buf, nbytes));
} /* ports0_read() */

#undef ports0_write
int ports0_write(int fd,
		 char *buf,
		 int nbytes)
{
    return(ports0_macro_write(fd, buf, nbytes));
} /* ports0_write() */

#undef ports0_fstat
int ports0_fstat(int fd,
		 struct stat *buf)
{
    return(ports0_macro_fstat(fd, buf));
} /* ports0_fstat() */

#undef ports0_lseek
int ports0_lseek(int fd,
		 off_t offset,
		 int whence)
{
    return(ports0_macro_lseek(fd, offset, whence));
} /* ports0_lseek() */
