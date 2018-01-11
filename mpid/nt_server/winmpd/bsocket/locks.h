#ifndef LOCKS_H
#define LOCKS_H

#include <stdio.h>

extern int g_nLockSpinCount;

#ifdef HAVE_MUTEX_INIT
/*   Only known system is Solaris */
#include <sys/systeminfo.h>
#include <sys/processor.h>
#include <sys/procset.h>
#include <synch.h>
#include <string.h>

typedef mutex_t                 MPIDU_Lock_t;
#define MPIDU_Init_lock(lock)   mutex_init(lock,USYNC_PROCESS,(void *)NULL)
#define MPIDU_Lock(lock)        mutex_lock(lock)
#define MPIDU_Unlock(lock)      mutex_unlock(lock)
#define MPIDU_Free_lock(lock)   mutex_destroy(lock)
static inline void MPIDU_Busy_wait( MPIDU_Lock_t *lock )
{
    int i;
    MPID_PROFILE_IN(MPIDU_BUSY_WAIT);
    mutex_lock(lock);
    mutex_unlock(lock);
    MPID_PROFILE_OUT(MPIDU_BUSY_WAIT);
}

#else

#ifdef USE_BUSY_LOCKS
#ifdef HAVE_MUTEX_INIT
typedef mutex_t MPIDU_Lock_t;
#else
typedef volatile long MPIDU_Lock_t;
#endif
#else
#ifdef HAVE_NT_LOCKS
typedef HANDLE MPIDU_Lock_t;
#elif defined(HAVE_PTHREAD_H)
typedef pthread_mutex_t MPIDU_Lock_t;  
#else
#error You must have some sort of locking mechanism for shared memory.
#endif
#endif

#include <errno.h>
#ifdef HAVE_WINDOWS_H
#define FD_SETSIZE 256
#include <winsock2.h>
#include <windows.h>
#endif

#ifdef USE_BUSY_LOCKS

static inline void MPIDU_Init_lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_MUTEX_INIT
    int err;
#endif
    MPID_PROFILE_IN(MPIDU_INIT_LOCK);
#ifdef HAVE_MUTEX_INIT
    memset(lock, 0, sizeof(MPIDU_Lock_t));
    err = mutex_init(lock, USYNC_PROCESS, 0);
    if (err)
      printf("mutex_init error: %d\n", err);
#else
    *(lock) = 0;
#endif
    MPID_PROFILE_OUT(MPIDU_INIT_LOCK);
}

static inline void MPIDU_Lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_MUTEX_INIT
    int err;
    err = mutex_lock(lock);
    if (err)
      printf("mutex_lock error: %d\n", err);
#else
    int i;
    MPID_PROFILE_IN(MPIDU_BUSY_LOCK);
    for (;;)
    {
        for (i=0; i<g_nLockSpinCount; i++)
        {
            if (*lock == 0)
            {
#ifdef HAVE_INTERLOCKEDEXCHANGE
                if (InterlockedExchange((LPLONG)lock, 1) == 0)
                {
                    /*printf("lock %x\n", lock);fflush(stdout);*/
                    MPID_PROFILE_OUT(MPIDU_BUSY_LOCK);
                    return;
                }
#elif defined(HAVE_COMPARE_AND_SWAP)
                if (compare_and_swap(lock, 0, 1) == 1)
                {
                    MPID_PROFILE_OUT(MPIDU_BUSY_LOCK);
                    return;
                }
#else
#error Atomic memory operation needed to implement busy locks
#endif
            }
        }
        MPIDU_Yield();
    }
    MPID_PROFILE_OUT(MPIDU_BUSY_LOCK);
#endif
}

static inline void MPIDU_Unlock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_MUTEX_INIT
    int err;
#endif
    MPID_PROFILE_IN(MPIDU_UNLOCK);
#ifdef HAVE_MUTEX_INIT
    err = mutex_lock(lock);
    if (err)
      printf("mutex_unlock error: %d\n", err);
#else
    *(lock) = 0;
#endif
    MPID_PROFILE_OUT(MPIDU_UNLOCK);
}

static inline void MPIDU_Busy_wait( MPIDU_Lock_t *lock )
{
    int i;
#ifdef HAVE_MUTEX_INIT
    int err;
#endif
    MPID_PROFILE_IN(MPIDU_BUSY_WAIT);
#ifdef HAVE_MUTEX_INIT
    err = mutex_lock(lock);
    if (err)
      printf("mutex_lock error: %d\n", err);
    err = mutex_unlock(lock);
    if (err)
      printf("mutex_unlock error: %d\n", err);
#else
    for (;;)
    {
        for (i=0; i<g_nLockSpinCount; i++)
            if (!*lock)
            {
                MPID_PROFILE_OUT(MPIDU_BUSY_WAIT);
                return;
            }
        MPIDU_Yield();
    }
#endif
    MPID_PROFILE_OUT(MPIDU_BUSY_WAIT);
}

static inline void MPIDU_Free_lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_MUTEX_INIT
    int err;
#endif
    MPID_PROFILE_IN(MPIDU_FREE_LOCK);
#ifdef HAVE_MUTEX_INIT
    err = mutex_destroy(lock);
    if (err)
      printf("mutex_destroy error: %d\n", err);
#endif
    MPID_PROFILE_OUT(MPIDU_FREE_LOCK);
}

#else

void MPIDU_Init_lock( MPIDU_Lock_t *lock );
void MPIDU_Lock( MPIDU_Lock_t *lock );
void MPIDU_Unlock( MPIDU_Lock_t *lock );
void MPIDU_Free_lock( MPIDU_Lock_t *lock );
void MPIDU_Busy_wait( MPIDU_Lock_t *lock );

#endif /* #ifdef USE_BUSY_LOCKS */
#endif /* #ifdef HAVE_MUTEX_INIT */

/*@
   MPIDU_Compare_swap - 

   Parameters:
+  void **dest
.  void *new_val
.  void *compare_val
.  MPIDU_Lock_t *lock
-  void **original_val

   Notes:
@*/
static inline int MPIDU_Compare_swap( void **dest, void *new_val, void *compare_val,            
                        MPIDU_Lock_t *lock, void **original_val )
{
    /* dest = pointer to value to be checked (address size)
       new_val = value to set dest to if *dest == compare_val
       original_val = value of dest prior to this operation */

    MPID_PROFILE_IN(MPIDU_COMPARE_SWAP);
#ifdef HAVE_NT_LOCKS
#ifdef USE_VC6_HEADERS
    *original_val = (void*)InterlockedCompareExchange(dest, new_val, compare_val);
#else
    *original_val = InterlockedCompareExchangePointer(dest, new_val, compare_val);
#endif
#elif defined(HAVE_COMPARE_AND_SWAP)
    if (compare_and_swap((volatile long *)dest, (long)compare_val, (long)new_val))
        *original_val = new_val;
#elif defined(HAVE_PTHREAD_H) || defined(HAVE_MUTEX_INIT)
    MPIDU_Lock( lock );

    *original_val = *dest;
    
    if ( *dest == compare_val )
        *dest = new_val;

    MPIDU_Unlock( lock );
#else
#error Locking functions not defined
#endif

    MPID_PROFILE_OUT(MPIDU_COMPARE_SWAP);
    return 0;
}

#endif
