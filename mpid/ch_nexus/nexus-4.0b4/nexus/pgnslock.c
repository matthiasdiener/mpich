/* This value should be different for the MP and XPS Paragon */

#include <pthread.h>

const long SPIN_LIMIT = 1;

void
_nexus_spin_lock(pthread_mutex_t *slock)
{
        volatile int * lock = &(slock->lock);
        register int    i;
 
        if (lock_try_set(lock))
                return;
 
        for (i = 0; i < SPIN_LIMIT; i++)
                if (lock_try_set(lock))
                        return;
 
        while (!lock_try_set(lock))
                vp_yield();
}

void
_nexus_spin_unlock(pthread_mutex_t *slock)
{
        lock_unset(&(slock->lock));
}
