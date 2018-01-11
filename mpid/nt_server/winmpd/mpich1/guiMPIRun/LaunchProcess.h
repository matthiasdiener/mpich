#ifndef LAUNCH_PROCESS_H
#define LAUNCH_PROCESS_H

#include <winsock2.h>
#include <windows.h>

class CGuiMPIRunView;

struct MPIRunLaunchProcessArg
{
    int i;
    char pszJobID[100];
    char pszHost[MAX_HOST_LENGTH];
    char pszEnv[MAX_CMD_LENGTH];
    char pszDir[MAX_PATH];
    char pszCmdLine[MAX_CMD_LENGTH];
    bool bLogon;
    char pszAccount[100];
    char pszPassword[300];
    char pszIOHostPort[100];
    char pszPassPhrase[257];
    CGuiMPIRunView *pDlg;
};

void MPIRunLaunchProcess(MPIRunLaunchProcessArg *arg);

#endif
