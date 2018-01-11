#include "GetStringOpt.h"
#include "mpdimpl.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "Translate_Error.h"

long g_nNumProcsRunning = 0;

struct LaunchThreadStruct
{
    LaunchThreadStruct();
    void Print();

    char pszHost[MAX_HOST_LENGTH];
    char pszSrcHost[MAX_HOST_LENGTH];
    char pszSrcId[10];
    char pszEnv[4096];
    char pszDir[MAX_PATH];
    char pszCmd[MAX_CMD_LENGTH];
    char pszAccount[25];
    char pszPassword[25];
    char pszStdin[MAX_HOST_LENGTH];
    char pszStdout[MAX_HOST_LENGTH];
    char pszStderr[MAX_HOST_LENGTH];
    bool bMergeOutErr;

    int nPid;
    int nKRank;
    char pszError[MAX_CMD_LENGTH];
    int nExitCode;
    HANDLE hProcess;
    HANDLE hThread;

    LaunchThreadStruct *pNext;
};

LaunchThreadStruct::LaunchThreadStruct()
{
    pszHost[0] = '\0';
    pszSrcHost[0] = '\0';
    pszSrcId[0] = '\0';
    pszEnv[0] = '\0';
    pszDir[0] = '\0';
    pszCmd[0] = '\0';
    pszAccount[0] = '\0';
    pszPassword[0] = '\0';
    pszStdin[0] = '\0';
    pszStdout[0] = '\0';
    pszStderr[0] = '\0';
    bMergeOutErr = false;

    nPid = -1;
    nKRank = 0;
    pszError[0] = '\0';
    nExitCode = -1;
    hProcess = NULL;
    hThread = NULL;
    
    pNext = NULL;
}

void LaunchThreadStruct::Print()
{
    /*
    printf("pszHost: '%s'\n", pszHost);
    printf("pszSrcHost: '%s'\n", pszSrcHost);
    printf("pszSrcId: '%s'\n", pszSrcId);
    printf("pszEnv: '%s'\n", pszEnv);
    printf("pszDir: '%s'\n", pszDir);
    printf("pszCmd: '%s'\n", pszCmd);
    printf("pszAccount: '%s'\n", pszAccount);
    printf("pszPassword: ");
    if (strlen(pszPassword) || strlen(pszAccount))
	printf("***\n");
    else
	printf("\n");
    printf("pszStdin: '%s'\n", pszStdin);
    printf("pszStdout: '%s'\n", pszStdout);
    printf("pszStderr: '%s'\n", pszStderr);

    printf("nPid: %d\n", nPid);
    printf("nKRank: %d\n", nKRank);
    printf("pszError: '%s'\n", pszError);
    printf("nExitCode: %d\n", nExitCode);
    printf("hProcess: 0x%x\n", hProcess);
    printf("hThread: 0x%x\n", hThread);
    
    printf("pNext: 0x%x\n", pNext);
    */
    printf("LAUNCH:\n");
    printf(" %s(%s) -> %s %s\n", pszSrcHost, pszSrcId, pszHost, pszCmd);
    if (pszDir[0] != '\0')
    {
	printf(" dir: ");
	int n = strlen(pszDir);
	if (n > 70)
	{
	    char pszTemp[71];
	    char *pszCur = pszDir;
	    bool bFirst = true;
	    while (n > 0)
	    {
		strncpy(pszTemp, pszCur, 70);
		pszTemp[70] = '\0';
		if (bFirst)
		{
		    printf("%s\n", pszTemp);
		    bFirst = false;
		}
		else
		    printf("      %s\n", pszTemp);
		pszCur += 70;
		n -= 70;
	    }
	}
	else
	    printf("%s\n", pszDir);
    }
    if (pszEnv[0] != '\0')
    {
	char pszEnv2[4096];
	char pszCheck[100];
	char *token;
	int i,n;
	strcpy(pszEnv2, pszEnv);

	token = strstr(pszEnv2, "PMI_PWD=");
	if (token != NULL)
	{
	    strncpy(pszCheck, &token[8], 100);
	    pszCheck[99] = '\0';
	    token = strtok(pszCheck, " '|\n");
	    n = strlen(pszCheck);
	    token = strstr(pszEnv2, "PMI_PWD=");
	    token = &token[8];
	    if (n > 0)
	    {
		if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		    n--;
		if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		    n--;
		for (i=0; i<n; i++)
		    token[i] = '*';
	    }
	}

	printf(" env: ");
	n = strlen(pszEnv2);
	if (n > 70)
	{
	    char pszTemp[71];
	    char *pszCur = pszEnv2;
	    bool bFirst = true;
	    while (n > 0)
	    {
		strncpy(pszTemp, pszCur, 70);
		pszTemp[70] = '\0';
		if (bFirst)
		{
		    printf("%s\n", pszTemp);
		    bFirst = false;
		}
		else
		    printf("      %s\n", pszTemp);
		pszCur += 70;
		n -= 70;
	    }
	}
	else
	    printf("%s\n", pszEnv2);
    }
    printf(" stdin|out|err: %s|%s|%s\n", pszStdin, pszStdout, pszStderr);
    printf(" krank: %d\n", nKRank);
    //printf("\n");
    fflush(stdout);
}

