#include "mpdimpl.h"
#include "bsocket.h"
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "Service.h"
#include "database.h"
#include "GetStringOpt.h"
#include "Translate_Error.h"

enum LaunchStatus
{
    LAUNCH_SUCCESS,
    LAUNCH_PENDING,
    LAUNCH_FAIL,
    LAUNCH_EXITED,
    LAUNCH_INVALID
};

struct LaunchStateStruct
{
    LaunchStateStruct();
    int nId;
    int nBfd;
    int nPid;
    LaunchStatus nStatus;
    char pszError[256];
    int nExitCode;
    bool bPidRequested;
    bool bExitStateRequested;
    char pszHost[MAX_HOST_LENGTH];
    LaunchStateStruct *pNext;
};

LaunchStateStruct::LaunchStateStruct()
{
    nId = 0;
    nBfd = BFD_INVALID_SOCKET;
    nPid = -1;
    nStatus = LAUNCH_INVALID;
    pszError[0] = '\0';
    nExitCode = 0;
    bPidRequested = false;
    bExitStateRequested = false;
    pszHost[0] = '\0';
    pNext = NULL;
}

int g_nCurrentLaunchId = 0;
LaunchStateStruct *g_pLaunchList = NULL;

LaunchStateStruct* GetLaunchStruct(int nId)
{
    PUSH_FUNC("GetLaunchStruct");
    LaunchStateStruct *p = g_pLaunchList;
    while (p)
    {
	if (p->nId == nId)
	{
	    POP_FUNC();
	    return p;
	}
	p = p->pNext;
    }
    POP_FUNC();
    return NULL;
}

int ConsoleGetExitCode(int nPid)
{
    LaunchStateStruct *pLS = GetLaunchStruct(nPid);
    if (pLS != NULL)
    {
	if (pLS->nStatus == LAUNCH_EXITED)
	{
	    return pLS->nExitCode;
	}
	return -1;
    }
    return -2;
}

void RemoveStateStruct(LaunchStateStruct *p)
{
    LaunchStateStruct *pTrailer = g_pLaunchList;

    // Remove p from the list
    if (p == NULL)
	return;

    if (p == g_pLaunchList)
	g_pLaunchList = g_pLaunchList->pNext;
    else
    {
	while (pTrailer && pTrailer->pNext != p)
	    pTrailer = pTrailer->pNext;
	if (pTrailer)
	    pTrailer->pNext = p->pNext;
    }

    //dbg_printf("removing LaunchStateStruct[%d]\n", p->nId);
    // free the structure
    delete p;
}

void SavePid(int nId, int nPid)
{
    LaunchStateStruct *p;
    
    PUSH_FUNC("SavePid");

    p = GetLaunchStruct(nId);

    if (p != NULL)
    {
	p->nStatus = LAUNCH_SUCCESS;
	p->nPid = nPid;
	strcpy(p->pszError, "ERROR_SUCCESS");
	if (p->bPidRequested)
	{
	    char pszStr[10];
	    sprintf(pszStr, "%d", p->nPid);
	    beasy_send(p->nBfd, pszStr, strlen(pszStr)+1);
	    p->bPidRequested = false;
	}
    }
    POP_FUNC();
}

void SaveError(int nId, char *pszError)
{
    LaunchStateStruct *p;
    
    PUSH_FUNC("SaveError");

    p = GetLaunchStruct(nId);
    if (p != NULL)
    {
	p->nStatus = LAUNCH_FAIL;
	strcpy(p->pszError, pszError);
	if (p->bPidRequested)
	{
	    beasy_send(p->nBfd, "-1", strlen("-1")+1);
	    p->bPidRequested = false;
	}
	if (p->bExitStateRequested)
	{
	    beasy_send(p->nBfd, "FAIL", strlen("FAIL")+1);
	    p->bExitStateRequested = false;
	}
    }
    POP_FUNC();
}

void SaveExitCode(int nId, int nExitCode)
{
    LaunchStateStruct *p;
    
    PUSH_FUNC("SaveExitCode");
    p = GetLaunchStruct(nId);
    if (p != NULL)
    {
	p->nStatus = LAUNCH_EXITED;
	p->nExitCode = nExitCode;
	if (p->bExitStateRequested)
	{
	    char pszStr[10];
	    sprintf(pszStr, "%d:%d", nExitCode, p->nPid);
	    beasy_send(p->nBfd, pszStr, strlen(pszStr)+1);
	    p->bExitStateRequested = false;
	    dbg_printf("sending exit code %d:%d\n", nId, nExitCode);
	}
    }
    else
    {
	err_printf("ERROR: Saving exit code for launchid %d failed\n", nId);
    }
    POP_FUNC();
}

