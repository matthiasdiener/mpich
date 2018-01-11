/* clog.c */
#include <stdlib.h>
#include <string.h>
#include "clogimpl.h"

char   CLOG_outdir[CLOG_DIR_LEN];
int    CLOG_status = 2;	/* initialized to CLOG not init, but logging on */
int    CLOG_Comm;		/* Default communicator */
void   *CLOG_ptr;		/* pointer into buffer, where next rec goes */
void   *CLOG_block_end;		/* pointer to end of buffer */
CLOG_BLOCK *CLOG_first, *CLOG_currbuff; /* blocks of buffer */
int    intsperdouble, charsperdouble;
int    CLOG_srcid = 900;	/* next id for source code location */
int    CLOG_nextevent = CLOG_MAXEVENT;
int    CLOG_nextstate = CLOG_MAXSTATE;
char   CLOG_execname[256];	/* name used for naming logfile (executable) */

/*@
    CLOG_Init - Initialize for CLOG logging
@*/
void CLOG_Init( )
{
    CLOG_Comm      = 0;		/* until we pass communicator in to CLOG_Init*/
    CLOG_timeinit();		/* initialize timer */
    CLOG_newbuff(&CLOG_first);	/* get first buffer */
    CLOG_status	  &= 0x01;	/* set initialized  */
    intsperdouble  = sizeof(double) / sizeof(int);
    charsperdouble = sizeof(double) / sizeof(char);
}

/*@
    CLOG_Finalize - Finalize  CLOG logging
@*/
void CLOG_Finalize( )
{
    CLOG_LOGENDLOG();
}

/*@
    CLOG_newbuff - get and initialize new block of buffer

Input Parameters:

. bufptr - pointer to be filled in with address of new block

@*/
void CLOG_newbuff( bufptr )
CLOG_BLOCK **bufptr;
{
    *bufptr = (CLOG_BLOCK *) MALLOC ( sizeof (CLOG_BLOCK) );
    (*bufptr)->next = NULL;
    CLOG_currbuff = *bufptr;
    CLOG_ptr = (*bufptr)->data;
    CLOG_block_end = (void *) ((char *) CLOG_ptr + CLOG_BLOCK_SIZE);
}

/************* to become macros once debugged ***********************/

void CLOG_put_hdr( type )
int type;
{
    if (((char *) CLOG_ptr + CLOG_MAX_REC_LEN) >= (char *) CLOG_block_end) {
	CLOG_LOGENDBLOCK();
	CLOG_newbuff(&CLOG_currbuff->next);
    }
    ((CLOG_HEADER *) CLOG_ptr)->timestamp = CLOG_timestamp(); 
    ((CLOG_HEADER *) CLOG_ptr)->rectype   = type;
				/* int length will be filled in later */
				/* int procid will be filled in later */
    CLOG_ptr = ((CLOG_HEADER *) CLOG_ptr)->rest; /* point past header */
}

void CLOG_LOGMSG( etype, tag, partner, comm, size )
int etype, tag, partner, comm, size;
{
    if (CLOG_OK) {                                                    
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_MSGEVENT);
	    ((CLOG_MSG *) CLOG_ptr)-> etype   = etype;
	    ((CLOG_MSG *) CLOG_ptr)-> tag     = tag;
	    ((CLOG_MSG *) CLOG_ptr)-> partner = partner;
	    ((CLOG_MSG *) CLOG_ptr)-> comm    = comm;
	    ((CLOG_MSG *) CLOG_ptr)-> size    = size;
	    ((CLOG_MSG *) CLOG_ptr)-> srcloc  = srcloc;
	    CLOG_ptr			      = ((CLOG_MSG *) CLOG_ptr)->end;
	}
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;
}

void CLOG_LOGRAW( etype, data, string )
int etype, data;
char *string;
{
    if (CLOG_OK) {                                                    
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_RAWEVENT);
	    ((CLOG_RAW *) CLOG_ptr)-> etype   = etype;
	    ((CLOG_RAW *) CLOG_ptr)-> data    = data;
	    ((CLOG_RAW *) CLOG_ptr)-> srcloc  = srcloc;
	    if (string)
		strncpy(((CLOG_RAW *)CLOG_ptr)->string, string,
			sizeof(CLOG_DESC));
	    else
		* ((char *) ((CLOG_RAW *)CLOG_ptr)->string) = '\0';

	    CLOG_ptr			      = ((CLOG_RAW *) CLOG_ptr)->end;
	    * ((char *) CLOG_ptr - 1)	= '\0'; /* ensure null terminated */
	}
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;                                                  
}

void CLOG_LOGCOLL( etype, root, size, comm )
int etype, root, size, comm;
{
    if (CLOG_OK) {                                                    
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_COLLEVENT);
	    ((CLOG_COLL *) CLOG_ptr)-> etype   = etype;
	    ((CLOG_COLL *) CLOG_ptr)-> root    = root;
	    ((CLOG_COLL *) CLOG_ptr)-> comm    = comm;
	    ((CLOG_COLL *) CLOG_ptr)-> size    = size;
	    ((CLOG_COLL *) CLOG_ptr)-> srcloc  = srcloc;
	    CLOG_ptr			      = ((CLOG_COLL *) CLOG_ptr)->end;
	}
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;                                                  
}