LaunchThreadStruct *g_pProcessList = NULL;

void RemoveProcessStruct(LaunchThreadStruct *p)
{
    HANDLE hProcessStructMutex = CreateMutex(NULL, FALSE, "mpdProcessStructMutex");
    WaitForSingleObject(hProcessStructMutex, INFINITE);

    LaunchThreadStruct *pTrailer = g_pProcessList;

    // Remove p from the list
    if (p == NULL)
    {
	ReleaseMutex(hProcessStructMutex);
	CloseHandle(hProcessStructMutex);
	return;
    }

    if (p == g_pProcessList)
	g_pProcessList = g_pProcessList->pNext;
    else
    {
	while (pTrailer && pTrailer->pNext != p)
	    pTrailer = pTrailer->pNext;
	if (pTrailer)
	    pTrailer->pNext = p->pNext;
    }

    // Close any open handles
    if (p->hProcess != NULL)
	CloseHandle(p->hProcess);
    if (p->hThread != NULL)
	CloseHandle(p->hThread);

    //dbg_printf("removing ProcessStruct[%d]\n", p->nPid);
    // free the structure
    delete p;

    ReleaseMutex(hProcessStructMutex);
    CloseHandle(hProcessStructMutex);
}

bool ConnectAndRedirectInput(HANDLE hIn, char *pszHostPort, HANDLE hProcess, DWORD dwPid, int nRank)
{
    DWORD dwThreadID;
    RedirectSocketArg *pArg;
    int bfd;
    char pszHost[MAX_HOST_LENGTH];
    int nPort;
    char *pszPort;
    int nLength;
    char ch = 0;

    if ((pszHostPort == NULL) || (*pszHostPort == '\0'))
    {
	if (hIn != NULL)
	    CloseHandle(hIn);
	return true;
    }

    pszPort = strstr(pszHostPort, ":");
    if (pszPort == NULL)
    {
	if (hIn != NULL)
	    CloseHandle(hIn);
	return false;
    }
    nLength = pszPort - pszHostPort;
    strncpy(pszHost, pszHostPort, nLength);
    pszHost[nLength] = '\0';
    pszPort++;

    nPort = atoi(pszPort);
    
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectInput: beasy_create failed, error %d\n", WSAGetLastError());
	return false;
    }
    if (beasy_connect(bfd, pszHost, nPort) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectInput: beasy_connect(%s:%d) failed, error %d\n", pszHost, nPort, WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Send header indicating stdin redirection
    if (beasy_send(bfd, &ch, sizeof(char)) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectInput: beasy_send(%d) failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Start thread to transfer data
    pArg = new RedirectSocketArg;
    pArg->hRead = NULL;
    pArg->bfdRead = bfd;
    pArg->hWrite = hIn;
    pArg->bfdWrite = BFD_INVALID_SOCKET;
    pArg->bReadisPipe = false;
    pArg->bWriteisPipe = true;
    pArg->hProcess = hProcess;
    pArg->dwPid = dwPid;
    pArg->nRank = nRank;
    pArg->cType = 0;
    HANDLE hThread;
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectSocketThread, pArg, 0, &dwThreadID);
    if (hThread == NULL)
    {
	err_printf("ConnectAndRedirectInput: CreateThread failed, error %d\n", GetLastError());
	CloseHandle(hIn);
	beasy_closesocket(bfd);
	return false;
    }
    else
	CloseHandle(hThread);
    return true;
}

