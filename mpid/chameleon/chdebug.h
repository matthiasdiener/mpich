#ifdef MPID_DEBUG_ALL  
                       /* #DEBUG_START# */
#ifdef MEMCPY
#undef MEMCPY
#endif
#define MEMCPY(a,b,c)\
{if (MPID_DebugFlag) {\
    fprintf( MPID_DEBUG_FILE, \
	    "[%d]R About to copy to %d from %d (n=%d) (%s:%d)...\n", \
	    MPID_MyWorldRank, a, b, c, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE ); }\
memcpy( a, b, c );}

#define DEBUG_PRINT_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	fprintf( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, MPID_PKT_SEND_GET(pkt,tag), dest, \
	       MPID_PKT_SEND_GET(pkt,context_id), \
	       MPID_PKT_SEND_GET(pkt,len) );\
	MPID_Print_mode( MPID_DEBUG_FILE, MPID_PKT_SEND_ADDR(pkt) );\
	fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define DEBUG_PRINT_RECV_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	fprintf( MPID_DEBUG_FILE,\
"[%d]%s for tag = %d, source = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, MPID_PKT_RECV_GET(pkt,tag), dest, \
	       MPID_PKT_RECV_GET(pkt,context_id), \
	       MPID_PKT_RECV_GET(pkt,len) );\
	MPID_Print_mode( MPID_DEBUG_FILE, MPID_PKT_RECV_ADDR(pkt) );\
	fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    fprintf( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	    MPID_MyWorldRank, msg, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}}
	
#define DEBUG_PRINT_PKT(msg,pkt)    
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d]%s (%s:%d)\n", MPID_MyWorldRank, msg, __FILE__, __LINE__ );
    MPID_Print_packet( MPID_DEBUG_FILE, MPID_PKT_RECV_ADDR(pkt) );
    }
#endif                  /* #DEBUG_END# */

                       /* #DEBUG_END# */
     
#else
#define DEBUG_PRINT_PKT(msg,pkt)
#define DEBUG_PRINT_MSG(msg)
#define DEBUG_PRINT_SEND_PKT(msg,pkt)
#define DEBUG_PRINT_RECV_PKT(msg,pkt)
#define DEBUG_PRINT_SYNCACK(msg,pkt)

#endif