void CLOG_LOGCOMM( etype, parent, newcomm )
int etype, parent, newcomm;
{
    if (CLOG_OK) {
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_COMMEVENT);
	    ((CLOG_COMM *) CLOG_ptr)-> etype   = etype;
	    ((CLOG_COMM *) CLOG_ptr)-> parent  = parent;
	    ((CLOG_COMM *) CLOG_ptr)-> newcomm = newcomm;
	    ((CLOG_COMM *) CLOG_ptr)-> srcloc  = srcloc;
	    CLOG_ptr			       = ((CLOG_COMM *) CLOG_ptr)->end;
	}	
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;                                                  
}

void CLOG_LOGSTATE( stateid, startetype, endetype, color, description )
int stateid, startetype, endetype;
char *color, *description;
{
    if (CLOG_OK) {
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_STATEDEF);
	    ((CLOG_STATE *) CLOG_ptr)-> stateid	   = stateid;
	    ((CLOG_STATE *) CLOG_ptr)-> startetype = startetype;
	    ((CLOG_STATE *) CLOG_ptr)-> endetype   = endetype;
	    if (color)
		strncpy(((CLOG_STATE *)CLOG_ptr)->color, color,
			sizeof(CLOG_CNAME));
	    else
		* ((char *) ((CLOG_STATE *)CLOG_ptr)->color) = '\0';

	    if (description)
		strncpy(((CLOG_STATE *)CLOG_ptr)->description, description,
			sizeof(CLOG_CNAME));
	    else
		* ((char *) ((CLOG_STATE *)CLOG_ptr)->description) = '\0';

	    CLOG_ptr	              = ((CLOG_STATE *) CLOG_ptr)->end;
	    * ((char *) CLOG_ptr - 1) = '\0'; /* ensure null terminated */
	}	
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;                                                  
}

void CLOG_LOGEVENT( etype, description )
int etype;
char *description;
{
    if (CLOG_OK) {
	{   
	    static int first = 1;
	    static int srcloc;
	    if (first) {
		first = 0;
		srcloc = CLOG_srcid++;
		CLOG_LOGSRCLOC(srcloc, __LINE__, __FILE__);
	    }
	    CLOG_put_hdr(CLOG_EVENTDEF);
	    ((CLOG_EVENT *) CLOG_ptr)-> etype	  = etype;

	    if (description)
		strncpy(((CLOG_EVENT *)CLOG_ptr)->description, description,
			sizeof(CLOG_CNAME));
	    else
		* ((char *) ((CLOG_EVENT *)CLOG_ptr)->description) = '\0';

	    CLOG_ptr	       = ((CLOG_EVENT *) CLOG_ptr)->end;
	    * ((char *) CLOG_ptr - 1)	= '\0'; /* ensure null terminated */
	}	
    }                                                                 
    else if (CLOG_ERROR)                                              
	CLOG_not_init;                                                  
}

void CLOG_LOGSRCLOC( srcloc, lineno, filename )
int srcloc, lineno;
char *filename;
{
    if (CLOG_OK) {
	CLOG_put_hdr(CLOG_SRCLOC);
	((CLOG_SRC *) CLOG_ptr)->srcloc	= srcloc; 
	((CLOG_SRC *) CLOG_ptr)->lineno	= lineno; 
        strncpy(((CLOG_SRC *)CLOG_ptr)->filename, filename, sizeof(CLOG_FILE));
	CLOG_ptr			= ((CLOG_SRC *) CLOG_ptr)->end;
	* ((char *) CLOG_ptr - 1)	= '\0'; /* ensure null terminated */
    }
    else if (CLOG_ERROR)
	CLOG_not_init;
}

void CLOG_LOGTIMESHIFT( shift )
double shift;
{
    if (CLOG_OK) {
	CLOG_put_hdr(CLOG_SHIFT);
	((CLOG_TSHIFT *) CLOG_ptr)->timeshift = shift; 
	CLOG_ptr		= ((CLOG_TSHIFT *) CLOG_ptr)->end;
    }
    else if (CLOG_ERROR)
	CLOG_not_init;
}

void CLOG_LOGENDBLOCK()
{
    /* assumes there is room for this record */
    if (CLOG_OK) {
	((CLOG_HEADER *) CLOG_ptr)->timestamp = CLOG_timestamp(); 
	((CLOG_HEADER *) CLOG_ptr)->rectype   = CLOG_ENDBLOCK;
	CLOG_ptr = ((CLOG_HEADER *) CLOG_ptr)->rest; 
    }
    else
	if (CLOG_ERROR)                                              
	    CLOG_not_init;                                                  
}
    
void CLOG_LOGENDLOG()
{
    /* assumes there is room for this record */
    if (CLOG_OK) {
	((CLOG_HEADER *) CLOG_ptr)->timestamp = CLOG_timestamp(); 
	((CLOG_HEADER *) CLOG_ptr)->rectype   = CLOG_ENDLOG;
	CLOG_ptr = ((CLOG_HEADER *) CLOG_ptr)->rest; 
    }
    else
	if (CLOG_ERROR)                                              
	    CLOG_not_init;                                                  
}
       
/*@
    CLOG_get_new_event - obtain unused event id
@*/
int CLOG_get_new_event()
{
    return CLOG_nextevent++;
}

/*@
    CLOG_get_new_state - obtain unused state id
@*/
int CLOG_get_new_state()
{
    return CLOG_nextstate++;
}