bool ConnectAndRedirectOutput(HANDLE hOut, char *pszHostPort, HANDLE hProcess, DWORD dwPid, int nRank, char cType)
{
    DWORD dwThreadID;
    RedirectSocketArg *pArg;
    int bfd;
    char pszHost[MAX_HOST_LENGTH];
    int nPort;
    char *pszPort;
    int nLength;

    if ((pszHostPort == NULL) || (*pszHostPort == '\0'))
    {
	if (hOut != NULL)
	    CloseHandle(hOut);
	return true;
    }

    pszPort = strstr(pszHostPort, ":");
    if (pszPort == NULL)
    {
	if (hOut != NULL)
	    CloseHandle(hOut);
	return false;
    }
    nLength = pszPort - pszHostPort;
    strncpy(pszHost, pszHostPort, nLength);
    pszHost[nLength] = '\0';
    pszPort++;

    nPort = atoi(pszPort);
    
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectOutput: beasy_create failed, error %d\n", WSAGetLastError());
	return false;
    }
    if (beasy_connect(bfd, pszHost, nPort) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectOutput: beasy_connect(%s:%d) failed, error %d\n", pszHost, nPort, WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Send header indicating stdout/err redirection
    if (beasy_send(bfd, &cType, sizeof(char)) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirectOutput: beasy_send(%d) failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Start thread to transfer data
    pArg = new RedirectSocketArg;
    pArg->hWrite = NULL;
    pArg->bfdWrite = bfd;
    pArg->hRead = hOut;
    pArg->bfdRead = BFD_INVALID_SOCKET;
    pArg->bReadisPipe = true;
    pArg->bWriteisPipe = false;
    pArg->hProcess = hProcess;
    pArg->dwPid = dwPid;
    pArg->nRank = nRank;
    pArg->cType = cType;
    HANDLE hThread;
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectSocketThread, pArg, 0, &dwThreadID);
    if (hThread == NULL)
    {
	err_printf("ConnectAndRedirectOutput: CreateThread failed, error %d\n", GetLastError());
	CloseHandle(hOut);
	beasy_closesocket(bfd);
	return false;
    }
    else
	CloseHandle(hThread);
    return true;
}

