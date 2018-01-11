/* 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_ibmds.h,v 1.6 1996/10/07 04:40:16 tuecke Exp $"
 */
typedef void (*PFV)(void);
typedef struct PFVSTR {
    PFV init, term;
} PFVSTR;

typedef struct context_info_s {
    unsigned int toc_size; 
    unsigned int toc_offset;
    unsigned int dataseg_size;
    long dataFP;
    char * file_name;
    int context_count;
    struct context_info_s * master_context;
    unsigned int relocation_cnt;
    unsigned int * relocations;
    struct context_info_s * (*entry_point)(char *);
    char * data_segment;
    char * toc;
    int (*NexusBoot)(void);
    void (*SwitchContext)(void *);
    int (*killfunc)(struct context_info_s *);
#ifdef NEXUS_CPP_HACK
    PFVSTR *cdtorfuncs;
    int cdtorcount;
    int dtorstainted;
#endif
} ContextInfo;

typedef struct _ibmds_handle_t
{
    ContextInfo *contextPtr;
    char *     filename;
} ibmds_handle_t;

#ifdef __cplusplus
extern "C" {
#endif
void set_context(ContextInfo *);
#ifdef __cplusplus
};
#endif
