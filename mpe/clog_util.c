#include "clogimpl.h"

void CLOG_dumplog()
{
    CLOG_BLOCK *block;

    block = CLOG_first;
    while (block) {
	printf("block at 0x%x: \n", (char *) block);
	CLOG_dumpblock(block->data);
	block = block->next;
    }
}

void CLOG_outblock( p )
double *p;
{
    CLOG_dumpblock(p);		/* for the time being */
}

void CLOG_dumpblock( p )
double *p;
{
    int         rtype;
    CLOG_HEADER *h;

    rtype = CLOG_UNDEF;
    while (rtype != CLOG_ENDBLOCK && rtype != CLOG_ENDLOG) {
	h = (CLOG_HEADER *) p;
	rtype = h->rectype;
	
	printf("ts=%f type=", h->timestamp);
	CLOG_rectype(h->rectype); /* print record type */
	printf(" len=%d, pid=%d ", h->length, h->procid);
	p = (double *) (h->rest);	/* skip to end of header */
	switch (rtype) {
	case CLOG_MSGEVENT:
	    printf("et="); CLOG_msgtype(((CLOG_MSG *) p)->etype);
	    printf(" tg=%d ",      ((CLOG_MSG *) p)->tag);
	    printf("prt=%d ",      ((CLOG_MSG *) p)->partner);
	    printf("cm=%d ",       ((CLOG_MSG *) p)->comm);
	    printf("sz=%d ",       ((CLOG_MSG *) p)->size);
	    printf("loc=%d\n",     ((CLOG_MSG *) p)->srcloc);
	    p = (double *)        (((CLOG_MSG *) p)->end);
	    break;
	case CLOG_COLLEVENT:
	    printf("et="); CLOG_colltype(((CLOG_COLL *) p)->etype);
	    printf(" root=%d ",   ((CLOG_COLL *) p)->root);
	    printf("cm=%d ",      ((CLOG_COLL *) p)->comm);
	    printf("sz=%d\n",     ((CLOG_COLL *) p)->size);
	    p = (double *)       (((CLOG_COLL *) p)->end);
	    break;
	case CLOG_SRCLOC:
	    printf("srcid=%d ",    ((CLOG_SRC *) p)->srcloc);
	    printf("line=%d ",     ((CLOG_SRC *) p)->lineno);
	    printf("file=%s\n",    ((CLOG_SRC *) p)->filename);
	    p = (double *)        (((CLOG_SRC *) p)->end);
	    break;
	case CLOG_COMMEVENT:
	    printf("et="); CLOG_commtype(((CLOG_MSG *) p)->etype);
	    printf(" pt=%d ",     ((CLOG_COMM *) p)->parent);
	    printf("ncomm=%d ",   ((CLOG_COMM *) p)->newcomm);
	    printf("srcid=%d\n",  ((CLOG_COMM *) p)->srcloc);
	    p = (double *)       (((CLOG_COMM *) p)->end);
	    break;
	case CLOG_STATEDEF:
	    printf("id=%d ",     ((CLOG_STATE *) p)->stateid);
	    printf("start=%d ",  ((CLOG_STATE *) p)->startetype);
	    printf("end=%d ",    ((CLOG_STATE *) p)->endetype);
	    printf("color=%s ",  ((CLOG_STATE *) p)->color);
	    printf("desc=%s\n",  ((CLOG_STATE *) p)->description);
	    p = (double *)      (((CLOG_STATE *) p)->end);
	    break;
	case CLOG_EVENTDEF:
	    printf("id=%d ",     ((CLOG_EVENT *) p)->etype);
	    printf("desc=%s\n",  ((CLOG_EVENT *) p)->description);
	    p = (double *)      (((CLOG_EVENT *) p)->end);
	    break;
	case CLOG_SHIFT:
	    printf("shift=%f\n",((CLOG_TSHIFT *) p)->timeshift);
	    p = (double *)     (((CLOG_TSHIFT *) p)->end);
	    break;
	case CLOG_RAWEVENT:
	    printf("id=%d ",       ((CLOG_RAW *) p)->etype);
	    printf("data=%d ",     ((CLOG_RAW *) p)->data);
	    printf("srcid=%d ",    ((CLOG_RAW *) p)->srcloc);
	    printf("desc=%s\n",    ((CLOG_RAW *) p)->string);
	    p = (double *)        (((CLOG_RAW *) p)->end);
	    break;
	case CLOG_ENDBLOCK:
	    printf("end of block\n");
	    break;
	case CLOG_ENDLOG:
	    printf("end of log\n");
	    break;
	default:
	    printf("unrecognized record type\n");
	}
    }
}

