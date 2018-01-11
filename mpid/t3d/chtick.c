#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


extern double MPID_CH_Wtime();

double MPID_CH_Wtick()
{
  static double tickval = -1.0;
  double t1,t2;
  int cnt;
  int icnt;

  if (tickval < 0.0)
  {
    tickval = 1.0e6;
    for (icnt=0; icnt < 10; icnt++)
    {
      cnt = 1000;
      t1 = MPID_CH_Wtime();
      while ((cnt--) && ((t2 = MPID_CH_Wtime()) <= t1));
      if ((cnt) && ((t2-t1) < tickval))
	tickval = t2 - t1;
    }
  }
  return tickval;
}
