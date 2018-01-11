#include "mpdimpl.h"
#include "bsocket.h"
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "Service.h"
#include "GetStringOpt.h"
#include "database.h"
#include "Translate_Error.h"
#include <time.h>

struct TmpFileStruct
{
    char pszFileName[MAX_PATH];
    TmpFileStruct *pNext;
};

TmpFileStruct *g_pTmpFileList = NULL;

static void CreateTmpFile(char *pszFileName, bool bDelete = true)
{
    char pszDir[MAX_PATH] = "C:\\";
    char pszTemp[MAX_PATH];
    char *namepart;
    
    if (!ReadMPDRegistry("temp", pszDir))
	dbg_printf("no temp directory specified, using c:\\\n");
    
    if (GetTempFileName(pszDir, "mpi", 0, pszTemp) == 0)
    //if (GetTempFileName(g_pszTempDir, "mpi", 0, pszTemp) == 0)
    {
	int nError = GetLastError();
	Translate_Error(nError, pszFileName, "FAIL ");
	err_printf("GetTempFileName(%s) failed, %s", pszDir, pszFileName);
	return;
    }
    
    GetFullPathName(pszTemp, MAX_PATH, pszFileName, &namepart);

    if (bDelete)
    {
	// Add this name to the global list
	// These names will be matched with corresponding "deletetmpfile" commands
	// All remaining files will be deleted when the mpd exits
	TmpFileStruct *pNode = new TmpFileStruct;
	strcpy(pNode->pszFileName, pszFileName);
	pNode->pNext = g_pTmpFileList;
	g_pTmpFileList = pNode;
    }
}

static bool DeleteTmpFile(char *pszFileName)
{
    TmpFileStruct *pNode, *pTrailer;
    pTrailer = pNode = g_pTmpFileList;
    while (pNode)
    {
	if (stricmp(pszFileName, pNode->pszFileName) == 0)
	{
	    if (pNode == g_pTmpFileList)
	    {
		g_pTmpFileList = g_pTmpFileList->pNext;
		delete pNode;
	    }
	    else
	    {
		pTrailer->pNext = pNode->pNext;
		delete pNode;
	    }
	    return (DeleteFile(pszFileName) == TRUE);
	}
	if (pTrailer != pNode)
	    pTrailer = pTrailer->pNext;
	pNode = pNode->pNext;
    }
    return false;
}

void RemoveAllTmpFiles()
{
    TmpFileStruct *pNode;
    while (g_pTmpFileList)
    {
	pNode = g_pTmpFileList;
	g_pTmpFileList = g_pTmpFileList->pNext;
	if (strlen(pNode->pszFileName) > 0)
	    DeleteFile(pNode->pszFileName);
	delete pNode;
    }
}

#define DEFAULT_MPICH_ROOT_TIMEOUT 7

static int GetPortFromFile(char *pszFileName, int nPid, int *nPort)
{
    int nError;
    DWORD num_read = 0;
    char pBuffer[100] = "";
    char *pChar = pBuffer;
    clock_t cStart;
    HANDLE hProcess = NULL;
    DWORD dwExitCode;
    int nTimeout = DEFAULT_MPICH_ROOT_TIMEOUT;

    if (ReadMPDRegistry("timeout", pBuffer))
    {
	nTimeout = atoi(pBuffer);
	if (nTimeout > 1000)
	    nTimeout = nTimeout / 1000;
	if (nTimeout < 1)
	    nTimeout = 1;
	pBuffer[0] = '\0';
    }

    HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
	nError = GetLastError();
	return nError;
    }
    
    cStart = clock();
    while (true)
    {
	num_read = 0;
	if (!ReadFile(hFile, pChar, 100, &num_read, NULL))
	{
	    nError = GetLastError();
	    CloseHandle(hFile);
	    DeleteTmpFile(pszFileName);
	    if (hProcess)
		CloseHandle(hProcess);
	    return nError;
	}
	if (num_read == 0)
	{
	    if (!hProcess)
	    {
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, nPid);
		if (hProcess == NULL)
		{
		    int error = GetLastError();
		    if (error == ERROR_INVALID_PARAMETER)
		    {
			int nExitCode = ConsoleGetExitCode(nPid);
			if (nExitCode != -1 && nExitCode != -2)
			{
			    if (nExitCode == ERROR_WAIT_NO_CHILDREN)
				return -2;
			    *nPort = nExitCode;
			    return -3;
			}
		    }
		}
	    }
	    if (hProcess)
	    {
		if (GetExitCodeProcess(hProcess, &dwExitCode))
		{
		    if (dwExitCode != STILL_ACTIVE)
		    {
			CloseHandle(hProcess);
			if (dwExitCode == ERROR_WAIT_NO_CHILDREN)
			    return -2;
			*nPort = dwExitCode;
			return -3;
		    }
		}
	    }
	    if (clock() - cStart > nTimeout * CLOCKS_PER_SEC)
	    {
		CloseHandle(hFile);
		DeleteTmpFile(pszFileName);
		if (hProcess)
		    CloseHandle(hProcess);
		return -1;
	    }
	    Sleep(100);
	}
	else
	{
	    for (unsigned int i=0; i<num_read; i++)
	    {
		if (*pChar == '\n')
		    break;
		pChar ++;
	    }
	    if (*pChar == '\n')
		break;
	}
    }
    CloseHandle(hFile);
    DeleteTmpFile(pszFileName);
    
    *nPort = atoi(pBuffer);
    
    if (hProcess)
	CloseHandle(hProcess);
    return 0;
}

