#include "clog_time.h"

static double starttime;

void CLOG_timeinit()
{
    int flag;

    starttime = PMPI_Wtime();
    PMPI_Initialized(&flag);
    if (!flag)
        PMPI_Init(0,0);
}

double CLOG_timestamp()
{
    return ( PMPI_Wtime() - starttime );
}