bool ConnectAndRedirect2Outputs(HANDLE hOut, HANDLE hErr, char *pszHostPort, HANDLE hProcess, DWORD dwPid, int nRank)
{
    DWORD dwThreadID;
    RedirectSocketArg *pArg;
    int bfd;
    char pszHost[MAX_HOST_LENGTH];
    int nPort;
    char *pszPort;
    int nLength;
    char cType = 1;

    if ((pszHostPort == NULL) || (*pszHostPort == '\0'))
    {
	if (hOut != NULL)
	    CloseHandle(hOut);
	return true;
    }

    pszPort = strstr(pszHostPort, ":");
    if (pszPort == NULL)
    {
	if (hOut != NULL)
	    CloseHandle(hOut);
	return false;
    }
    nLength = pszPort - pszHostPort;
    strncpy(pszHost, pszHostPort, nLength);
    pszHost[nLength] = '\0';
    pszPort++;

    nPort = atoi(pszPort);
    
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirect2Outputs: beasy_create failed, error %d\n", WSAGetLastError());
	return false;
    }
    if (beasy_connect(bfd, pszHost, nPort) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirect2Outputs: beasy_connect(%s:%d) failed, error %d\n", pszHost, nPort, WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Send header indicating stdout/err redirection
    if (beasy_send(bfd, &cType, sizeof(char)) == SOCKET_ERROR)
    {
	err_printf("ConnectAndRedirect2Outputs: beasy_send(%d) failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    HANDLE hThread;

    // Start thread to transfer data from hOut
    pArg = new RedirectSocketArg;
    pArg->hWrite = NULL;
    pArg->bfdWrite = bfd;
    pArg->hRead = hOut;
    pArg->bfdRead = BFD_INVALID_SOCKET;
    pArg->bReadisPipe = true;
    pArg->bWriteisPipe = false;
    pArg->hProcess = hProcess;
    pArg->dwPid = dwPid;
    pArg->hMutex = hMutex;
    pArg->bFreeMutex = false;
    pArg->nRank = nRank;
    pArg->cType = 1;
    pArg->hOtherThread = NULL;
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectLockedSocketThread, pArg, 0, &dwThreadID);
    if (hThread == NULL)
    {
	err_printf("ConnectAndRedirect2Outputs: CreateThread failed, error %d\n", GetLastError());
	CloseHandle(hOut);
	beasy_closesocket(bfd);
	CloseHandle(hErr);
	return false;
    }

    // Start thread to transfer data from hErr
    pArg = new RedirectSocketArg;
    pArg->nRank = nRank;
    pArg->hWrite = NULL;
    pArg->bfdWrite = bfd;
    pArg->hRead = hErr;
    pArg->bfdRead = BFD_INVALID_SOCKET;
    pArg->bReadisPipe = true;
    pArg->bWriteisPipe = false;
    pArg->hProcess = NULL;
    pArg->dwPid = -1;
    pArg->hMutex = hMutex;
    pArg->bFreeMutex = true;
    pArg->nRank = nRank;
    pArg->cType = 2;
    pArg->hOtherThread = hThread;
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectLockedSocketThread, pArg, 0, &dwThreadID);
    if (hThread == NULL)
    {
	err_printf("ConnectAndRedirect2Outputs: CreateThread failed, error %d\n", GetLastError());
	CloseHandle(hErr);
	beasy_closesocket(bfd);
	return false;
    }
    else
	CloseHandle(hThread);
    return true;
}