void GetNameKeyValue(char *str, char *name, char *key, char *value)
{
    bool bName = false;
    bool bKey = false;
    bool bValue = false;

    PUSH_FUNC("GetNameKeyValue");

    //dbg_printf("GetNameKeyValue(");

    if ((name != NULL) && (!GetStringOpt(str, "name", name)))
    {
	bName = true;
    }
    /*
    else
    {
	if (name != NULL)
	{
	    dbg_printf("name='%s' ", name);
	}
    }
    */
    if ((key != NULL) && (!GetStringOpt(str, "key", key)))
    {
	bKey = true;
    }
    /*
    else
    {
	if (key != NULL)
	{
	    dbg_printf("key='%s' ", key);
	}
    }
    */
    if ((value != NULL) && (!GetStringOpt(str, "value", value)))
    {
	bValue = true;
    }
    /*
    else
    {
	if (value != NULL)
	{
	    dbg_printf("value='%s'", value);
	}
    }
    */

    char str2[MAX_CMD_LENGTH];
    char *token;
    if (bName)
    {
	strcpy(str2, str);
	token = strtok(str2, ":");
	if (token != NULL)
	{
	    strcpy(name, token);
	    //dbg_printf("name='%s' ", name);
	    if (bKey)
	    {
		token = strtok(NULL, ":");
		if (token != NULL)
		{
		    strcpy(key, token);
		    //dbg_printf("key='%s' ", key);
		    if (bValue)
		    {
			token = strtok(NULL, ":");
			if (token != NULL)
			{
			    strcpy(value, token);
			    //dbg_printf("value='%s'", value);
			}
		    }
		}
	    }
	}
    }
    else if (bKey)
    {
	strcpy(str2, str);
	token = strtok(str2, ":");
	if (token != NULL)
	{
	    strcpy(key, token);
	    //dbg_printf("key='%s' ", key);
	    if (bValue)
	    {
		token = strtok(NULL, ":");
		if (token != NULL)
		{
		    strcpy(value, token);
		    //dbg_printf("value='%s'", value);
		}
	    }
	}
    }
    else if (bValue)
    {
	strcpy(value, str);
	//dbg_printf("value='%s'", value);
    }

    //dbg_printf(")\n");

    //dbg_printf("GetNameKeyValue('%s' '%s' '%s')\n", name ? name : "NULL", key ? key : "NULL", value ? value : "NULL");
    POP_FUNC();
}

static void ParseAccountDomain(char *DomainAccount, char *tAccount, char *tDomain)
{
    char *pCh, *pCh2;
    
    pCh = DomainAccount;
    pCh2 = tDomain;
    while ((*pCh != '\\') && (*pCh != '\0'))
    {
	*pCh2 = *pCh;
	pCh++;
	pCh2++;
    }
    if (*pCh == '\\')
    {
	pCh++;
	strcpy(tAccount, pCh);
	*pCh2 = '\0';
    }
    else
    {
	strcpy(tAccount, DomainAccount);
	tDomain[0] = '\0';
    }
}

HANDLE BecomeUser(char *domainaccount, char *password, int *pnError)
{
    HANDLE hUser;
    char account[50], domain[50], *pszDomain;
    ParseAccountDomain(domainaccount, account, domain);
    if (strlen(domain) < 1)
	pszDomain = NULL;
    else
	pszDomain = domain;

    HANDLE hLogonMutex = CreateMutex(NULL, FALSE, "mpdLaunchMutex");
    WaitForSingleObject(hLogonMutex, 10000);

    if (!LogonUser(
	account,
	pszDomain, 
	password,
	LOGON32_LOGON_INTERACTIVE, 
	//LOGON32_LOGON_BATCH,  // quicker?
	LOGON32_PROVIDER_DEFAULT, 
	&hUser))
    {
	*pnError = GetLastError();
	ReleaseMutex(hLogonMutex);
	CloseHandle(hLogonMutex);
	return (HANDLE)-1;
    }

    if (!ImpersonateLoggedOnUser(hUser))
    {
	*pnError = GetLastError();
	CloseHandle(hUser);
	ReleaseMutex(hLogonMutex);
	CloseHandle(hLogonMutex);
	if (!g_bSingleUser)
	    RevertToSelf();
	return (HANDLE)-1;
    }

    ReleaseMutex(hLogonMutex);
    CloseHandle(hLogonMutex);

    return hUser;
}

#define TRANSFER_BUFFER_SIZE 20*1024

bool TryCreateDir(char *pszFileName, char *pszError)
{
    char pszTemp[MAX_PATH];
    char *token, *next_token;
    int error;

    if (pszFileName[1] == ':')
    {
	strncpy(pszTemp, pszFileName, 3);
	pszTemp[3] = '\0';
	//dbg_printf("changing into directory '%s'\n", pszTemp);
	if (!SetCurrentDirectory(pszTemp))
	{
	    sprintf(pszError, "unable to change to '%s' directory", pszTemp);
	    return false;
	}
	strcpy(pszTemp, &pszFileName[3]);
    }
    else
    {
	sprintf(pszError, "full path not provided");
	// full path not provided
	return false;
    }

    token = strtok(pszTemp, "\\/");
    while (token)
    {
	next_token = strtok(NULL, "\\/");
	if (next_token == NULL)
	    return true;
	//dbg_printf("creating directory '%s'\n", token);
	if (!CreateDirectory(token, NULL))
	{
	    error = GetLastError();
	    if (error != ERROR_ALREADY_EXISTS)
	    {
		sprintf(pszError, "unable to create directory '%s', error %d\n", token, error);
		return false;
	    }
	}
	SetCurrentDirectory(token);
	token = next_token;
    }
    strcpy(pszError, "unknown error");
    return false;
}

