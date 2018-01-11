#ifndef LAUNCH_PROCESS_H
#define LAUNCH_PROCESS_H

HANDLE LaunchProcess(char *cmd, char *env, char *dir, HANDLE *hIn, HANDLE *hOut, HANDLE *hErr, int *pdwPid, int *nError, char *pszError);

#endif
