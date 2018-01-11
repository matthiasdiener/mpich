
#ifndef MPID_CH_Wtime
#if defined(HAVE_GETTIMEOFDAY) || defined(HAVE_WIERDGETTIMEOFDAY) || \
    defined(HAVE_BSDGETTIMEOFDAY)
#include <sys/types.h>
#include <sys/time.h>
#endif
void MPID_CH_Wtime( seconds )
double *seconds;
{
#if defined(HAVE_BSDGETTIMEOFDAY)
    struct timeval tp;
    struct timezone tzp;

    BSDgettimeofday(&tp,&tzp);
    *seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(USE_WIERDGETTIMEOFDAY)
    /* This is for Solaris, where they decided to change the CALLING
       SEQUENCE OF gettimeofday! (Solaris 2.3 and 2.4 only?) */
    struct timeval tp;

    gettimeofday(&tp);
    *seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(HAVE_GETTIMEOFDAY)
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    *seconds = ((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#else
    *seconds = 0;
#endif
}
#endif

