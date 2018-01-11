#include "p2p.h"

void *p2p_shmalloc();

struct p2p_mon_queue;  /* for c++ folks */
struct p2p_monitor {
    p2p_lock_t mon_lock;
    struct p2p_mon_queue *qs;
};

typedef struct p2p_monitor p2p_monitor_t;


struct p2p_mon_queue {
    int count;
    p2p_lock_t delay_lock;
};


struct p2p_getsub_monitor {
    struct p2p_monitor m;
    int sub;
};

typedef struct p2p_getsub_monitor p2p_getsub_monitor_t;

#define p2p_getsub(gs,s,max,nprocs) p2p_getsubs(gs,s,max,nprocs,1)

struct p2p_barrier_monitor {
    struct p2p_monitor m;
};

typedef struct p2p_barrier_monitor p2p_barrier_monitor_t;

struct p2p_askfor_monitor {
    struct p2p_monitor m;
    int pgdone;
    int pbdone;
};

typedef struct p2p_askfor_monitor p2p_askfor_monitor_t;

