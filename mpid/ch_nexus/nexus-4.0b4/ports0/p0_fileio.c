/*
 * p0_fileio.c
 *
 * Reentrant file I/O stuff
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_fileio.c,v 1.2 1995/03/30 18:22:52 tuecke Exp $";

#include "p0_internal.h"

extern int errno;

#ifdef PORTS0_FILEIO_NOT_REENTRANT

/*
 * ports0_reentrant_open()
 *
 * Reentrant version of open().
 */
int ports0_reentrant_open(char *path,
			  int flags,
			  int mode)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = open(path, flags, mode);
    save_errno = errno;
    /* Should set the fd to non-blocking here */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_open() */


/*
 * ports0_reentrant_close()
 *
 * Reentrant version of close().
 */
int ports0_reentrant_close(int fd)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = close(fd);
    save_errno = errno;
    /* Should convert EWOULDBLOCK to EINTR */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_close() */


/*
 * ports0_reentrant_read()
 *
 * Reentrant version of read().
 */
int ports0_reentrant_read(int fd,
			  char *buf,
			  int nbytes)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = read(fd, buf, nbytes);
    save_errno = errno;
    /* Should convert EWOULDBLOCK to EINTR */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_read() */


/*
 * ports0_reentrant_write()
 *
 * Reentrant version of write().
 */
int ports0_reentrant_write(int fd,
			   char *buf,
			   int nbytes)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = write(fd, buf, nbytes);
    save_errno = errno;
    /* Should convert EWOULDBLOCK to EINTR */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_write() */


/*
 * ports0_reentrant_fstat()
 *
 * Reentrant version of fstat().
 */
int ports0_reentrant_fstat(int fd,
			   struct stat *buf)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = fstat(fd, buf);
    save_errno = errno;
    /* Should convert EWOULDBLOCK to EINTR */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_fstat() */


/*
 * ports0_reentrant_lseek()
 *
 * Reentrant version of lseek().
 */
int ports0_reentrant_lseek(int fd,
			   off_t offset,
			   int whence)
{
    int rc;
    int save_errno;
    ports0_reentrant_lock();
    rc = lseek(fd, offset, whence);
    save_errno = errno;
    /* Should convert EWOULDBLOCK to EINTR */
    ports0_reentrant_unlock();
    errno = save_errno;
    return(rc);
} /* ports0_reentrant_lseek() */

#endif /* PORTS0_FILEIO_NOT_REENTRANT */
