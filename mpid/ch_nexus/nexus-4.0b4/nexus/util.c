/*
 * util.c
 *
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/util.c,v 1.28 1997/02/13 23:22:25 tuecke Exp $";

#include "internal.h"

typedef struct _nx_barrier_t
{
    nexus_mutex_t mutex;
    nexus_cond_t  cond;
    nexus_bool_t  done;
    int           rc;
} nx_barrier_t;

static void blocking_write_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes);
static void blocking_read_callback(void *arg,
				   int fd,
				   char *buf,
				   size_t nbytes,
				   char **new_buf,
				   size_t *new_max_nbytes,
				   size_t *new_wait_for_nbytes);
static void blocking_error_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes,
				    int error);

/* This is for the access() call */
#include <sys/file.h>

/*
 * _nx_copy_string()
 *
 * Copy the string into malloced space and return it.
 *
 * Reminder: This function returns a pointer to malloced memory, so
 * don't forget to use NexusFree() on it...
 */
char *_nx_copy_string(char *s)
{
    char *rc;

    NexusMalloc(_nx_copy_string(), rc, char *, (strlen(s) + (size_t)(1)));
    strcpy(rc, s);
    return (rc);
} /* _nx_copy_string() */


/*
 * _nx_executable_name()
 */
char *_nx_executable_name(void)
{
    char *argv0;
    char *executable_name;
    char current_directory[MAX_PATH_LENGTH];
    int save_error;
    
    if (getwd(current_directory) == 0)
    {
	save_error = errno;
	nexus_fatal("_nx_tcp_start_nodes(): getwd failed: %s\n",
		   _nx_md_system_error_string(save_error));
    }
    _nx_strip_tmp_mnt_from_path(current_directory);

    NexusMalloc(_nx_executable_name(), executable_name, char *, MAX_PATH_LENGTH);

    argv0 = _nx_get_argv0();
    
    if (*argv0 == '/' || _nx_dont_expand_executables) 
    {
        _nx_strip_tmp_mnt_from_path(argv0);
	strcpy(executable_name, argv0);
    }
    else if (strchr(argv0, '/') != (char *) NULL)
    {
	strcpy(executable_name, current_directory);
	strcat(executable_name, "/");
	strcat(executable_name, argv0);
    }
    else
    {
	/* We have to search the PATH */

	char *path;

	path = getenv("PATH");
	if (path == NULL)
	{
	    /* Give up and just return argv0 */
	    strcpy(executable_name, argv0);
	}
	else
	{
	    int done, gotit;
	    char *pptr, *s, *pathelt;
	    
	    path = _nx_copy_string(path);

	    for (gotit = NEXUS_FALSE, pptr = path, done = NEXUS_FALSE; !done; )
	    {
		pathelt = pptr;
		if ((s = strchr(pptr, ':')) != (char *) NULL)
		{
		    *s = '\0';
		    pptr = s + 1;
		}
		else
		    done = 1;

		if (pathelt[0] == '.')
		{
		    strcpy(executable_name, current_directory);
		    strcat(executable_name, "/");
		}
		else
		    executable_name[0] = '\0';

		strcat(executable_name, pathelt);
		strcat(executable_name, "/");
		strcat(executable_name, argv0);
		if (access(executable_name, X_OK) == 0)
		{
		    gotit = 1;
		    done = 1;
		}
	    }
	    NexusFree(path);
	    if (!gotit)
	    {
		/* Give up and just return argv0 */
		strcpy(executable_name, argv0);
	    }
	}
    }
    
    return (executable_name);
} /* _nx_executable_name() */


/*
 * _nx_current_working_directory()
 */
char *_nx_current_working_directory(void)
{
    char my_directory_path[MAX_PATH_LENGTH];
    if (getwd(my_directory_path) == 0)
    {
	int save_error = errno;
	nexus_fatal("_nx_current_working_directory(): getwd failed: %s\n",
		    _nx_md_system_error_string(save_error));
    }
    _nx_strip_tmp_mnt_from_path(my_directory_path);
    return(_nx_copy_string(my_directory_path));
} /* _nx_current_working_directory() */


/*
 * _nx_strip_tmp_mnt_from_path()
 *
 * Modify the passed directory name, 'dir', to remove a leading "/tmp_mnt"
 * or "/private".  These are common prefixes that are used by
 * NFS automounters, which sometimes get in the way.
 */
