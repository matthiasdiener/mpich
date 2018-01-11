#ifndef LAUNCH_PROCESS_H
#define LAUNCH_PROCESS_H

#include <winsock2.h>
#include <windows.h>

class CGuiMPIRunView;

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
    CGuiMPIRunView *pDlg;
};

void MPIRunLaunchProcess(MPIRunLaunchProcessArg *arg);

#endif
