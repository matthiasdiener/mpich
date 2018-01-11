#ifndef REQUESTS
#define REQUESTS

typedef struct request_list_ {
    MPI_Request request;
    int         status, size, tag, otherParty;
    struct request_list_ *next;
} request_list;

#define RQ_SEND    0x1
#define RQ_RECV    0x2
#define RQ_CANCEL  0x4
/* if MPI_Cancel is called on a request, 'or' RQ_CANCEL into status.
** After a Wait* or Test* is called on that request, check for RQ_CANCEL.
** If the bit is set, check with MPI_Test_cancelled before registering
** the send/receive as 'happening'.
**
*/

#define rq_remove( head, rq ) { \
  request_list *ptr, *last; \
  ptr = head; \
  last = 0; \
  while (ptr && (ptr->request != rq)) { \
    last = ptr; \
    ptr = ptr->next; \
  } \
  if (ptr) { \
    if (!last) { \
      head = ptr->next; \
    } else { \
      last->next = ptr->next; \
    } \
    free( ptr ); }}

#define rq_add( head, tail, rq ) { \
  if (!head) { \
    head = tail = rq; \
  } else { \
    tail->next = rq; \
  }}

#define rq_find( head, req, rq ) { \
  rq = head; \
  while (rq && (rq->request != req)) rq = rq->next; }

#endif