void _nx_strip_tmp_mnt_from_path(char *dir)
{
#ifdef NEXUS_ARCH_AEROSPACE_CSRD
    /*
     * These rules are appropriate for the CSRD subnet.
     *
     * /tmp_mnt/home -> /home
     * /tmp_mnt/ -> /nfsmounts
     * /amd/.../export/home -> /home
     * /amd/.../export -> /nfsmounts
     *
     * The /.../ components are hostnames.
     */
    if (strncmp(dir, "/tmp_mnt/home", 13 ) == 0)
    {
        memmove(dir, dir+8, strlen(dir) - 7);
    } 
    else if (strncmp(dir, "/tmp_mnt", 8 ) == 0)
    {
        char tmp[1024];
	
	memmove(tmp, dir+8, strlen(dir) - 7);
	memmove(dir, "/nfsmounts", 10);
        memmove(dir+10, tmp, strlen(tmp));
    }
    else if (strncmp(dir, "/amd", 4 ) == 0)
    {
	char *s;
	int shift;
	
	if ((s = strstr(dir, "/export/home")) != NULL)
	{
	    shift = (s - dir) + 7;
	    memmove(dir, dir+shift, strlen(dir) + 1 - shift );
	}
	else if ((s = strstr(dir, "/export")) != NULL)
	{
	    char tmp[1024];
	    
	    shift = (s - dir) + 7;
	    memmove(tmp, dir+shift, strlen(dir) + 1 - shift );
	    memmove(dir, "/nfsmounts", 10);
	    memmove(dir+10, tmp, strlen(tmp));
	}
    }
    else
#endif /* NEXUS_ARCH_AEROSPACE_CSRD */
    {
	if (strncmp(dir, "/tmp_mnt", 8) == 0)
	{
	    memmove(dir, dir+8, strlen(dir) - 7);
	}
	else if (strncmp(dir, "/private", 8) == 0)
	{
	    memmove(dir, dir+8, strlen(dir) - 7);
	}
    }
} /* _nx_strip_tmp_mnt_from_path() */


/*
 * nexus_ids()
 *
 * Return node_id, context_id, and thread_id for the calling thread.
 */
void nexus_ids(int *node_id, int *context_id, int *thread_id)
{
    _nx_node_id(node_id);
    _nx_context_id(context_id);
    _nx_thread_id(thread_id);
} /* nexus_ids() */


/*
 * _nx_find_attribute()
 */
char *_nx_find_attribute(char *attr, char *search_string, char separator)
{
    char *pos;
    char *tmp;

    tmp = search_string;
    while ((pos = strstr(tmp, attr)))
    {
        if (   (*(pos-1)==separator)
	    && (   (*(pos+strlen(attr))=='=')
		|| (*(pos+strlen(attr))==separator) ) )
        {
	    char *value;
	    char *i;
	    int j;

            /* got right place */
	    NexusMalloc(_nx_find_attribute(),
		        value,
		        char *,
		        sizeof(char) * /* some arbitrary size */ 100);
	    for (j = 0, i = pos+strlen(attr); *i; i++)
	    {
		if (*i == separator)
		{
		    break;
		}
	        if (!isspace(*i) && *i != '=')
	        {
		    value[j++] = *i;
	        }
	    }
	    value[j] = '\0';
	    return (value);
        }
	else
	{
	    tmp = pos + strlen(attr);
	}
    }
    
    return NULL;
} /* _nx_find_attribute() */


/*
 * _nx_get_next_value()
 */
void _nx_get_next_value(char *string,
			char separator,
			char **next,
			char **value)
{
    if (!*string)
    {
	*value = NULL;
	*next = NULL;
    }
    *value = _nx_copy_string(string);
    if ((*next = strchr(*value, separator)) != (char *) NULL)
    {
	**next = '\0';
	*next = strchr(string, separator);
	(*next)++;
    }
    else if ((*next = strchr(*value, '\0')) != (char *) NULL)
    {
	*next = NULL;
    }
} /* _nx_get_next_value() */


/*
 * _nx_hex_encode_byte_array()
 *
 * Encode the first 'length' bytes of 'bytes' as hex digits, placing
 * the result in 'hex'.
 *
 * 'hex' must be an array of at least length: ((2 * length) + 1)
 */
void _nx_hex_encode_byte_array(nexus_byte_t *bytes,
			       int length,
			       char *hex)
{
    int i;
    char buf[4];
    hex[0] = '\0';
    nexus_stdio_lock();
    for (i = 0; i < length; i++)
    {
	if (bytes[i] <= 0xF)
	{
	    sprintf(buf, "0%1x", (int) bytes[i]);
	}
	else
	{
	    sprintf(buf, "%2x", (int) bytes[i]);
	}
	strcat(hex, buf);
    }
    nexus_stdio_unlock();
} /* _nx_hex_encode_byte_array() */


/*
 * _nx_hex_decode_byte_array()
 *
 * Decode 'length' bytes from the 'hex' array (encoded in hex values)
 * into the 'bytes' array.
 *
 * 'bytes' must be an array of at least length: (length)
 */
void _nx_hex_decode_byte_array(char *hex,
			       int length,
			       nexus_byte_t *bytes)
{
    int i, j;
    char *h = hex;
    nexus_stdio_lock();
    for (i = 0; i < length; i++)
    {
	sscanf(h, "%2x", &j);
	bytes[i] = (nexus_byte_t) j;
	h += 2;
    }
    nexus_stdio_unlock();
} /* _nx_hex_decode_byte_array() */


