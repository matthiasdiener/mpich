/*
 * p0_trace.c
 *
 * Code for printing stack tracebacks.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_trace.c,v 1.3 1996/02/28 20:43:20 patton Exp $";


#include "p0_internal.h"

#ifdef HAVE_STRUCT_TBTABLE

#define HAVE_TRACEBACK

/*
 * Synopsis:
 *	none
 *
 * Description:
 *	Describes the frame layout and TOC layout for the '6000
 *
 * Written by:
 *	Clem Dickey
 *
 * (C) Copyright IBM Corp. 1991
 *
 * $Id: hw_stack.h,v 1.9 92/09/30 14:06:09 clem Exp Locker: clem
 */

#include <sys/debug.h>

typedef void code;

struct frame;

struct links
{
    struct frame *sp;
    long cr;
    code *lr;
    code *buff[2];
    int *toc;
};

struct frame
{
    struct links linkarea;
    int parms[8];
};

struct tocentry
{
    code *module;
    int *toc;
    void *pdsa; /*environment pointer "for languages such as Pascal and PL/I"*/
};

static struct tbtable *tbpFindTracebackTable( struct frame *pFrame );
static struct frame *fpThisFrame( int );
static struct frame *fpPrevFrame( struct frame * );
static int cwSizeOfFrame( struct frame * );
static unsigned cwCopyFrame( int *, struct frame *, struct frame * );

/*
 * Synopsis:
 *	none
 *
 * Description:
 *	Implementation of machine-specific functions for the AT&T C++
 *	Coroutine Library
 *
 * Written by:
 *	Clem Dickey
 *
 * (C) Copyright IBM Corp. 1991
 
char rcsid[] = "$Id: hw_stack.C,v 1.6 92/09/30 14:05:29 clem Exp Locker: clem";
 */


/*
 * Given a frame (which includes a Link Register), return the traceback table
 * for that frame.
 */

static struct tbtable *tbpFindTracebackTable( struct frame *pFrame )
{
    int *wp = (int *)pFrame->linkarea.lr;
    while(*wp++);
    return (struct tbtable *)wp;
}

/*
 * return the caller's frame pointer.  It is 6 words below the address of
 * our parameter, in rios stack format.
 */

static struct frame *fpThisFrame(int i)
{
    return (struct frame *)( (char *)&i - sizeof(struct links) );
}

static struct frame *fpPrevFrame( struct frame *fpCurrent )
{
    return fpCurrent->linkarea.sp;
}

static int cwSizeOfFrame( struct frame *fp )
{
    return (int *)fpPrevFrame( fp ) - (int *)fp;
}

static void num_to_string(int n, char *str)
{
    int done = 0;
    int ones;
    int neg;
    char tmp[30], *s, *t;

    if (n < 0)
    {
	neg = 1;
	n = -n;
    }
    else
    {
	neg = 0;
    }

    s = tmp;
    while (!done)
    {
	ones = n % 10;
	n /= 10;

	*s++ = ones + 48;
	done = (n == 0);
    }
    s--;
    t = str;
    if (neg)
    {
	*t++ = '-';
    }
    while (s >= tmp)
    {
	*t = *s;
	t++;
	s--;
    }
    *t = '\0';
	
}

/*
 * This routine is the first routine that is called after
 * the struct sigcontext has been pushed onto the stack.
 * The sigcontext contains the entire machine context..
 * which may be used to determine the stack pointer before
 * the signal was received..
 */
static char trigger_name[] = "cma___sig_sync_term";

static void get_frame_name(struct frame *f, char *name, int name_len)
{
    struct tbtable *tbpTrace;
    int n_chars;

    name_len--;
    
    tbpTrace = tbpFindTracebackTable( f );
    if (tbpTrace->tb.version==0 && tbpTrace->tb.name_present)
    {
	char *c = (char *)&tbpTrace->tb_ext;
	struct tbtable_ext *x=0;
	/* discard missing fields */
	if (tbpTrace->tb.fixedparms+tbpTrace->tb.floatparms==0)
	    c-=sizeof x->parminfo;
	if (tbpTrace->tb.has_tboff==0) c-=sizeof x->tb_offset;
	if (tbpTrace->tb.int_hndl==0) c-=sizeof x->hand_mask;
	if (tbpTrace->tb.has_ctl)
	{
	    x = (struct tbtable_ext *) c;
	    c+=x->ctl_info*sizeof x->ctl_info_disp;
	} else {
	    c-=sizeof x->ctl_info;
	}
	c-=sizeof x->ctl_info_disp;
	x = (struct tbtable_ext *) c;

	if (name_len > x->name_len)
	    n_chars = x->name_len;
	else
	    n_chars = name_len;

	strncpy(name, x->name, n_chars);
	name[n_chars] = '\0';
    }
    else
    {
	strcpy(name, "?");
    }
}

void ports0_traceback(char *output_prefix)
{
    char line[1024];
    char num[30];
    struct frame *f;
    int fd;

    if (!output_prefix)
	output_prefix = "";
    
    strcpy(line, output_prefix);
    strcat(line, "Traceback:\n");

    if (_p0_stdout != (FILE *) NULL)
	fd = fileno(_p0_stdout);
    else
	fd = 1;

    write(fd, line, strlen(line));
    
    f = fpThisFrame(0);

    while (f)
    {
	char name[100];
	get_frame_name(f, name, sizeof(name));
	strcpy(line, output_prefix);
	strcat(line, name);
	strcat(line, "\n");
	write(fd, line, strlen(line));
	if( !strcmp( name, trigger_name ) )
	{
	    int proc_pid;
	    int *nf;
	    
	    proc_pid = getpid();

	    /*
	     * we just slammed into the roof.. 620 bytes of sigcontext
	     * is a nasty thing to hit with your head...
	     */
	    nf = (int *) f;
	    nf += 34;
	    
	    /* now we should be close to the edge between our local
	     * variables and the start of the sigcontext..
	     * the uncertainty arises from the fact that the number
	     * of saved registers can vary..
	     * (though this should be available with a lot of mucking
	     *  by reading throught the tbtable)
	     */
	    while( (*nf) != proc_pid ) nf++;
	    
	    /*
	     * nf should now point to the pid segment of the sigcontext.
	     * (if you are interested.. the very next field contains
	     * the signal number that put us into this stupid situation
	     * to begin with)
	     */
	    nf += 49;
	    
	    /*
	     * ok, now nf should contain the value of the stack pointer
	     * just before everything went to hell.
	     * and hopefully it will point to a valid stack frame.
	     */
	    f = (struct frame *)nf;
	}
	f = fpPrevFrame(f);
    }
} /* ports0_traceback() */

#endif /* HAVE_STRUCT_TBTABLE */



#ifndef HAVE_TRACEBACK

void ports0_traceback(char *output_prefix)
{
} /* ports0_traceback() */

#endif /* HAVE_TRACEBACK */
