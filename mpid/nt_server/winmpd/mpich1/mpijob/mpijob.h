#ifndef MPIJOB_H
#define MPIJOB_H

#include "mpdutil.h"
#include "mpd.h"
#include "bsocket.h"

// main functions
void ListJobs(char *host, int port, char *altphrase);
void DisplayJob(char *job, char *host, int port, char *altphrase, bool bFullOutput, bool bToFile, char *filename);
void KillJob(char *job, char *host, int port, char *altphrase);
void ClearJobs(char *option, char *host, int port, char *altphrase);
void JobsToFile(char *filename, char *option, char *host, int port, char *altphrase);

// helper functions
void GetKeyAndValue(char *str, char *key, char *value);
bool GetRankAndOption(char *str, int &rank, char *option);
bool ParseTimeStamp(char *str, int &year, int &month, int &day, int &hour, int &minute, int &second);
bool CompareTimeStamps(char *t1, char *t2, int &relation);

#endif
