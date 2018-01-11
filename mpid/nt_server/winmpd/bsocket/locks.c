#include "bsocket.h"
#include <errno.h>
#ifdef HAVE_WINDOWS_H
#undef FD_SETSIZE
#define FD_SETSIZE 256
#include <winsock2.h>
#include <windows.h>
#endif

int g_nLockSpinCount = 100;

#ifndef USE_BUSY_LOCKS
/*@
   MPIDU_Init_lock - 

   Parameters:
+  MPIDU_Lock_t *lock

   Notes:
@*/
void MPIDU_Init_lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_NT_LOCKS
    MPID_PROFILE_IN(MPIDU_INIT_LOCK);
    *lock = CreateMutex(NULL, FALSE, NULL);
    if (*lock == NULL)
    {
        printf("error in mutex_init: %d\n", GetLastError());
    }
#elif defined(HAVE_PTHREAD_H)
    /* should be called by one process only */
    int err;
    pthread_mutexattr_t attr;
    MPID_PROFILE_IN(MPIDU_INIT_LOCK);
#ifdef HAVE_PTHREAD_MUTEXATTR_INIT
    err = pthread_mutexattr_init(&attr);
    if (err != 0)
      printf("error in pthread_mutexattr_init: %s\n", strerror(err));
#endif
#ifdef HAVE_PTHREAD_MUTEXATTR_SETPSHARED
    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if (err != 0)
      printf("error in pthread_mutexattr_setpshared: %s\n", strerror(err));

    err = pthread_mutex_init( lock, &attr );
#else
    err = pthread_mutex_init( lock, NULL );
#endif
    if ( err != 0 ) 
        printf( "error in mutex_init: %s\n", strerror(err) );
#else
#error Locking functions not defined
#endif
    MPID_PROFILE_OUT(MPIDU_INIT_LOCK);
}

/*@
   MPIDU_Lock - 

   Parameters:
+  MPIDU_Lock_t *lock

   Notes:
@*/
void MPIDU_Lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_NT_LOCKS
    DWORD dwRetVal;
    MPID_PROFILE_IN(MPIDU_LOCK);
    /*printf("nt lock %x\n", lock);fflush(stdout);*/
    dwRetVal = WaitForSingleObject(*lock, INFINITE);
    if (dwRetVal != WAIT_OBJECT_0)
    {
        if (dwRetVal == WAIT_FAILED)
            printf("error in mutex_lock: %s\n", strerror(GetLastError()));
        else
            printf("error in mutex_lock: %d\n", GetLastError());
    }
    /*printf("lock: Handle = %u\n", (unsigned long)*lock);*/
#elif defined(HAVE_PTHREAD_H)
    int err;

    MPID_PROFILE_IN(MPIDU_LOCK);
    err = pthread_mutex_lock( lock );
    if ( err != 0 ) 
        printf( "error in mutex_lock: %s\n", strerror(err) );
#else
#error Locking functions not defined
#endif
    MPID_PROFILE_OUT(MPIDU_LOCK);
}

/*@
   MPIDU_Unlock - 

   Parameters:
+  MPIDU_Lock_t *lock

   Notes:
@*/
void MPIDU_Unlock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_NT_LOCKS
    MPID_PROFILE_IN(MPIDU_UNLOCK);
    if (!ReleaseMutex(*lock))
    {
        printf("error in mutex_unlock: %d\n", GetLastError());
        printf("Handle = %u\n", (unsigned long)*lock);
    }
    /*printf("unlock: Handle = %u\n", (unsigned long)*lock);*/
#elif defined(HAVE_PTHREAD_H)
    int err;

    MPID_PROFILE_IN(MPIDU_UNLOCK);
    err = pthread_mutex_unlock( lock );
    if ( err != 0 ) 
        printf( "error in mutex_unlock: %s\n", strerror(err) );
#else
#error Locking functions not defined
#endif
    MPID_PROFILE_OUT(MPIDU_UNLOCK);
}

/*@
   MPIDU_Busy_wait - 

   Parameters:
+  MPIDU_Lock_t *lock

   Notes:
@*/
void MPIDU_Busy_wait( MPIDU_Lock_t *lock )
{
    MPID_PROFILE_IN(MPIDU_BUSY_WAIT);
    MPIDU_Lock(lock);
    MPIDU_Unlock(lock);
    MPID_PROFILE_OUT(MPIDU_BUSY_WAIT);
}

/*@
   MPIDU_Free_lock - 

   Parameters:
+  MPIDU_Lock_t *lock

   Notes:
@*/
void MPIDU_Free_lock( MPIDU_Lock_t *lock )
{
#ifdef HAVE_NT_LOCKS
    MPID_PROFILE_IN(MPIDU_FREE_LOCK);
    /*printf("Free_lock: Handle = %u\n", (unsigned long)*lock);*/
    CloseHandle(*lock);
    *lock = NULL;
#elif defined(HAVE_PTHREAD_H)
    int err;

    MPID_PROFILE_IN(MPIDU_FREE_LOCK);
    err = pthread_mutex_destroy( lock );
    if ( err != 0 ) 
    printf( "error in mutex_destroy: %s\n", strerror(err) );
#else
#error Locking functions not defined
#endif
    MPID_PROFILE_OUT(MPIDU_FREE_LOCK);
}
#endif /* #ifndef USE_BUSY_LOCKS */
