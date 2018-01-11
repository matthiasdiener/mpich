struct p4_queued_msg;  /* for c++ folks; defined below */
struct p4_msg_queue {
    p4_monitor_t m;
    p4_lock_t ack_lock;
    struct p4_queued_msg *first_msg, *last_msg;
};

#define CONN_ME 		1
#define CONN_REMOTE_SWITCH 	2
#define CONN_REMOTE_NON_EST 	3
#define CONN_REMOTE_EST 	4
#define CONN_SHMEM	 	5
#define CONN_CUBE 		6
#define CONN_TCMP 		7
#define CONN_REMOTE_DYING       8

#define P4_MAX_MSGLEN (1<<26)  /* Used in free_p4_msg as sanity check
				  increase as desired */

/* Returns true if node is out of allowed range */
#define CHECKNODE(node) ((node<0) || (node>=p4_num_total_ids()))

#if defined(IPSC860) || defined(CM5)  ||  defined(NCUBE)  \
                     ||  defined(SP1_EUI) || defined(SP1_EUIH)
#    define CONN_LOCAL CONN_CUBE
#else
#    if defined(TCMP)
#        define CONN_LOCAL CONN_TCMP
#    else
#        define CONN_LOCAL CONN_SHMEM
#    endif
#endif

#define XDR_PAD 4
#define XDR_INT_LEN 4
#define XDR_LNG_LEN 4
#define XDR_FLT_LEN 4
#define XDR_DBL_LEN 8
#define XDR_CHR_LEN 4
#define XDR_BUFF_LEN 4096

/* data types typically used by xdr, but also for other purposes */
#define P4NOX 0
#define P4INT 1
#define P4LNG 2
#define P4FLT 3
#define P4DBL 4


/* send_message(type,from,to,msg,len,data_type,ack_req,p4_buff_ind) */

#define  p4_send(TYPE,TO,MSG,LEN)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,P4NOX,FALSE,FALSE)
#define  p4_sendr(TYPE,TO,MSG,LEN)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,P4NOX,TRUE,FALSE)
#define  p4_sendx(TYPE,TO,MSG,LEN,DATATYPE)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,DATATYPE,FALSE,FALSE)
#define  p4_sendrx(TYPE,TO,MSG,LEN,DATATYPE)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,DATATYPE,TRUE,FALSE)
#define  p4_sendb(TYPE,TO,MSG,LEN)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,P4NOX,FALSE,TRUE)
#define  p4_sendbr(TYPE,TO,MSG,LEN)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,P4NOX,TRUE,TRUE)
#define  p4_sendbx(TYPE,TO,MSG,LEN,DATATYPE)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,DATATYPE,FALSE,TRUE)
#define  p4_sendbrx(TYPE,TO,MSG,LEN,DATATYPE)  \
    send_message(TYPE,p4_get_my_id(),TO,MSG,LEN,DATATYPE,TRUE,TRUE)

#define  p4_broadcast(TYPE,MSG,LEN)  p4_broadcastx(TYPE,MSG,LEN,P4NOX)
