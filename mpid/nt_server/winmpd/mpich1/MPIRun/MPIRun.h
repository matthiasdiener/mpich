#ifndef MPIRUN_H
#define MPIRUN_H

#ifdef WSOCK2_BEFORE_WINDOWS
#include <winsock2.h>
#endif
#include <windows.h>
#include <tchar.h>

#include "global.h"

#define SERIALIZE_ROOT_PROCESS
//#undef SERIALIZE_ROOT_PROCESS

extern long g_nHosts;
extern char g_pszExe[MAX_CMD_LENGTH];
extern char g_pszArgs[MAX_CMD_LENGTH];
extern char g_pszEnv[MAX_CMD_LENGTH];
extern char g_pszDir[MAX_PATH];
extern bool g_bNoMPI;

BOOL WINAPI CtrlHandlerRoutine(DWORD dwCtrlType);
void PrintError(int error, char *msg, ...);
void WaitForExitCommands();

#endif