FILE* CreateCheckFile(char *pszFullFileName, bool bReplace, bool bCreateDir, char *pszError)
{
    char pszPath[MAX_PATH];
    char *pszFileName, *p1, *p2;
    FILE *fout;

    if (bCreateDir)
    {
	if (!TryCreateDir(pszFullFileName, pszError))
	    return NULL;
    }
    strcpy(pszPath, pszFullFileName);
    p1 = strrchr(pszPath, '\\');
    p2 = strrchr(pszPath, '/');
    pszFileName = max(p1, p2);
    *pszFileName = '\0';
    pszFileName++;
    //dbg_printf("pszPath: '%s', pszFileName: '%s'\n", pszPath, pszFileName);
    if (!SetCurrentDirectory(pszPath))
    {
	sprintf(pszError, "SetCurrentDirectory(%s) failed, error %d", pszPath, GetLastError());
	return NULL;
    }

    if (bReplace)
    {
	fout = fopen(pszFileName, "wb");
    }
    else
    {
	fout = fopen(pszFileName, "r");
	if (fout != NULL)
	{
	    sprintf(pszError, "file exists");
	    fclose(fout);
	    return NULL;
	}
	fclose(fout);
	fout = fopen(pszFileName, "wb");
    }
    if (fout == NULL)
    {
	sprintf(pszError, "fopen failed, error %d", GetLastError());
	return NULL;
    }

    return fout;
}

static int WriteString(int bfd, char *str)
{
    return beasy_send(bfd, str, strlen(str)+1);
}

HANDLE ParseBecomeUser(MPD_Context *p, char *pszInputStr, bool bMinusOneOnError)
{
    int nError;
    HANDLE hUser = NULL;

    if (!g_bSingleUser)
    {
	if (!p->bFileInitCalled)
	{
	    if (bMinusOneOnError)
		WriteString(p->bfd, "-1");
	    WriteString(p->bfd, "ERROR - no account and password provided");
	    return (HANDLE)-1;
	}
	hUser = BecomeUser(p->pszFileAccount, p->pszFilePassword, &nError);
	if (hUser == (HANDLE)-1)
	{
	    char pszStr[256];
	    Translate_Error(nError, pszStr, "ERROR - ");
	    if (bMinusOneOnError)
		WriteString(p->bfd, "-1");
	    WriteString(p->bfd, pszStr);
	    return (HANDLE)-1;
	}
    }
    return hUser;
}

void LoseTheUser(HANDLE hUser)
{
    if (!g_bSingleUser)
    {
	RevertToSelf();
	if (hUser != NULL)
	    CloseHandle(hUser);
    }
}

static void ConsolePutFile(int bfd, char *pszInputStr)
{
    char pszFileName[MAX_PATH];
    int nLength;
    int nNumRead;
    FILE *fin;
    char pBuffer[TRANSFER_BUFFER_SIZE];
    char pszStr[256];
    int nError;

    // Get the file name
    if (!GetStringOpt(pszInputStr, "name", pszFileName))
    {
	WriteString(bfd, "-1");
	WriteString(bfd, "ERROR - no file name provided");
	return;
    }

    // Open the file
    fin = fopen(pszFileName, "rb");
    if (fin == NULL)
    {
	nError = GetLastError();
	Translate_Error(nError, pszStr, "ERROR - fopen failed, ");
	WriteString(bfd, "-1");
	WriteString(bfd, pszStr);
	return;
    }

    // Send the size
    fseek(fin, 0, SEEK_END);
    nLength = ftell(fin);
    if (nLength == -1)
    {
	nError = GetLastError();
	Translate_Error(nError, pszStr, "ERROR - Unable to determine the size of the file, ");
	WriteString(bfd, "-1");
	WriteString(bfd, pszStr);
	return;
    }
    sprintf(pszStr, "%d", nLength);
    WriteString(bfd, pszStr);

    // Rewind back to the beginning
    fseek(fin, 0, SEEK_SET);

    // Send the data
    while (nLength)
    {
	nNumRead = min(nLength, TRANSFER_BUFFER_SIZE);
	nNumRead = fread(pBuffer, 1, nNumRead, fin);
	if (nNumRead < 1)
	{
	    err_printf("fread failed, %d\n", ferror(fin));
	    fclose(fin);
	    return;
	}
	if (beasy_send(bfd, pBuffer, nNumRead) == SOCKET_ERROR)
	{
	    err_printf("sending file data failed, file=%s, error=%d", pszFileName, WSAGetLastError());
	    fclose(fin);
	    return;
	}
	//printf("%d bytes sent\n", nNumRead);fflush(stdout);
	nLength -= nNumRead;
    }
    fclose(fin);
}

static void ConsoleGetFile(int bfd, char *pszInputStr)
{
    bool bReplace = true, bCreateDir = false;
    char pszFileName[MAX_PATH];
    char pszStr[256];
    int nLength;
    FILE *fout;
    char pBuffer[TRANSFER_BUFFER_SIZE];
    int nNumRead;
    int nNumWritten;

    if (GetStringOpt(pszInputStr, "replace", pszStr))
    {
	bReplace = (stricmp(pszStr, "yes") == 0);
    }
    if (GetStringOpt(pszInputStr, "createdir", pszStr))
    {
	bCreateDir = (stricmp(pszStr, "yes") == 0);
    }
    if (GetStringOpt(pszInputStr, "length", pszStr))
    {
	nLength = atoi(pszStr);
	//dbg_printf("nLength: %d\n", nLength);
    }
    else
    {
	WriteString(bfd, "ERROR - length not provided");
	return;
    }
    if (nLength < 1)
    {
	WriteString(bfd, "ERROR - invalid length");
	return;
    }

    if (!GetStringOpt(pszInputStr, "name", pszFileName))
    {
	WriteString(bfd, "ERROR - no file name provided");
	return;
    }

    //dbg_printf("creating file '%s'\n", pszFileName);
    fout = CreateCheckFile(pszFileName, bReplace, bCreateDir, pszStr);

    if (fout == NULL)
    {
	WriteString(bfd, pszStr);
	return;
    }

    //dbg_printf("SEND\n");
    WriteString(bfd, "SEND");

    while (nLength)
    {
	nNumRead = min(nLength, TRANSFER_BUFFER_SIZE);
	if (beasy_receive(bfd, pBuffer, nNumRead) == SOCKET_ERROR)
	{
	    err_printf("ERROR: beasy_receive failed, error %d\n", WSAGetLastError());
	    fclose(fout);
	    DeleteFile(pszFileName);
	    return;
	}
	nNumWritten = fwrite(pBuffer, 1, nNumRead, fout);
	if (nNumWritten != nNumRead)
	{
	    err_printf("ERROR: received %d bytes but only wrote %d bytes\n", nNumRead, nNumWritten);
	}
	//dbg_printf("%d bytes read, %d bytes written\n", nNumRead, nNumWritten);
	nLength -= nNumRead;
    }

    fclose(fout);

    WriteString(bfd, "SUCCESS");
}