void LaunchThread(LaunchThreadStruct *pArg)
{
    DWORD dwExitCode;
    char pszStr[MAX_CMD_LENGTH];
    char pszError[MAX_PATH];

    pArg->Print();

    HANDLE hIn, hOut, hErr;
    int nError;
    pszStr[0] = '\0';
    if (g_bSingleUser)
    {
	pArg->hProcess = LaunchProcess(pArg->pszCmd, pArg->pszEnv, pArg->pszDir, &hIn, &hOut, &hErr, &pArg->nPid, &nError, pszStr);
    }
    else
    {
	pArg->hProcess = LaunchProcessLogon(pArg->pszAccount, pArg->pszPassword, 
					pArg->pszCmd, pArg->pszEnv, pArg->pszDir, &hIn, &hOut, &hErr, &pArg->nPid, &nError, pszStr);
    }
    if (pArg->hProcess == INVALID_HANDLE_VALUE)
    {
	Translate_Error(nError, pszError, pszStr);
	sprintf(pszStr, "launched src=%s dest=%s id=%s error=LaunchProcess failed, %s", g_pszHost, pArg->pszSrcHost, pArg->pszSrcId, pszError);
	EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_RESULT);
	ResetSelect();
	RemoveProcessStruct(pArg);
	InterlockedDecrement(&g_nNumProcsRunning);
	return;
    }

    sprintf(pszStr, "launched pid=%d src=%s dest=%s id=%s", pArg->nPid, g_pszHost, pArg->pszSrcHost, pArg->pszSrcId);
    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_RESULT);
    ResetSelect();

    if (!ConnectAndRedirectInput(hIn, pArg->pszStdin, pArg->hProcess, pArg->nPid, pArg->nKRank))
    {
	TerminateProcess(pArg->hProcess, 1);
    }
    else 
    {
	if (pArg->bMergeOutErr)
	{
	    if (!ConnectAndRedirect2Outputs(hOut, hErr, pArg->pszStdout, pArg->hProcess, pArg->nPid, pArg->nKRank))
	    {
		TerminateProcess(pArg->hProcess, 1);
	    }
	}
	else
	{
	    if (!ConnectAndRedirectOutput(hOut, pArg->pszStdout, pArg->hProcess, pArg->nPid, pArg->nKRank, 1))
	    {
		TerminateProcess(pArg->hProcess, 1);
	    }
	    else if (!ConnectAndRedirectOutput(hErr, pArg->pszStderr, pArg->hProcess, pArg->nPid, pArg->nKRank, 2))
	    {
		TerminateProcess(pArg->hProcess, 1);
	    }
	}
    }

    WaitForSingleObject(pArg->hProcess, INFINITE);
    GetExitCodeProcess(pArg->hProcess, &dwExitCode);
    pArg->nExitCode = dwExitCode;
    CloseHandle(pArg->hProcess);
    pArg->hProcess = NULL;

    sprintf(pszStr, "exitcode code=%d src=%s dest=%s id=%s", pArg->nExitCode, g_pszHost, pArg->pszSrcHost, pArg->pszSrcId);
    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_EXITCODE);
    ResetSelect();

    RemoveProcessStruct(pArg);
    InterlockedDecrement(&g_nNumProcsRunning);
}

void ShutdownAllProcesses()
{
    DWORD dwExitCode;
    HANDLE hProcessStructMutex = CreateMutex(NULL, FALSE, "mpdProcessStructMutex");
    WaitForSingleObject(hProcessStructMutex, INFINITE);
    LaunchThreadStruct *p = g_pProcessList;
    while (p)
    {
	if (p->hProcess)
	{
	    if (GetExitCodeProcess(p->hProcess, &dwExitCode))
	    {
		if (dwExitCode == STILL_ACTIVE)
		{
		    if (!TerminateProcess(p->hProcess, 0))
		    {
			err_printf("TerminateProcess failed for process %d, %d, error %d\n", p->hProcess, p->nPid, GetLastError());
			// If I can't stop a process for some reason,
			// decrement its value so this function doesn't hang
			InterlockedDecrement(&g_nNumProcsRunning);
		    }
		}
	    }
	}
	p = p->pNext;
    }
    ReleaseMutex(hProcessStructMutex);
    CloseHandle(hProcessStructMutex);

    // Wait for all the threads to clean up the terminated processes
    while (g_nNumProcsRunning > 0)
	Sleep(250);
}

void MPD_KillProcess(int nPid)
{
    DWORD dwExitCode;
    HANDLE hProcessStructMutex = CreateMutex(NULL, FALSE, "mpdProcessStructMutex");
    WaitForSingleObject(hProcessStructMutex, INFINITE);
    LaunchThreadStruct *p = g_pProcessList;
    while (p)
    {
	if (p->nPid == nPid)
	{
	    //dbg_printf("MPD_KillProcess found pid %d\n", nPid);
	    if (p->hProcess && (p->hProcess != INVALID_HANDLE_VALUE))
	    {
		//dbg_printf("MPD_KillProcess found valid hProcess 0x%x\n", p->hProcess);
		if (GetExitCodeProcess(p->hProcess, &dwExitCode))
		{
		    if (dwExitCode == STILL_ACTIVE)
		    {
			//dbg_printf("MPD_KillProcess - terminating process\n");
			if (!TerminateProcess(p->hProcess, 0))
			{
			    err_printf("TerminateProcess failed for process %d, %d, error %d\n", p->hProcess, p->nPid, GetLastError());
			    // If I can't stop a process for some reason,
			    // decrement its value so this function doesn't hang
			    InterlockedDecrement(&g_nNumProcsRunning);
			    // Should I also remove the LaunchThreadStruct p?
			    // If there are lots of failed process terminations, this will lead to wasted memory allocation.
			}
		    }
		}
		else
		{
		    err_printf("MPD_KillProcess - GetExitCodeProcess failed, error %d\n", GetLastError());
		}
	    }
	    ReleaseMutex(hProcessStructMutex);
	    CloseHandle(hProcessStructMutex);
	    return;
	}
	p = p->pNext;
    }
    ReleaseMutex(hProcessStructMutex);
    CloseHandle(hProcessStructMutex);
}

