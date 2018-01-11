#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

extern double t3d_reference_time;

double MPID_CH_Wtime()
{
  double current_time;
  double wtime;

  current_time = (double)rtclock();
  wtime = (current_time - t3d_reference_time) * 6.6e-3 * 0.000001;
  return wtime;
}