static void GetDirectoryFiles(int bfd, char *pszInputStr)
{
    char pszPath[MAX_PATH];
    char pszStr[MAX_CMD_LENGTH];
    int nFolders = 0, nFiles = 0;
    WIN32_FIND_DATA data;
    HANDLE hFind;

    if (!GetStringOpt(pszInputStr, "path", pszPath))
    {
	WriteString(bfd, "ERROR: no path specified");
	return;
    }
    if (strlen(pszPath) < 1)
    {
	WriteString(bfd, "ERROR: empty path specified");
	return;
    }

    if (pszPath[strlen(pszPath)-1] != '\\')
    {
	strcat(pszPath, "\\");
    }
    strcat(pszPath, "*");

    // Count the files and folders
    // What if the contents change between the counting and the sending?
    hFind = FindFirstFile(pszPath, &data);

    if (hFind == INVALID_HANDLE_VALUE)
    {
	Translate_Error(GetLastError(), pszStr, "ERROR: ");
	WriteString(bfd, pszStr);
	return;
    }

    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
	if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, ".."))
	    nFolders++;
    }
    else
	nFiles++;

    while (FindNextFile(hFind, &data))
    {
	if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, ".."))
		nFolders++;
	}
	else
	    nFiles++;
    }

    FindClose(hFind);

    // Send the folders
    sprintf(pszStr, "%d", nFolders);
    WriteString(bfd, pszStr);

    hFind = FindFirstFile(pszPath, &data);

    if (hFind == INVALID_HANDLE_VALUE)
	return;

    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
	if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, ".."))
	    WriteString(bfd, data.cFileName);
    }

    while (FindNextFile(hFind, &data))
    {
	if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, ".."))
		WriteString(bfd, data.cFileName);
	}
    }

    FindClose(hFind);

    // Send the files
    sprintf(pszStr, "%d", nFiles);
    WriteString(bfd, pszStr);

    hFind = FindFirstFile(pszPath, &data);

    if (hFind == INVALID_HANDLE_VALUE)
	return;

    if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
	WriteString(bfd, data.cFileName);
	if (data.nFileSizeHigh > 0)
	    sprintf(pszStr, "%d:%d", data.nFileSizeLow, data.nFileSizeHigh);
	else
	    sprintf(pszStr, "%d", data.nFileSizeLow);
	WriteString(bfd, pszStr);
    }

    while (FindNextFile(hFind, &data))
    {
	if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
	    WriteString(bfd, data.cFileName);
	    if (data.nFileSizeHigh > 0)
		sprintf(pszStr, "%d:%d", data.nFileSizeLow, data.nFileSizeHigh);
	    else
		sprintf(pszStr, "%d", data.nFileSizeLow);
	    WriteString(bfd, pszStr);
	}
    }

    FindClose(hFind);
}

