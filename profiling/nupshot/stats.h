/*

   Simple stats stuff

   Ed Karrels
   Argonne National Laboratory
*/

#ifndef STATS_H_
#define STATS_H_


typedef struct {
  double sum;
  double sum_sq;
  double max, min;
  int n;
} statData;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

  /* initialize stats structure */
statData *Stats_Create();

  /* add a data point */
int Stats_Add ARGS(( statData *stats, double x ));

  /* add an array of data points */
int Stats_AddArray ARGS(( statData *stats, double *nums, int n ));

  /* get the # of data points */
int Stats_N ARGS(( statData *stats ));

  /* get the sum of the data points */
double Stats_Sum ARGS(( statData *stats ));

  /* get the sum of the data points */
double Stats_Min ARGS(( statData *stats ));

  /* get the sum of the data points */
double Stats_Max ARGS(( statData *stats ));

  /* get the average of the data */
double Stats_Av ARGS(( statData *stats ));

  /* get the standard deviation of the data */
double Stats_StdDev ARGS(( statData *stats ));

  /* clear the stats data */
int Stats_Reset ARGS(( statData *stats ));

  /* close the stats data */
int Stats_Close ARGS(( statData *stats ));

#endif  /* #ifndef STATS_H_ */
