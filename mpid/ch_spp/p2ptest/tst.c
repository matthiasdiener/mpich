#include "p2p.h"

main()
{
    p2p_lock_t *mylock;
    int masterid = getpid();

    p2p_init(1,500000);
    mylock = (p2p_lock_t *) p2p_shmalloc(sizeof(p2p_lock_t));
    p2p_lock_init(mylock);
    p2p_create_procs(1);
    if (masterid != getpid())
	sleep(5);
    printf("%d: before the lock\n",getpid());
    p2p_lock(mylock);
    printf("%d: after  the lock\n",getpid());
    p2p_cleanup();

    for(;;) ;
}