static void HandleDBCommandRead(MPD_Context *p)
{
    char name[MAX_DBS_NAME_LEN+1] = "";
    char key[MAX_DBS_KEY_LEN+1] = "";
    char value[MAX_DBS_VALUE_LEN+1] = "";
    char pszStr[MAX_CMD_LENGTH] = "";

    PUSH_FUNC("HandleDBCommandRead");

    if (strnicmp(p->pszIn, "dbput ", 6) == 0)
    {
	GetNameKeyValue(&p->pszIn[6], name, key, value);
	if (dbs_put(name, key, value) == DBS_SUCCESS)
	{
	    EnqueueWrite(p, DBS_SUCCESS_STR, MPD_WRITING_RESULT);
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (strnicmp(p->pszIn, "dbget ", 6) == 0)
    {
	GetNameKeyValue(&p->pszIn[6], name, key, NULL);
	if (dbs_get(name, key, value) == DBS_SUCCESS)
	{
	    EnqueueWrite(p, value, MPD_WRITING_RESULT);
	}
	else
	{
	    sprintf(pszStr, "dbget src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[6]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
    }
    else if (stricmp(p->pszIn, "dbcreate") == 0)
    {
	// Create the database locally
	if (dbs_create(name) == DBS_SUCCESS)
	{
	    // Write the name back to the user
	    EnqueueWrite(p, name, MPD_WRITING_RESULT);
	    // Create the database on all the other nodes
	    sprintf(pszStr, "dbcreate src=%s bfd=%d name=%s", g_pszHost, p->bfd, name);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (strnicmp(p->pszIn, "dbcreate ", 9) == 0)
    {
	GetNameKeyValue(&p->pszIn[9], name, NULL, NULL);
	if (dbs_create_name_in(name) == DBS_SUCCESS)
	{
	    EnqueueWrite(p, DBS_SUCCESS_STR, MPD_WRITING_RESULT);
	    // Create the database on all the other nodes
	    sprintf(pszStr, "dbcreate src=%s bfd=%d name=%s", g_pszHost, p->bfd, name);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (strnicmp(p->pszIn, "dbdestroy ", 10) == 0)
    {
	// forward the destroy command
	sprintf(pszStr, "dbdestroy src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[10]);
	EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);

	// destroy the database locally
	GetNameKeyValue(&p->pszIn[10], name, NULL, NULL);
	if (dbs_destroy(name) == DBS_SUCCESS)
	    EnqueueWrite(p, DBS_SUCCESS_STR, MPD_WRITING_RESULT);
	else
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
    }
    else if (strnicmp(p->pszIn, "dbfirst ", 8) == 0)
    {
	GetNameKeyValue(&p->pszIn[8], name, NULL, NULL);
	if (dbs_first(name, key, value) == DBS_SUCCESS)
	{
	    // forward the first command
	    sprintf(pszStr, "dbfirst src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[8]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);

	    if (*key == '\0')
	    {
		// If the local database is empty, forward a dbnext command
		sprintf(pszStr, "dbnext src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[8]);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	    else
	    {
		sprintf(p->pszOut, "key=%s value=%s", key, value);
		EnqueueWrite(p, p->pszOut, MPD_WRITING_RESULT);
	    }
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (strnicmp(p->pszIn, "dbnext ", 7) == 0)
    {
	GetNameKeyValue(&p->pszIn[7], name, NULL, NULL);
	if (dbs_next(name, key, value) == DBS_SUCCESS)
	{
	    if (*key == '\0')
	    {
		sprintf(pszStr, "dbnext src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[7]);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	    else
	    {
		sprintf(p->pszOut, "key=%s value=%s", key, value);
		EnqueueWrite(p, p->pszOut, MPD_WRITING_RESULT);
	    }
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (stricmp(p->pszIn, "dbfirstdb") == 0)
    {
	if (dbs_firstdb(name) == DBS_SUCCESS)
	{
	    if (*name == '\0')
		strcpy(p->pszOut, DBS_END_STR);
	    else
		sprintf(p->pszOut, "name=%s", name);
	    EnqueueWrite(p, p->pszOut, MPD_WRITING_RESULT);
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (stricmp(p->pszIn, "dbnextdb") == 0)
    {
	if (dbs_nextdb(name) == DBS_SUCCESS)
	{
	    if (*name == '\0')
		strcpy(p->pszOut, DBS_END_STR);
	    else
		sprintf(p->pszOut, "name=%s", name);
	    EnqueueWrite(p, p->pszOut, MPD_WRITING_RESULT);
	}
	else
	{
	    EnqueueWrite(p, DBS_FAIL_STR, MPD_WRITING_RESULT);
	}
    }
    else if (strnicmp(p->pszIn, "dbdelete ", 9) == 0)
    {
	// Attempt to delete locally
	GetNameKeyValue(&p->pszIn[9], name, key, NULL);
	if (dbs_delete(name, key) == DBS_SUCCESS)
	{
	    EnqueueWrite(p, DBS_SUCCESS_STR, MPD_WRITING_RESULT);
	}
	else
	{
	    // forward the delete command
	    sprintf(pszStr, "dbdelete src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[9]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
    }
    else
    {
	err_printf("unknown command '%s'", p->pszIn);
    }
    POP_FUNC();
}

void HandleConsoleRead(MPD_Context *p)
{
    char pszStr[MAX_CMD_LENGTH];

    PUSH_FUNC("HandleConsoleRead");

    dbg_printf("ConsoleRead[%d]: '%s'\n", p->bfd, p->pszIn);
    switch(p->nLLState)
    {
    case MPD_READING_CMD:
	if (strnicmp(p->pszIn, "db", 2) == 0)
	{
	    HandleDBCommandRead(p);
	}
	else if (strnicmp(p->pszIn, "launch ", 7) == 0)
	{
	    char pszHost[MAX_HOST_LENGTH];
	    g_nCurrentLaunchId++;
	    LaunchStateStruct *pLS = new LaunchStateStruct;
	    pLS->nStatus = LAUNCH_PENDING;
	    strcpy(pLS->pszError, "LAUNCH_PENDING");
	    pLS->nId = g_nCurrentLaunchId;
	    pLS->nBfd = p->bfd;
	    pLS->pNext = g_pLaunchList;
	    if (!GetStringOpt(&p->pszIn[7], "h", pLS->pszHost))
		strcpy(pLS->pszHost, g_pszHost);
	    g_pLaunchList = pLS;
	    sprintf(pszStr, "launch src=%s id=%d %s", g_pszHost, pLS->nId, &p->pszIn[7]);
	    if (GetStringOpt(pszStr, "h", pszHost))
	    {
		if ((stricmp(pszHost, g_pszHost) == 0) || (strcmp(pszHost, g_pszIP) == 0))
		    Launch(pszStr);
		else
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_LAUNCH_CMD);
	    }
	    else
		Launch(pszStr); // No host provided so launch locally
	    sprintf(pszStr, "%d", pLS->nId);
	    EnqueueWrite(p, pszStr, MPD_WRITING_LAUNCH_RESULT);
	}
	else if (strnicmp(p->pszIn, "getpid ", 7) == 0)
	{
	    LaunchStateStruct *pLS = GetLaunchStruct(atoi(&p->pszIn[7]));
	    if (pLS != NULL)
	    {
		if (pLS->nStatus == LAUNCH_PENDING)
		{
		    pLS->bPidRequested = true;
		}
		else
		{
		    if (pLS->nStatus == LAUNCH_SUCCESS)
			sprintf(pszStr, "%d", pLS->nPid);
		    else
			strcpy(pszStr, "-1");
		    EnqueueWrite(p, pszStr, MPD_WRITING_LAUNCH_RESULT);
		}
	    }
	    else
	    {
		EnqueueWrite(p, "-1", MPD_WRITING_LAUNCH_RESULT);
	    }
	}
	else if (strnicmp(p->pszIn, "getexitcode ", 12) == 0)
	{
	    LaunchStateStruct *pLS = GetLaunchStruct(atoi(&p->pszIn[12]));
	    if (pLS != NULL)
	    {
		if (pLS->nStatus == LAUNCH_EXITED)
		{
		    dbg_printf("sending exit code %d:%d\n", atoi(&p->pszIn[12]), pLS->nExitCode);
		    sprintf(pszStr, "%d", pLS->nExitCode);
		}
		else
		{
		    if (pLS->nStatus == LAUNCH_SUCCESS)
			strcpy(pszStr, "ACTIVE");
		    else
			strcpy(pszStr, "FAIL");
		}
		EnqueueWrite(p, pszStr, MPD_WRITING_LAUNCH_RESULT);
	    }
	    else
	    {
		EnqueueWrite(p, "FAIL", MPD_WRITING_LAUNCH_RESULT);
	    }
	}
	else if (strnicmp(p->pszIn, "getexitcodewait ", 16) == 0)
	{
	    LaunchStateStruct *pLS = GetLaunchStruct(atoi(&p->pszIn[16]));
	    if (pLS != NULL)
	    {
		if (pLS->nStatus == LAUNCH_SUCCESS)
		{
		    pLS->bExitStateRequested = true;
		}
		else
		{
		    if (pLS->nStatus == LAUNCH_EXITED)
		    {
			dbg_printf("sending exit code %d:%d\n", atoi(&p->pszIn[16]), pLS->nExitCode);
			sprintf(pszStr, "%d", pLS->nExitCode);
		    }
		    else
			strcpy(pszStr, "FAIL");
		    EnqueueWrite(p, pszStr, MPD_WRITING_LAUNCH_RESULT);
		}
	    }
	    else
	    {
		EnqueueWrite(p, "FAIL", MPD_WRITING_LAUNCH_RESULT);
	    }
	}
	else if (strnicmp(p->pszIn, "geterror ", 9) == 0)
	{
	    LaunchStateStruct *pLS = GetLaunchStruct(atoi(&p->pszIn[9]));
	    if (pLS != NULL)
	    {
		EnqueueWrite(p, pLS->pszError, MPD_WRITING_RESULT);
	    }
	    else
	    {
		EnqueueWrite(p, "invalid launch id", MPD_WRITING_RESULT);
	    }
	}
	else if (strnicmp(p->pszIn, "freeprocess ", 12) == 0)
	{
	    RemoveStateStruct(GetLaunchStruct(atoi(&p->pszIn[12])));
	}
	else if (strnicmp(p->pszIn, "kill ", 5) == 0)
	{
	    char pszTemp1[MAX_HOST_LENGTH], pszTemp2[10];
	    if (GetStringOpt(p->pszIn, "host", pszTemp1) && GetStringOpt(p->pszIn, "pid", pszTemp2))
	    {
		strcat(p->pszIn, " src=");
		strcat(p->pszIn, g_pszHost);
		EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_CMD);
	    }
	    else
	    {
		LaunchStateStruct *pLS = GetLaunchStruct(atoi(&p->pszIn[5]));
		if (pLS != NULL)
		{
		    sprintf(pszStr, "kill src= %s host=%s pid=%d", g_pszHost, pLS->pszHost, pLS->nPid);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		{
		    EnqueueWrite(p, "invalid launch id", MPD_WRITING_RESULT);
		}
	    }
	}
	else if (strnicmp(p->pszIn, "map ", 4) == 0)
	{
    	    char pszHost[MAX_HOST_LENGTH];
	    if (!GetStringOpt(p->pszIn, "host", pszHost))
	    {
		strcat(p->pszIn, " host=");
		strcat(p->pszIn, g_pszHost);
	    }
	    sprintf(pszStr, "map src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[4]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "unmap ", 6) == 0)
	{
    	    char pszHost[MAX_HOST_LENGTH];
	    if (!GetStringOpt(p->pszIn, "host", pszHost))
	    {
		strcat(p->pszIn, " host=");
		strcat(p->pszIn, g_pszHost);
	    }
	    sprintf(pszStr, "unmap src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[6]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (stricmp(p->pszIn, "killall") == 0)
	{
	    sprintf(pszStr, "killall src=%s", g_pszHost);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (stricmp(p->pszIn, "hosts") == 0)
	{
	    sprintf(pszStr, "hosts src=%s bfd=%d result=%s", g_pszHost, p->bfd, g_pszHost);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_HOSTS_CMD);
	}
	else if (strnicmp(p->pszIn, "next ", 5) == 0)
	{
	    int n = atoi(&p->pszIn[5]);
	    if ((n > 0) || (n < 16384))
	    {
		n--;
		WriteString(p->bfd, g_pszHost);
		if (n > 0)
		{
		    sprintf(pszStr, "next src=%s bfd=%d n=%d", g_pszHost, p->bfd, n);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
	    }
	    else
	    {
		WriteString(p->bfd, "Error: invalid number of hosts requested");
	    }
	}
	else if (strnicmp(p->pszIn, "barrier ", 8) == 0)
	{
	    char pszName[100], pszCount[10];
	    if (GetStringOpt(p->pszIn, "name", pszName))
	    {
		if (GetStringOpt(p->pszIn, "count", pszCount))
		{
		    SetBarrier(pszName, atoi(pszCount), p->bfd);
		    sprintf(pszStr, "barrier src=%s name=%s count=%s", g_pszHost, pszName, pszCount);
		    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
		}
		else
		    WriteString(p->bfd, "Error: invalid barrier command, no count specified");
	    }
	    else
		WriteString(p->bfd, "Error: invalid barrier command, no name specified");
	}
	else if (stricmp(p->pszIn, "ps") == 0)
	{
	    sprintf(pszStr, "ps src=%s bfd=%d result=", g_pszHost, p->bfd);
	    ConcatenateProcessesToString(pszStr);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (stricmp(p->pszIn, "extract") == 0)
	{
	    if (!Extract(true))
	    {
		err_printf("Extract failed\n");
	    }
	    p->nLLState = MPD_READING_CMD;
	}
	else if (stricmp(p->pszIn, "done") == 0)
	{
	    p->bDeleteMe = true;
	    p->nState = MPD_INVALID;
	}
	else if (stricmp(p->pszIn, "set nodes") == 0)
	{
	    sprintf(pszStr, "lefthost src=%s host=%s", g_pszHost, g_pszHost);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "set ", 4) == 0)
	{
	    char pszKey[100], *pszValue;
	    int nLength;
	    pszValue = strstr(p->pszIn, "=");
	    if (pszValue != NULL)
	    {
		nLength = pszValue - &p->pszIn[4];
		memcpy(pszKey, &p->pszIn[4], nLength);
		pszKey[nLength] = '\0';
		pszValue++;
		sprintf(pszStr, "set src=%s key=%s value=%s", g_pszHost, pszKey, pszValue);
		EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	    }
	}
	else if (strnicmp(p->pszIn, "lset ", 5) == 0)
	{
	    char pszKey[100], *pszValue;
	    int nLength;
	    pszValue = strstr(p->pszIn, "=");
	    if (pszValue != NULL)
	    {
		nLength = pszValue - &p->pszIn[5];
		memcpy(pszKey, &p->pszIn[5], nLength);
		pszKey[nLength] = '\0';
		pszValue++;
		WriteMPDRegistry(pszKey, pszValue);
	    }
	}
	else if (strnicmp(p->pszIn, "lget ", 5) == 0)
	{
	    pszStr[0] = '\0';
	    ReadMPDRegistry(&p->pszIn[5], pszStr);
	    EnqueueWrite(p, pszStr, MPD_WRITING_RESULT);
	}
	else if (strnicmp(p->pszIn, "ldelete ", 8) == 0)
	{
	    DeleteMPDRegistry(&p->pszIn[8]);
	}
	else if (strnicmp(p->pszIn, "insert ", 7) == 0)
	{
	    if (!InsertIntoRing(&p->pszIn[7]))
	    {
		sprintf(pszStr, "%s failed\n", p->pszIn);
		EnqueueWrite(p, pszStr, MPD_WRITING_RESULT);
	    }
	    else
	    {
		p->nLLState = MPD_READING_CMD;
	    }
	}
	else if (stricmp(p->pszIn, "shutdown") == 0)
	{
	    ServiceStop();
	}
	else if (stricmp(p->pszIn, "exitall") == 0)
	{
	    g_bExitAllRoot = true;
	    EnqueueWrite(g_pRightContext, "exitall", MPD_WRITING_FIRST_EXITALL_CMD);
	}
	else if (stricmp(p->pszIn, "version") == 0)
	{
	    sprintf(pszStr, "%d.%d.%d", VERSION_RELEASE, VERSION_MAJOR, VERSION_MINOR);
	    EnqueueWrite(p, pszStr, MPD_WRITING_RESULT);
	}
	else if (stricmp(p->pszIn, "config") == 0)
	{
	    pszStr[0] = '\0';
	    MPDRegistryToString(pszStr);
	    EnqueueWrite(p, pszStr, MPD_WRITING_RESULT);
	}
	else if (stricmp(p->pszIn, "print") == 0)
	{
	    int nSent;
	    char *buf, *pBuf;
	    int size;
	    FILE *fout = tmpfile();
	    
	    PrintState(fout);

	    size = ftell(fout);
	    //dbg_printf("print command wrote %d bytes to tmp file\n", size);
	    fseek( fout, 0L, SEEK_SET );
	    buf = new char[size+1];
	    pBuf = buf;
	    while (size)
	    {
		nSent = fread(pBuf, 1, size, fout);
		if (nSent == size)
		{
		    pBuf[size] = '\0';
		    beasy_send(p->bfd, pBuf, size+1);
		}
		else
		    beasy_send(p->bfd, pBuf, nSent);
		size = size - nSent;
		pBuf = pBuf + nSent;
	    }
	    delete buf;
	    fclose(fout);
	}
	else if (strnicmp(p->pszIn, "createforwarder ", 16) == 0)
	{
	    sprintf(pszStr, "createforwarder src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[16]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "stopforwarder ", 14) == 0)
	{
	    char pszHost[100];
	    if (GetStringOpt(p->pszIn, "host", pszHost))
	    {
		char *token = strtok(pszHost, ":");
		if (token != NULL)
		{
		    token = strtok(NULL, "\n");
		    if (token != NULL)
		    {
			int nPort = atoi(token);
			if (nPort > 0)
			{
			    sprintf(&p->pszIn[14], "host=%s port=%d", pszHost, nPort);
			}
		    }
		}
	    }
	    else
	    {
		if (GetStringOpt(p->pszIn, "port", pszHost))
		{
		    strcat(p->pszIn, " host=");
		    strcat(p->pszIn, g_pszHost);
		}
		else
		{
		    if (strstr(p->pszIn, ":") != NULL)
		    {
			strncpy(pszHost, &p->pszIn[14], 100);
			pszHost[99] = '\0';
			char *token = strtok(pszHost, ":");
			if (token != NULL)
			{
			    token = strtok(NULL, "\n");
			    if (token != NULL)
			    {
				int nPort = atoi(token);
				if (nPort > 0)
				{
				    sprintf(&p->pszIn[14], "host=%s port=%d", pszHost, nPort);
				}
			    }
			}
		    }
		    else
		    {
			int nPort = atoi(&p->pszIn[14]);
			if (nPort > 0)
			{
			    sprintf(&p->pszIn[14], "host=%s port=%d", g_pszHost, nPort);
			}
		    }
		}
	    }
	    sprintf(pszStr, "stopforwarder src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[14]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (stricmp(p->pszIn, "forwarders") == 0)
	{
	    sprintf(pszStr, "forwarders src=%s bfd=%d result=", g_pszHost, p->bfd);
	    ConcatenateForwardersToString(pszStr);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (stricmp(p->pszIn, "killforwarders") == 0)
	{
	    sprintf(pszStr, "killforwarders src=%s", g_pszHost);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "createtmpfile ", 14) == 0)
	{
	    sprintf(pszStr, "createtmpfile src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[14]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "deletetmpfile ", 14) == 0)
	{
	    sprintf(pszStr, "deletetmpfile src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[14]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "mpich1readint ", 14) == 0)
	{
	    sprintf(pszStr, "mpich1readint src=%s bfd=%d %s", g_pszHost, p->bfd, &p->pszIn[14]);
	    EnqueueWrite(g_pRightContext, pszStr, MPD_WRITING_CMD);
	}
	else if (strnicmp(p->pszIn, "putfile ", 8) == 0)
	{
	    HANDLE hUser;
	    hUser = ParseBecomeUser(p, &p->pszIn[8], false);
	    if (hUser != (HANDLE)-1)
	    {
		ConsoleGetFile(p->bfd, &p->pszIn[8]);
		LoseTheUser(hUser);
	    }
	}
	else if (strnicmp(p->pszIn, "getfile ", 8) == 0)
	{
	    HANDLE hUser;
	    hUser = ParseBecomeUser(p, &p->pszIn[8], true);
	    if (hUser != (HANDLE)-1)
	    {
		ConsolePutFile(p->bfd, &p->pszIn[8]);
		LoseTheUser(hUser);
	    }
	}
	else if (strnicmp(p->pszIn, "getdir ", 7) == 0)
	{
	    HANDLE hUser;
	    hUser = ParseBecomeUser(p, &p->pszIn[7], false);
	    if (hUser != (HANDLE)-1)
	    {
		GetDirectoryFiles(p->bfd, &p->pszIn[7]);
		LoseTheUser(hUser);
	    }
	}
	else if (strnicmp(p->pszIn, "fileinit ", 9) == 0)
	{
	    if (GetStringOpt(p->pszIn, "account", p->pszFileAccount) && 
		GetStringOpt(p->pszIn, "password", p->pszFilePassword))
	    {
		p->bFileInitCalled = true;
	    }
	}
	else if (strnicmp(p->pszIn, "update ", 7) == 0)
	{
	    UpdateMPD(&p->pszIn[7]);
	}
	else if (stricmp(p->pszIn, "restart") == 0)
	{
	    WriteString(p->bfd, "Restarting mpd...");
	    RestartMPD();
	}
	else
	{
	    err_printf("console socket read unknown command: '%s'\n", p->pszIn);
	    p->nLLState = MPD_READING_CMD;
	}
	break;
    default:
	err_printf("unexpected read in console state %d, '%s'\n", p->nLLState, p->pszIn);
	p->nLLState = MPD_READING_CMD;
	break;
    }
    POP_FUNC();
}

void HandleConsoleWritten(MPD_Context *p)
{
    PUSH_FUNC("HandleConsoleWritten");

    dbg_printf("ConsoleWritten[%d]: %s\n", p->bfd, p->pszOut);
    switch (p->nLLState)
    {
    case MPD_WRITING_RESULT:
    case MPD_WRITING_HOSTS_RESULT:
	p->nLLState = MPD_READING_CMD;
	break;
    default:
	break;
    }
    DequeueWrite(p);

    POP_FUNC();
}
