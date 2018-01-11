#include <sys/types.h>
#include <elan/elan.h>

/* This function provides an accurate elasped time clock for the
   Meiko CS2.  It is used for the NX version of the codes */

double dclock()
{
    ELAN_TIMEVAL clock;
    int          secs;
    int          nsecs;
    static int   init_nsec_clock;
    static void  *context;
    
    if (init_nsec_clock == 0)
    {
	context         = elan_init();
	init_nsec_clock = 1;
    }
    
    elan_clock(context, (ELAN_TIMEVAL *)&clock);
    
    secs  = clock.tv_sec;
    nsecs = clock.tv_nsec;
    
    return((double)secs + (double)nsecs * (double)0.000000001);
    
}
