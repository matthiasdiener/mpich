/*
 * p0_test.c
 *
 * Test of ports0.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_test.c,v 1.7 1996/10/19 17:10:03 carl Exp $";


/*#define USE_MACROS*/
#include "ports0.h"
static void condition_test_proc(void);


int main(int argc, char *argv[])
{
    ports0_init(&argc, &argv, "ports0");
    condition_test_proc();
    ports0_shutdown();
    return(0);
}


/*****************************************************************
 *		CONDITION_TEST
 *****************************************************************/

static ports0_mutex_t	condition_test_mutex;
static ports0_cond_t	condition_test_cond;

static int  cond_flag1;
static int  cond_flag2;

void *condition_test_thread2(void *arg)
{
    ports0_printf("condition_test(): thread2: entering\n");

    ports0_mutex_lock(&condition_test_mutex);
    
    ports0_printf("condition_test(): thread2: signaling\n");
    cond_flag1 = PORTS0_TRUE;
    ports0_cond_signal(&condition_test_cond);
    ports0_printf("condition_test(): thread2: done signaling\n");

    ports0_printf("condition_test(): thread2: waiting for broadcast\n");
    cond_flag2 = PORTS0_FALSE;
    while (!cond_flag2)
    {
	ports0_cond_wait(&condition_test_cond, &condition_test_mutex);
    }
    ports0_printf("condition_test(): thread2: awoken\n");

    ports0_printf("condition_test(): thread2: signaling\n");
    cond_flag1 = PORTS0_TRUE;
    ports0_cond_signal(&condition_test_cond);
    ports0_printf("condition_test(): thread2: done signaling\n");

    ports0_mutex_unlock(&condition_test_mutex);
    
    ports0_printf("condition_test(): thread2: exiting\n");

    return (NULL);
} /* condition_test_thread2() */

/*
 * condition_test_proc()
 */
static void condition_test_proc(void)
{
    ports0_thread_t thread;
    
    ports0_printf("condition_test(): starting\n");
    
    ports0_mutex_init(&condition_test_mutex, (ports0_mutexattr_t *) NULL);
    ports0_cond_init(&condition_test_cond, (ports0_condattr_t *) NULL);
    
    ports0_printf("condition_test(): thread1: before lock and unlock\n");
    ports0_mutex_lock(&condition_test_mutex);
    ports0_mutex_unlock(&condition_test_mutex);
    ports0_printf("condition_test(): thread1: after lock and unlock\n");

    ports0_mutex_lock(&condition_test_mutex);

    ports0_printf("condition_test(): thread1: creating thread2\n");
    ports0_thread_create(&thread,
			(ports0_threadattr_t *) NULL,
			condition_test_thread2,
			(void *) NULL);

    ports0_printf("condition_test(): thread1: waiting for signal \n");
    cond_flag1 = PORTS0_FALSE;
    while (!cond_flag1)
    {
	ports0_cond_wait(&condition_test_cond, &condition_test_mutex);
    }
    ports0_printf("condition_test(): thread1: awoken\n");

    ports0_printf("condition_test(): thread1: broadcasting\n");
    cond_flag2 = PORTS0_TRUE;
    ports0_cond_broadcast(&condition_test_cond);
    ports0_printf("condition_test(): thread1: done broadcasting\n");

    ports0_printf("condition_test(): thread1: waiting for signal\n");
    cond_flag1 = PORTS0_FALSE;
    while (!cond_flag1)
    {
	ports0_cond_wait(&condition_test_cond, &condition_test_mutex);
    }
    ports0_printf("condition_test(): thread1: awoken\n");
    
    ports0_mutex_unlock(&condition_test_mutex);
    
    ports0_mutex_destroy(&condition_test_mutex);
    ports0_cond_destroy(&condition_test_cond);
    
    ports0_printf("condition_test(): complete\n");
} /* condition_test() */