#ifdef BUILD_DEBUG
nexus_bool_t NexusDbgState( unsigned long catagory,
			    unsigned long module,
			    unsigned long operation,
			    unsigned long level )
{
    int i=0;
/*
 *   nexus_printf("trying to match %x:%x:%x:%x\n",catagory, module,
 *		 operation, level);
 */
    while( i < NEXUS_MAX_DEBUG_LEVELS )
    {
/*	nexus_printf("checking %x:%x:%x:%x\n",
 * 	             _nx_debug_levels[i].catagory,
 *		     _nx_debug_levels[i].module,
 *		     _nx_debug_levels[i].operation,
 *		     _nx_debug_levels[i].level );
 */
	if(   ( _nx_debug_levels[i].catagory  & catagory  )
	   && ( _nx_debug_levels[i].module    & module    )
	   && ( _nx_debug_levels[i].operation & operation )
	   && ( _nx_debug_levels[i].level     >= level    ) )
	{
	    return NEXUS_TRUE;
	}
	i++;
    }
    
    return NEXUS_FALSE;
}
#endif /* BUILD_DEBUG */


#ifndef HAVE_STRTOUL
/*
 * SunOS 4.1.x does not have strtoul().
 * This version was snarfed from FreeBSD.
 */

/*
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <limits.h>

/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
unsigned long strtoul(const char *nptr,
		      char **endptr,
		      int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
		errno = ERANGE;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
} /* strtoul() */

#endif /* !HAVE_STRTOUL */

/*
 * _nx_write_blocking()
 */
int _nx_write_blocking(int fd, void *buf, size_t size)
{
    nx_barrier_t write_barrier;

    nexus_mutex_init(&write_barrier.mutex, (nexus_mutexattr_t *)NULL);
    nexus_cond_init(&write_barrier.cond, (nexus_condattr_t *)NULL);
    write_barrier.done = NEXUS_FALSE;
    write_barrier.rc = 0;
    
    nexus_fd_register_for_write(fd,
				buf,
				size,
				blocking_write_callback,
				blocking_error_callback,
				(void *) &write_barrier);

    nexus_mutex_lock(&write_barrier.mutex);
    while(!write_barrier.done)
    {
        nexus_cond_wait(&write_barrier.cond, &write_barrier.mutex);
    }
    nexus_mutex_unlock(&write_barrier.mutex);

    nexus_mutex_destroy(&write_barrier.mutex);
    nexus_cond_destroy(&write_barrier.cond);

    return write_barrier.rc;
} /* _nx_write_blocking() */

/*
 * _nx_read_blocking()
 */
int _nx_read_blocking(int fd, void *buf, size_t size)
{
    nx_barrier_t read_barrier;

    nexus_mutex_init(&read_barrier.mutex, (nexus_mutexattr_t *)NULL);
    nexus_cond_init(&read_barrier.cond, (nexus_condattr_t *)NULL);
    read_barrier.done = NEXUS_FALSE;
    read_barrier.rc = 0;

    nexus_fd_register_for_read(fd,
			       buf,
			       size,
			       size,
			       blocking_read_callback,
			       blocking_error_callback,
			       (void *) &read_barrier);

    nexus_mutex_lock(&read_barrier.mutex);
    while(!read_barrier.done)
    {
        nexus_cond_wait(&read_barrier.cond, &read_barrier.mutex);
    }
    nexus_mutex_unlock(&read_barrier.mutex);

    nexus_mutex_destroy(&read_barrier.mutex);
    nexus_cond_destroy(&read_barrier.cond);

    return read_barrier.rc;
} /* _nx_blocking_read() */
			       

/*
 * blocking_write_callback()
 */
static void blocking_write_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes)
{
    nx_barrier_t *barrier = (nx_barrier_t *)arg;
    nexus_mutex_lock(&barrier->mutex);
    barrier->done = NEXUS_TRUE;
    nexus_cond_signal(&barrier->cond);
    nexus_mutex_unlock(&barrier->mutex);
} /* blocking_write_callback() */


/*
 * blocking_read_callback()
 */
static void blocking_read_callback(void *arg,
				   int fd,
				   char *buf,
				   size_t nbytes,
				   char **new_buf,
				   size_t *new_max_nbytes,
				   size_t *new_wait_for_nbytes)
{
    nx_barrier_t *barrier = (nx_barrier_t *)arg;
    nexus_mutex_lock(&barrier->mutex);
    barrier->done = NEXUS_TRUE;
    nexus_cond_signal(&barrier->cond);
    nexus_mutex_unlock(&barrier->mutex);
} /* blocking_read_callback() */


/*
 * blocking_error_callback()
 */
static void blocking_error_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes,
				    int error)
{
    nx_barrier_t *barrier = (nx_barrier_t *)arg;
    nexus_mutex_lock(&barrier->mutex);
    barrier->done = NEXUS_TRUE;
    barrier->rc = (error == 0 ? -1 : error);
    nexus_cond_signal(&barrier->cond);
    nexus_mutex_unlock(&barrier->mutex);
} /* blocking_error_callback() */
