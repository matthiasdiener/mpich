#ifndef LAUNCH_PROCESS_H
#define LAUNCH_PROCESS_H

#ifdef WSOCK2_BEFORE_WINDOWS
#include <winsock2.h>
#endif
#include <windows.h>

struct MPIRunLaunchProcessArg
{
    int i;
    char pszJobID[100];
    char pszHost[100];
    char pszEnv[MAX_PATH];
    char pszDir[MAX_PATH];
    char pszCmdLine[MAX_PATH];
    bool bLogon;
    char pszAccount[100];
    char pszPassword[100];
    char pszIOHostPort[100];
    char pszPassPhrase[257];
    //bool bMapDrive;
    //char cMapDrive;
    //char pszMapShare[MAX_PATH];
    //char pszMapAccount[40];
    //char pszMapPassword[40];
};

void MPIRunLaunchProcess(MPIRunLaunchProcessArg *arg);

#endif