void Launch(char *pszStr)
{
    char sTemp[10];
    LaunchThreadStruct *pArg = new LaunchThreadStruct;

    if (GetStringOpt(pszStr, "k", sTemp))
	pArg->nKRank = atoi(sTemp);
    else
	pArg->nKRank = 0;
    if (!GetStringOpt(pszStr, "h", pArg->pszHost))
	strcpy(pArg->pszHost, g_pszHost);
    GetStringOpt(pszStr, "src", pArg->pszSrcHost);
    GetStringOpt(pszStr, "id", pArg->pszSrcId);
    GetStringOpt(pszStr, "e", pArg->pszEnv);
    GetStringOpt(pszStr, "d", pArg->pszDir);
    GetStringOpt(pszStr, "c", pArg->pszCmd);
    GetStringOpt(pszStr, "a", pArg->pszAccount);
    GetStringOpt(pszStr, "p", pArg->pszPassword);
    GetStringOpt(pszStr, "0", pArg->pszStdin);
    GetStringOpt(pszStr, "1", pArg->pszStdout);
    GetStringOpt(pszStr, "2", pArg->pszStderr);
    if (GetStringOpt(pszStr, "12", pszStr))
    {
	strcpy(pArg->pszStdout, pszStr);
	strcpy(pArg->pszStderr, pszStr);
	pArg->bMergeOutErr = true;
    }
    if (GetStringOpt(pszStr, "012", pszStr))
    {
	strcpy(pArg->pszStdin, pszStr);
	strcpy(pArg->pszStdout, pszStr);
	strcpy(pArg->pszStderr, pszStr);
	pArg->bMergeOutErr = true;
    }

    HANDLE hProcessStructMutex = CreateMutex(NULL, FALSE, "mpdProcessStructMutex");
    WaitForSingleObject(hProcessStructMutex, INFINITE);
    if (!g_pProcessList)
    {
	g_pProcessList = pArg;
    }
    else
    {
	pArg->pNext = g_pProcessList;
	g_pProcessList = pArg;
    }
    ReleaseMutex(hProcessStructMutex);
    CloseHandle(hProcessStructMutex);

    InterlockedIncrement(&g_nNumProcsRunning);

    DWORD dwThreadID;
    pArg->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LaunchThread, pArg, 0, &dwThreadID);
    if (pArg->hThread == NULL)
    {
	err_printf("Launch: CreateThread failed, error %d\n", GetLastError());
	InterlockedDecrement(&g_nNumProcsRunning);
    }

    return;
}

void ConcatenateProcessesToString(char *pszStr)
{
    char pszLine[4096];
    HANDLE hProcessStructMutex = CreateMutex(NULL, FALSE, "mpdProcessStructMutex");
    WaitForSingleObject(hProcessStructMutex, INFINITE);
    LaunchThreadStruct *p = g_pProcessList;
    if (p)
    {
	sprintf(pszLine, "%s:\n", g_pszHost);
	strcat(pszStr, pszLine);
    }
    while (p)
    {
	sprintf(pszLine, "%04d : %s\n", p->nPid, p->pszCmd);
	strcat(pszStr, pszLine);
	p = p->pNext;
    }
    ReleaseMutex(hProcessStructMutex);
    CloseHandle(hProcessStructMutex);
}
