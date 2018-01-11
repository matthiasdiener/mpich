#include "globus_gram_myjob.h"
#include "globus_nexus.h"

typedef struct _mcl {
    globus_cond_t cond;
    struct _mcl *next;
} MPID_cond_list;

/* 
 * The following Cond_Variable free list macros are written with the
 * assumption that the message_queue_lock is being held.  This prevents
 * the free list from being trashed by two threads modifying values at
 * the same time.
 *
 * The reason for the message_queue_lock being the mutex of choice is
 * that the condition variable is always associated with some kind of
 * message action.  We are getting the condition variable to get
 * notified when a message arrives.  We need to prevent the message from
 * arriving while we are trying to get setup.
 */
/* defined only in nexuspriv.c */
#ifdef NEXUSPRIV
MPID_cond_list *cond_list;
#else
extern MPID_cond_list *cond_list;
#endif

#define MPID_Init_Cond_Variables() \
{ \
    cond_list = NULL; \
}

#define MPID_Destroy_Cond_Variables() \
{ \
    MPID_cond_list *cond_var; \
 \
    for (cond_var = cond_list; cond_var; cond_var = cond_var->next) \
    { \
	globus_cond_destroy(&cond_var->cond); \
	globus_free(cond_var); \
    } \
}

#define MPID_Get_Cond_Variable(cond_var) \
{ \
    if (!cond_list) \
    { \
	cond_var = (MPID_cond_list *)globus_malloc(sizeof(MPID_cond_list)); \
	globus_cond_init(&cond_var->cond, NULL); \
    } \
    else \
    { \
	cond_var = cond_list; \
	cond_list = cond_var->next; \
    } \
    cond_var->next = NULL; \
}

/* 
 * We don't call globus_cond_destroy() because we want to reduce that
 * overhead from the next time we call MPID_Get_Cond_Variable().  All it
 * has to do is set some pointers, and we're done.
 */
#define MPID_Free_Cond_Variable(cond_var) \
{ \
    cond_var->next = cond_list; \
    cond_list = cond_var; \
}