static void HandleDBCommandRead(MPD_Context *p)
{
    char name[MAX_DBS_NAME_LEN+1];
    char key[MAX_DBS_KEY_LEN+1];
    char value[MAX_DBS_VALUE_LEN+1];
    char pszStr[MAX_CMD_LENGTH];

    char pszSrc[100] = "";
    char pszBfd[10] = "";
    char *pszCmdData;

    PUSH_FUNC("HandleDBCommandRead");

    GetStringOpt(p->pszIn, "src", pszSrc);
    GetStringOpt(p->pszIn, "bfd", pszBfd);
    pszCmdData = strstr(p->pszIn, "bfd=");
    while (*pszCmdData != ' ')
	pszCmdData++;
    pszCmdData++;

    int bfd = atoi(pszBfd);
    sprintf(pszBfd, "%d", bfd);

    //dbg_printf("left - HandleDBCommand: src='%s' bfd='%s' data='%s'\n", pszSrc, pszBfd, pszCmdData);

    if ((stricmp(pszSrc, g_pszHost) == 0) || (strcmp(pszSrc, g_pszIP) == 0))
    {
	// Stop result-less commands
	if ((strnicmp(p->pszIn, "dbcreate ", 9) == 0) ||
	    (strnicmp(p->pszIn, "dbdestroy ", 10) == 0) ||
	    (strnicmp(p->pszIn, "dbfirst ", 8) == 0))
	{
	    POP_FUNC();
	    return;
	}

	// Handle the full ring commands
	if (strnicmp(p->pszIn, "dbnext ", 7) == 0)
	{
	    EnqueueWrite(GetContext(atoi(pszBfd)), DBS_END_STR, MPD_WRITING_RESULT);
	    POP_FUNC();
	    return;
	}

	// The command has gone full circle without succeeding
	if (p = GetContext(atoi(pszBfd)))
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	else
	{
	    err_printf("GetContext failed for '%s'\n", pszBfd);
	}
	POP_FUNC();
	return;
    }

    if (strnicmp(p->pszIn, "dbresult ", 9) == 0)
    {
	char pszDest[MAX_HOST_LENGTH] = "";
	GetStringOpt(p->pszIn, "dest", pszDest);
	if ((stricmp(pszDest, g_pszHost) == 0) || (strcmp(pszDest, g_pszIP) == 0))
	{
	    char *token;
	    token = strstr(p->pszIn, "result=");
	    if (token != NULL)
	    {
		token = &token[7];
		p = GetContext(atoi(pszBfd));
		if (p)
		    EnqueueWrite(p, token, MPD_WRITING_RESULT);
		else
		{
		    err_printf("GetContext failed for '%s'\n", pszBfd);
		}
	    }
	    else
	    {
		err_printf("'result=' not found in dbresult command\n");
	    }
	}
	else
	{
	    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	}
    }
    else if (strnicmp(p->pszIn, "dbget ", 6) == 0)
    {
	GetNameKeyValue(pszCmdData, name, key, NULL);
	if (dbs_get(name, key, value) == DBS_SUCCESS)
	{
	    sprintf(pszStr, "dbresult dest=%s bfd=%s result=%s", pszSrc, pszBfd, value);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else
	{
	    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	}
    }
    else if (strnicmp(p->pszIn, "dbcreate ", 9) == 0)
    {
	if (GetStringOpt(p->pszIn, "name", name))
	{
	    dbs_create_name_in(name);
	    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	}
	else
	{
	    err_printf("This cannot happen because it should have been caught at the source host\n");
	}
    }
    else if (strnicmp(p->pszIn, "dbdestroy ", 10) == 0)
    {
	GetNameKeyValue(pszCmdData, name, NULL, NULL);
	dbs_destroy(name);
	EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
    }
    else if (strnicmp(p->pszIn, "dbfirst ", 8) == 0)
    {
	GetNameKeyValue(pszCmdData, name, NULL, NULL);
	dbs_first(name, NULL, NULL);
	EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
    }
    else if (strnicmp(p->pszIn, "dbnext ", 7) == 0)
    {
	GetNameKeyValue(pszCmdData, name, NULL, NULL);
	if (dbs_next(name, key, value) == DBS_SUCCESS)
	{
	    if (*key == '\0')
		EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	    else
	    {
		sprintf(pszStr, "dbresult dest=%s bfd=%s result=key=%s value=%s", pszSrc, pszBfd, key, value);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else
	{
	    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	}
    }
    else if (strnicmp(p->pszIn, "dbdelete ", 9) == 0)
    {
	GetNameKeyValue(pszCmdData, name, key, NULL);
	if (dbs_delete(name, key) == DBS_SUCCESS)
	{
	    sprintf(pszStr, "dbresult dest=%s bfd=%s result=DBS_SUCCESS", pszSrc, pszBfd);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else
	{
	    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	}
    }
    else
    {
	err_printf("unknown command '%s'", p->pszIn);
    }
    POP_FUNC();
}

bool GetIPString(char *pszHost, char *pszIPStr)
{
    unsigned int a, b, c, d;
    struct hostent *pH;
    
    pH = gethostbyname(pszHost);
    if (pH == NULL)
	return false;

    a = (unsigned char)(pH->h_addr_list[0][0]);
    b = (unsigned char)(pH->h_addr_list[0][1]);
    c = (unsigned char)(pH->h_addr_list[0][2]);
    d = (unsigned char)(pH->h_addr_list[0][3]);

    sprintf(pszIPStr, "%u.%u.%u.%u", a, b, c, d);

    return true;
}

void HandleLeftRead(MPD_Context *p)
{
    MPD_Context *pContext;
    char pszStr[MAX_CMD_LENGTH];
    char pszHost[MAX_HOST_LENGTH] = "";

    PUSH_FUNC("HandleLeftRead");

    dbg_printf("LeftRead[%d]: '%s'\n", p->bfd, p->pszIn);
    switch (p->nLLState)
    {
    case MPD_READING_CMD:
	if (strnicmp(p->pszIn, "db", 2) == 0)
	{
	    HandleDBCommandRead(p);
	}
	else if (strnicmp(p->pszIn, "launch ", 7) == 0)
	{
	    pszHost[0] = '\0';
	    GetStringOpt(p->pszIn, "h", pszHost);
	    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
	    {
		Launch(p->pszIn);
	    }
	    else
	    {
		bool bNoHost = pszHost[0] == '\0';
		GetStringOpt(p->pszIn, "src", pszHost);
		if (strcmp(pszHost, g_pszHost) == 0)
		{
		    if (bNoHost)
		    {
			// This needs to be handled by HandleConsoleRead
			Launch(p->pszIn);
		    }
		    else
		    {
			if (GetStringOpt(p->pszIn, "try", pszStr))
			{
			    //dbg_printf("launch command went full circle without a match, discarding\n");
			    char pszId[10];
			    GetStringOpt(p->pszIn, "src", pszHost);
			    GetStringOpt(p->pszIn, "id", pszId);
			    sprintf(pszStr, "launched src=%s dest=%s id=%s error=invalid host", g_pszHost, pszHost, pszId);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_RESULT);
			}
			else
			{
			    pszHost[0] = '\0';
			    GetStringOpt(p->pszIn, "h", pszHost);

			    if (GetIPString(pszHost, pszHost))
			    {
				sprintf(pszStr, "launch h=%s try=2 %s", pszHost, &p->pszIn[7]);
				dbg_printf("trying launch again with ip string replacing the old hostname\n");
				EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_CMD);
			    }
			    else
			    {
				char pszId[10];
				GetStringOpt(p->pszIn, "src", pszHost);
				GetStringOpt(p->pszIn, "id", pszId);
				sprintf(pszStr, "launched src=%s dest=%s id=%s error=invalid host", g_pszHost, pszHost, pszId);
				EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_RESULT);
			    }
			}
		    }
		}
		else
		{
		    dbg_printf("forwarding launch command\n");
		    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_LAUNCH_CMD);
		}
	    }
	}
	else if (strnicmp(p->pszIn, "launched ", 9) == 0)
	{
	    GetStringOpt(p->pszIn, "dest", pszHost);
	    if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
	    {
		char pszId[10];
		char pszPid[10];
		GetStringOpt(p->pszIn, "id", pszId);
		if (GetStringOpt(p->pszIn, "pid", pszPid))
		{
		    SavePid(atoi(pszId), atoi(pszPid));
		}
		else
		{
		    if (GetStringOpt(p->pszIn, "error", pszStr))
		    {
			SaveError(atoi(pszId), pszStr);
		    }
		}
	    }
	    else
	    {
		GetStringOpt(p->pszIn, "src", pszHost);
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    err_printf("launched result went full circle, discarding\n");
		}
		else
		{
		    // forward the message along
		    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_LAUNCH_RESULT);
		}
	    }
	}
	else if (strnicmp(p->pszIn, "exitcode ", 9) == 0)
	{
	    GetStringOpt(p->pszIn, "dest", pszHost);
	    if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
	    {
		char pszId[10];
		char pszExitCode[10];
		GetStringOpt(p->pszIn, "id", pszId);
		GetStringOpt(p->pszIn, "code", pszExitCode);
		SaveExitCode(atoi(pszId), atoi(pszExitCode));
	    }
	    else
	    {
		GetStringOpt(p->pszIn, "src", pszHost);
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    err_printf("exitcode result went full circle, discarding\n");
		}
		else
		{
		    // forward the message along
		    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_EXITCODE);
		}
	    }
	}
	else if (strnicmp(p->pszIn, "hosts ", 6) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszBFD[10];
		    MPD_Context *pContext;
		    
		    GetStringOpt(p->pszIn, "result", pszStr);
		    GetStringOpt(p->pszIn, "bfd", pszBFD);
		    pContext = GetContext(atoi(pszBFD));
		    if (pContext != NULL)
		    {
			EnqueueWrite(pContext, pszStr, MPD_WRITING_HOSTS_RESULT);
		    }
		    else
		    {
			err_printf("console context not found\n");
		    }
		}
		else
		{
		    strcpy(pszStr, p->pszIn);
		    strcat(pszStr, ",");
		    strcat(pszStr, g_pszHost);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_HOSTS_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid hosts command '%s' read\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "next ", 5) == 0)
	{
	    char pszBfd[10];
	    char pszN[10] = "0";
	    int n;
	    GetStringOpt(p->pszIn, "n", pszN);
	    GetStringOpt(p->pszIn, "src", pszHost);
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    n = atoi(pszN);
	    if ((n > 0) || (n < 16384))
	    {
		n--;
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=%s", g_pszHost, pszHost, pszBfd, g_pszHost);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		if (n > 0)
		{
		    sprintf(pszStr, "next src=%s bfd=%s n=%d", pszHost, pszBfd, n);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=Error: invalid number of hosts requested", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "barrier ", 8) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		if (strcmp(pszHost, g_pszHost) && strcmp(pszHost, g_pszIP))
		{
		    char pszName[100], pszCount[10];
		    if (GetStringOpt(p->pszIn, "name", pszName))
		    {
			if (GetStringOpt(p->pszIn, "count", pszCount))
			{
			    SetBarrier(pszName, atoi(pszCount), BFD_INVALID_SOCKET);
			    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
			}
		    }
		}
	    }
	}
	else if (strnicmp(p->pszIn, "ps ", 3) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszBFD[10];
		    MPD_Context *pContext;
		    
		    if (GetStringOpt(p->pszIn, "result", pszStr))
		    {
			// Chop off the trailing carriage return
			int nLength = strlen(pszStr);
			if (nLength > 2)
			{
			    if (pszStr[nLength-1] == '\r' || pszStr[nLength-1] == '\n')
				pszStr[nLength-1] = '\0';
			    if (pszStr[nLength-2] == '\r' || pszStr[nLength-2] == '\n')
				pszStr[nLength-2] = '\0';
			}
		    }
		    GetStringOpt(p->pszIn, "bfd", pszBFD);
		    pContext = GetContext(atoi(pszBFD));
		    if (pContext != NULL)
		    {
			EnqueueWrite(pContext, pszStr, MPD_WRITING_RESULT);
		    }
		    else
		    {
			err_printf("console context not found\n");
		    }
		}
		else
		{
		    strcpy(pszStr, p->pszIn);
		    ConcatenateProcessesToString(pszStr);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_HOSTS_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid ps command '%s' read\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "lefthost ", 9) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost) && GetStringOpt(p->pszIn, "host", g_pszInsertHost))
	    {
		WriteMPDRegistry(INSERT1, g_pszInsertHost);
		
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    sprintf(pszStr, "leftlefthost src=%s host=%s", g_pszHost, g_pszInsertHost);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    sprintf(pszStr, "lefthost src=%s host=%s", pszHost, g_pszHost);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid lefthost command '%s'\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "leftlefthost ", 13) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost) && GetStringOpt(p->pszIn, "host", g_pszInsertHost2))
	    {
		WriteMPDRegistry(INSERT2, g_pszInsertHost2);
		
		if ((strcmp(pszHost, g_pszHost)) && (strcmp(pszHost, g_pszIP)))
		{
		    sprintf(pszStr, "leftlefthost src=%s host=%s", pszHost, g_pszInsertHost);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid lefthost command '%s'\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "kill ", 5) == 0)
	{
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszPid[10];
		    if (GetStringOpt(p->pszIn, "pid", pszPid))
		    {
			int nPid = atoi(pszPid);
			//dbg_printf("MPD_KillProcess host=%s pid=%d\n", g_pszHost, nPid);
			MPD_KillProcess(nPid);
		    }
		}
		else
		{
		    if (GetStringOpt(p->pszIn, "src", pszStr))
		    {
			if (stricmp(pszStr, g_pszHost) == 0)
			{
			    char pszTry[10], pszPid[10];
			    if (!GetStringOpt(p->pszIn, "try", pszTry))
			    {
				if (GetIPString(pszStr, pszHost))
				{
				    // try the kill command again with the ip string instead of the host name
				    sprintf(pszStr, "kill src=%s host=%s pid=%s try=2", g_pszHost, pszHost, pszPid);
				    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
				}
			    }
			    else
			    {
				dbg_printf("kill command went full circle without matching any hosts, '%s'\n", p->pszIn);
			    }
			}
		    }
		    else
		    {
			err_printf("kill command has no source, '%s'\n", p->pszIn);
		    }
		}
	    }
	}
	else if (strnicmp(p->pszIn, "map ", 4) == 0)
	{
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		char pszDrive[10];
		char pszShare[MAX_PATH];
		char pszAccount[40];
		char pszPassword[40];
		char pszError[256];
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    if (!GetStringOpt(p->pszIn, "drive", pszDrive))
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no drive specified", g_pszHost, pszHost, pszBfd);
		    }
		    else if (!GetStringOpt(p->pszIn, "share", pszShare))
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no share specified", g_pszHost, pszHost, pszBfd);
		    }
		    else if (!GetStringOpt(p->pszIn, "account", pszAccount))
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no user account specified", g_pszHost, pszHost, pszBfd);
		    }
		    else if (GetStringOpt(p->pszIn, "password", pszPassword))
		    {
			if (!MapDrive(pszDrive, pszShare, pszAccount, pszPassword, pszError))
			{
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - %s", g_pszHost, pszHost, pszBfd, pszError);
			}
			else
			{
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=SUCCESS", g_pszHost, pszHost, pszBfd);
			}
		    }
		    else
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no password specified", g_pszHost, pszHost, pszBfd);
		    }
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			GetStringOpt(p->pszIn, "host", pszHost);
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    if (!GetStringOpt(p->pszIn, "drive", pszDrive))
			    {
				sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no drive specified", g_pszHost, pszHost, pszBfd);
			    }
			    else if (!GetStringOpt(p->pszIn, "share", pszShare))
			    {
				sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no share specified", g_pszHost, pszHost, pszBfd);
			    }
			    else if (!GetStringOpt(p->pszIn, "account", pszAccount))
			    {
				sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no user account specified", g_pszHost, pszHost, pszBfd);
			    }
			    else if (GetStringOpt(p->pszIn, "password", pszPassword))
			    {
				GetIPString(pszHost, pszHost);
				sprintf(pszStr, "map src=%s host=%s bfd=%s try=2 drive=%s share=%s account=%s password=%s", g_pszHost, pszHost, pszBfd, pszDrive, pszShare, pszAccount, pszPassword);
			    }
			    else
			    {
				sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no password specified", g_pszHost, pszHost, pszBfd);
			    }
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - host not in the ring", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no host provided", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "unmap ", 4) == 0)
	{
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		char pszDrive[10];
		char pszError[256];
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    if (GetStringOpt(p->pszIn, "drive", pszDrive))
		    {
			if (!UnmapDrive(pszDrive, pszError))
			{
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - %s", g_pszHost, pszHost, pszBfd, pszError);
			}
			else
			{
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=SUCCESS", g_pszHost, pszHost, pszBfd);
			}
		    }
		    else
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no drive specified", g_pszHost, pszHost, pszBfd);
		    }
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			GetStringOpt(p->pszIn, "host", pszHost);
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    if (GetStringOpt(p->pszIn, "drive", pszDrive))
			    {
				GetIPString(pszHost, pszHost);
				sprintf(pszStr, "unmap src=%s host=%s bfd=%s try=2 drive=%s", g_pszHost, pszHost, pszBfd, pszDrive);
			    }
			    else
			    {
				sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no drive specified", g_pszHost, pszHost, pszBfd);
			    }
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    char pszBadHost[MAX_HOST_LENGTH];
			    GetStringOpt(p->pszIn, "host", pszBadHost);
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - host '%s' not in the ring", g_pszHost, pszHost, pszBfd, pszBadHost);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no host provided", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "killall ", 8) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		ShutdownAllProcesses();
		// Conceivably there could be forwarders at this node not associated with processes in this ring.
		// In that case you should not call AbortAllForwarders(), but I am not going to figure out how to track that
		// case any time soon.
		AbortAllForwarders();
		if ((strcmp(pszHost, g_pszHost) != 0) && (strcmp(pszHost, g_pszIP) != 0))
		{
		    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid killall command '%s' read\n", p->pszIn);
	    }
	}
	else if (stricmp(p->pszIn, "exitall") == 0)
	{
	    if (g_bExitAllRoot)
	    {
		RemoveContext(g_pRightContext);
		p->nState = MPD_INVALID;
		p->bDeleteMe = true;
		SignalExit();
		SignalExit();
	    }
	    else
	    {
		EnqueueWrite(g_pRightContext, "exitall", MPD_WRITING_EXITALL_CMD);
	    }
	}
	else if (stricmp(p->pszIn, "done") == 0)
	{
	    dbg_printf("left[%d] read 'done'\n", p->bfd);
	    p->nState = MPD_INVALID;
	    p->bDeleteMe = true;
	}
	else if (stricmp(p->pszIn, "new left") == 0)
	{
	    if (p == g_pLeftContext)
	    {
		err_printf("Error, current left thread context read 'new left' command\n");
	    }
	    EnqueueWrite(p, g_pLeftContext->pszHost, MPD_WRITING_OLD_LEFT_HOST);
	    EnqueueWrite(g_pLeftContext, "done", MPD_WRITING_DONE);
	    POP_FUNC();
	    return;
	}
	else if (stricmp(p->pszIn, "connect left") == 0)
	{
	    p->nLLState = MPD_READING_NEW_LEFT_HOST;
	    p->nCurPos = 0;
	    POP_FUNC();
	    return;
	}
	else if (strnicmp(p->pszIn, "set ", 4) == 0)
	{
	    char pszKey[100];
	    GetStringOpt(p->pszIn, "key", pszKey);
	    GetStringOpt(p->pszIn, "value", pszStr);
	    
	    WriteMPDRegistry(pszKey, pszStr);
	    GetStringOpt(p->pszIn, "src", pszHost);
	    if ((stricmp(pszHost, g_pszHost)) && (strcmp(pszHost, g_pszIP)))
	    {
		EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "createforwarder ", 16) == 0)
	{
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    if (GetStringOpt(p->pszIn, "forward", pszHost))
		    {
			char *token = strtok(pszHost, ":");
			if (token != NULL)
			{
			    token = strtok(NULL, "\n");
			    int nPort = CreateIOForwarder(pszHost, atoi(token));

			    GetStringOpt(p->pszIn, "src", pszHost);
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=%d", g_pszHost, pszHost, pszBfd, nPort);
			}
			else
			{
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=-1", g_pszHost, pszHost, pszBfd);
			}
		    }
		    else
		    {
			sprintf(pszStr, "result src=%s dest=%s bfd=%s result=-1", g_pszHost, pszHost, pszBfd);
		    }
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			char pszForward[100];
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    GetStringOpt(p->pszIn, "forward", pszForward);
			    GetStringOpt(p->pszIn, "host", pszHost);
			    GetIPString(pszHost, pszHost);
			    sprintf(pszStr, "createforwarder src=%s host=%s bfd=%s try=2 forward=%s", g_pszHost, pszHost, pszBfd, pszForward);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=-1", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=-1", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "stopforwarder ", 14) == 0)
	{
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszPort[10];
		    char pszAbort[10];
		    bool bAbort = true;
		    if (GetStringOpt(p->pszIn, "port", pszPort))
		    {
			if (GetStringOpt(p->pszIn, "abort", pszAbort))
			    bAbort = (stricmp(pszAbort, "yes") == 0);
			StopIOForwarder(atoi(pszPort), !bAbort);
		    }
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			char pszPort[10];
			char pszAbort[10];
			bool bAbort = true;
			if (GetStringOpt(p->pszIn, "port", pszPort))
			{
			    if (GetStringOpt(p->pszIn, "abort", pszAbort))
				bAbort = (stricmp(pszAbort, "yes") == 0);
			    if (!GetStringOpt(p->pszIn, "try", pszTry))
			    {
				GetStringOpt(p->pszIn, "host", pszHost);
				GetIPString(pszHost, pszHost);
				sprintf(pszStr, "stopforwarder src=%s host=%s bfd=%s try=2 port=%s", g_pszHost, pszHost, pszBfd, pszPort);
				if (!bAbort)
				    strcat(pszStr, " abort=no");
				EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			    }
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	}
	else if (strnicmp(p->pszIn, "forwarders ", 11) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		if ((strcmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszBFD[10];
		    MPD_Context *pContext;
		    
		    if (GetStringOpt(p->pszIn, "result", pszStr))
		    {
			// Chop off the trailing carriage return
			int nLength = strlen(pszStr);
			if (nLength > 2)
			{
			    if (pszStr[nLength-1] == '\r' || pszStr[nLength-1] == '\n')
				pszStr[nLength-1] = '\0';
			    if (pszStr[nLength-2] == '\r' || pszStr[nLength-2] == '\n')
				pszStr[nLength-2] = '\0';
			}
		    }
		    GetStringOpt(p->pszIn, "bfd", pszBFD);
		    pContext = GetContext(atoi(pszBFD));
		    if (pContext != NULL)
		    {
			EnqueueWrite(pContext, pszStr, MPD_WRITING_RESULT);
		    }
		    else
		    {
			err_printf("console context not found\n");
		    }
		}
		else
		{
		    strcpy(pszStr, p->pszIn);
		    ConcatenateForwardersToString(pszStr);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_HOSTS_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid forwarders command '%s' read\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "killforwarders ", 15) == 0)
	{
	    if (GetStringOpt(p->pszIn, "src", pszHost))
	    {
		AbortAllForwarders();
		if ((strcmp(pszHost, g_pszHost) != 0) && (strcmp(pszHost, g_pszIP) != 0))
		{
		    EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		err_printf("invalid killforwarders command '%s' read\n", p->pszIn);
	    }
	}
	else if (strnicmp(p->pszIn, "createtmpfile ", 14) == 0)
	{
	    char pszBfd[10];
	    bool bDelete = true;
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "delete", pszStr))
	    {
		bDelete = (stricmp(pszStr, "no") != 0);
	    }
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszTemp[MAX_PATH];
		    CreateTmpFile(pszTemp, bDelete);
		    GetStringOpt(p->pszIn, "src", pszHost);
		    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=%s", g_pszHost, pszHost, pszBfd, pszTemp);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    GetStringOpt(p->pszIn, "host", pszHost);
			    GetIPString(pszHost, pszHost);
			    sprintf(pszStr, "createtmpfile src=%s host=%s bfd=%s try=2", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - bad hostname", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no host provided", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "deletetmpfile ", 14) == 0)
	{
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszTemp[MAX_PATH];
		    if (GetStringOpt(p->pszIn, "file", pszTemp))
		    {
			if (DeleteTmpFile(pszTemp))
			    strcpy(pszTemp, "SUCCESS");
			else
			    sprintf(pszTemp, "FAIL - error %d", GetLastError());
		    }
		    else
			strcpy(pszTemp, "FAIL - no filename provided");
		    GetStringOpt(p->pszIn, "src", pszHost);
		    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=%s", g_pszHost, pszHost, pszBfd, pszTemp);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    GetStringOpt(p->pszIn, "host", pszHost);
			    GetIPString(pszHost, pszHost);
			    sprintf(pszStr, "deletetmpfile src=%s host=%s bfd=%s try=2", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - bad hostname", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no host provided", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "mpich1readint ", 14) == 0)
	{
	    char pszBfd[10];
	    char pszPid[10] = "0";
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    GetStringOpt(p->pszIn, "pid", pszPid);
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		{
		    char pszTemp[MAX_PATH];
		    if (GetStringOpt(p->pszIn, "file", pszTemp))
		    {
			int nPort, nError;
			nError = GetPortFromFile(pszTemp, atoi(pszPid), &nPort);
			if (nError == 0)
			    sprintf(pszTemp, "%d", nPort);
			else
			{
			    //sprintf(pszTemp, "FAIL - error %d", nError);
			    if (nError == -1)
			    {
				strcpy(pszTemp, "FAIL - timed out");
			    }
			    else if (nError == -2)
			    {
				strcpy(pszTemp, "FAIL - missing dll");
			    }
			    else if (nError == -3)
			    {
				sprintf(pszTemp, "FAIL - process exited with code %d", nPort);
			    }
			    else
			    {
				Translate_Error(nError, pszTemp, "FAIL - ");
			    }
			}
		    }
		    else
			strcpy(pszTemp, "FAIL - no filename provided");
		    GetStringOpt(p->pszIn, "src", pszHost);
		    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=%s", g_pszHost, pszHost, pszBfd, pszTemp);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    GetStringOpt(p->pszIn, "src", pszHost);
		    if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    {
			char pszTry[10];
			char pszTemp[MAX_PATH];
			if (!GetStringOpt(p->pszIn, "try", pszTry))
			{
			    GetStringOpt(p->pszIn, "file", pszTemp);
			    GetStringOpt(p->pszIn, "host", pszHost);
			    GetIPString(pszHost, pszHost);
			    sprintf(pszStr, "mpich1readint src=%s host=%s bfd=%s try=2 pid=%s file=%s", g_pszHost, pszHost, pszBfd, pszPid, pszTemp);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
			else
			{
			    // command went full circle, send fail result
			    sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - bad hostname", g_pszHost, pszHost, pszBfd);
			    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
			}
		    }
		    else
		    {
			// forward command
			EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
		    }
		}
	    }
	    else
	    {
		// no host provided, send fail result
		GetStringOpt(p->pszIn, "src", pszHost);
		sprintf(pszStr, "result src=%s dest=%s bfd=%s result=FAIL - no host provided", g_pszHost, pszHost, pszBfd);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "result ", 7) == 0)
	{
	    char pszDest[MAX_HOST_LENGTH] = "";
	    char pszBfd[10];
	    GetStringOpt(p->pszIn, "dest", pszDest);
	    GetStringOpt(p->pszIn, "bfd", pszBfd);
	    if ((stricmp(pszDest, g_pszHost) == 0) || (strcmp(pszDest, g_pszIP) == 0))
	    {
		char *token;
		token = strstr(p->pszIn, "result=");
		if (token != NULL)
		{
		    token = &token[7];
		    p = GetContext(atoi(pszBfd));
		    if (p)
			EnqueueWrite(p, token, MPD_WRITING_RESULT);
		    else
		    {
			err_printf("GetContext failed for '%s'\n", pszBfd);
		    }
		}
		else
		{
		    err_printf("'result=' not found in result command\n");
		}
	    }
	    else
	    {
		EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	    }
	}
	else
	{
	    err_printf("left socket %d read unknown command '%s'\n", p->bfd, p->pszIn);
	}
	p->nLLState = MPD_READING_CMD;
	break;
    case MPD_READING_NEW_LEFT_HOST:
	dbg_printf("connecting to new left host: '%s'\n", p->pszIn);
	dbg_printf("removing left[%d]\n", p->bfd);
	p->bDeleteMe = true;
	p->nState = MPD_INVALID;
	pContext = new MPD_Context;
	pContext->nState = MPD_IDLE;
	beasy_create(&pContext->bfd, ADDR_ANY, INADDR_ANY);
	if (beasy_connect(pContext->bfd, p->pszIn, g_nPort) == SOCKET_ERROR)
	{
	    err_printf("connect to new left host '%s' failed\n", p->pszIn);
	    RemoveContext(pContext);
	    Extract(true);
	    break;
	}
	strcpy(pContext->pszHost, p->pszIn);
	strcpy(g_pszLeftHost, p->pszIn);

	pContext->nType = MPD_SOCKET;
	pContext->nState = MPD_IDLE;
	pContext->nLLState = MPD_AUTHENTICATE_READING_APPEND;
	pContext->nConnectingState = MPD_CONNECTING_LEFT;
	pContext->pNext = g_pList;
	g_pList = pContext;
	DoReadSet(pContext->bfd);
	break;
    default:
	err_printf("left context read '%s' while in unhandled state %d\n", p->pszIn, p->nLLState);
	break;
    }
    POP_FUNC();
}

void HandleLeftWritten(MPD_Context *p)
{
    PUSH_FUNC("HandleLeftWritten");
    dbg_printf("LeftWritten[%d]: '%s'\n", p->bfd, p->pszOut);
    switch (p->nLLState)
    {
    case MPD_WRITING_DONE_EXIT:
	SignalExit();
    case MPD_WRITING_DONE:
	//dbg_printf("done written, left[%d] state: MPD_INVALID\n", p->bfd);
	p->bDeleteMe = true;
	p->nState = MPD_INVALID;
	break;
    case MPD_WRITING_OLD_LEFT_HOST:
	dbg_printf("wrote old left host '%s'\n", p->pszOut);
	// Make p the new left context and add it to the global list of contexts
	g_pLeftContext = p;
	strcpy(g_pszLeftHost, p->pszHost);
	break;
    default:
	err_printf("unhandled low level state %d after write on left socket %d\n", p->nLLState, p->bfd);
	break;
    }
    DequeueWrite(p);
    POP_FUNC();
}