/*@
     CLOG_reclen - get length (in doubles) of log record by type
@*/
int CLOG_reclen( type )
int type;
{
    int restlen;

    switch (type) {
    case CLOG_ENDLOG:
	restlen = 1; break;
    case CLOG_ENDBLOCK:
	restlen = 1; break;
    case CLOG_MSGEVENT:
	restlen = sizeof(CLOG_MSG) / sizeof(double);	break;
    case CLOG_COLLEVENT:
	restlen = sizeof(CLOG_COLL) / sizeof(double);   break;
    case CLOG_COMMEVENT:
	restlen = sizeof(CLOG_COMM) / sizeof(double);	break;
    case CLOG_EVENTDEF:
	restlen = sizeof(CLOG_EVENT) / sizeof(double);	break;
    case CLOG_STATEDEF:
	restlen = sizeof(CLOG_STATE) / sizeof(double);	break;
    case CLOG_SHIFT:
	restlen = sizeof(CLOG_TSHIFT) / sizeof(double);	break;
    case CLOG_RAWEVENT:
	restlen = sizeof(CLOG_RAW) / sizeof(double);	break;
    case CLOG_SRCLOC:
	restlen = sizeof(CLOG_SRC) / sizeof(double);	break;
    default:
	printf("CLOG: Can't get length of unknown record type %d\n", type);
    }
    /* The above caculation is off by 2 because of the "rest" and "end"
       markers which are overwritten, so we subtract 2 in the next line.
       (For ENDLOG and ENDBLOCK restlen is set to 1 to make this work.) */

    return ( (sizeof(CLOG_HEADER) / sizeof(double)) + restlen - 2 );
}

/*@
     CLOG_msgtype - print communication event type

. etype - event type for pt2pt communication event

@*/
void CLOG_msgtype( etype )
int etype;
{
    switch (etype) {
	/* none predefined */
    default:    printf("unk(%d)", etype);
    }
}

/*@
     CLOG_commtype - print communicator creation event type

. etype - event type for communicator creation event

@*/
void CLOG_commtype( etype )
int etype;
{
    switch (etype) {
    case INIT:   printf("init"); break;
    case DUP:    printf("dup "); break;
    case SPLIT:  printf("splt"); break;
    case CARTCR: printf("crtc"); break;
    case COMMCR: printf("cmmc"); break;
    case CFREE:  printf("free"); break;
    default:     printf("unknown(%d)", etype);
    }
}

void CLOG_colltype( etype )
int etype;
{
    switch (etype) {
        /* none predefined */
    default:      printf("unk(%d)", etype);
    }
}

/*@
     CLOG_rectype - print log record type

. rtype - record type

@*/
void CLOG_rectype( type )
int type;
{
    switch (type) {
    case CLOG_ENDLOG:    printf("elog"); break;
    case CLOG_ENDBLOCK:  printf("eblk"); break;
    case CLOG_UNDEF:     printf("udef"); break;
    case CLOG_MSGEVENT:  printf("msg "); break;
    case CLOG_COLLEVENT: printf("coll"); break;
    case CLOG_COMMEVENT: printf("comm"); break;
    case CLOG_EVENTDEF:  printf("edef"); break;
    case CLOG_STATEDEF:  printf("sdef"); break;
    case CLOG_SRCLOC:    printf("loc "); break;
    case CLOG_SHIFT:     printf("shft"); break;
    case CLOG_RAWEVENT:  printf("raw "); break;
    default:             printf("unknown(%d)", type);
    }
}

