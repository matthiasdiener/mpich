#ifndef LAUNCH_PROCESS_H
#define LAUNCH_PROCESS_H

#ifdef WSOCK2_BEFORE_WINDOWS
#include <winsock2.h>
#endif
#include <windows.h>

#include "global.h"

struct MPIRunLaunchProcessArg
{
    int i, n;
    char pszJobID[100];
    char pszHost[100];
    char pszEnv[MAX_CMD_LENGTH];
    char pszDir[MAX_PATH];
    char pszCmdLine[MAX_CMD_LENGTH];
    bool bLogon;
    char pszAccount[100];
    char pszPassword[300];
    char pszIOHostPort[100];
    char pszPassPhrase[257];
    //bool bMapDrive;
    //char cMapDrive;
    //char pszMapShare[MAX_PATH];
    //char pszMapAccount[40];
    //char pszMapPassword[40];
};

void MPIRunLaunchProcess(MPIRunLaunchProcessArg *arg);
void PutJobInDatabase(MPIRunLaunchProcessArg *arg);
void PutJobProcessInDatabase(MPIRunLaunchProcessArg *arg, int pid);
void UpdateJobState(char *state);

#endif
