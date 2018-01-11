// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsocket.h"
#include "service.h"
#include "mpdimpl.h"
#include "GetOpt.h"
#include "GetStringOpt.h"
#include "database.h"
#include "Translate_Error.h"

// Global variables

__declspec(thread) char g_call_stack[100][FUNC_STR_LENGTH+1];
__declspec(thread) int g_call_index = 0;

int g_nPort = 0;
char g_pszHost[MAX_HOST_LENGTH] = "";
char g_pszLeftHost[MAX_HOST_LENGTH] = "";
char g_pszRightHost[MAX_HOST_LENGTH] = "";
char g_pszInsertHost[MAX_HOST_LENGTH] = "";
char g_pszInsertHost2[MAX_HOST_LENGTH] = "";
char g_pszIP[25] = "";
unsigned long g_nIP = 0;
char g_pszTempDir[MAX_PATH] = "C:\\";

MPD_Context *g_pList = NULL;
MPD_Context *g_pRightContext = NULL;
MPD_Context *g_pLeftContext = NULL;
bfd_set g_ReadSet, g_WriteSet;
int g_bfdSignal = BFD_INVALID_SOCKET;
int g_bfdReSelect = BFD_INVALID_SOCKET;
int g_maxfds = 0;
int g_nSignalCount = 2;
bool g_bExitAllRoot = false;
bool g_bSingleUser = false;
bool g_bStartAlone = false;
int g_nActiveW = 0;
int g_nActiveR = 0;

extern "C" {
__declspec(dllexport) int mpdVersionRelease = VERSION_RELEASE;
__declspec(dllexport) int mpdVersionMajor = VERSION_MAJOR;
__declspec(dllexport) int mpdVersionMinor = VERSION_MINOR;
__declspec(dllexport) char mpdVersionDate[] = __DATE__;
}

void GetMPDVersion(char *str, int length)
{
    _snprintf(str, length, "%d.%d.%d %s", VERSION_RELEASE, VERSION_MAJOR, VERSION_MINOR, __DATE__);
}

void GetMPICHVersion(char *str, int length)
{
    void (*pGetMPICHVersion)(char *str, int length);
    char *filename = NULL, *name_part;
    DWORD len;
    HMODULE hModule;
    char err_msg[1024];

    if (length < 1)
	return;

    len = SearchPath(NULL, "mpich.dll", NULL, 0, filename, &name_part);

    if (len == 0)
    {
	printf("unable to find mpich.dll\n");fflush(stdout);
	*str = '\0';
	return;
    }

    filename = new char[len*2+2];
    len = SearchPath(NULL, "mpich.dll", NULL, len*2, filename, &name_part);
    if (len == 0)
    {
	printf("unable to find mpich.dll\n");fflush(stdout);
	*str = '\0';
	delete filename;
	return;
    }

    hModule = LoadLibrary(filename);
    delete filename;

    if (hModule == NULL)
    {
	Translate_Error(GetLastError(), err_msg, NULL);
	printf("LoadLibrary(mpich.dll) failed, ");
	printf("%s\n", err_msg);fflush(stdout);
	*str = '\0';
	return;
    }

    pGetMPICHVersion = (void (*)(char *, int))GetProcAddress(hModule, "GetMPICHVersion");

    if (pGetMPICHVersion == NULL)
    {
	Translate_Error(GetLastError(), err_msg, "GetProcAddress(\"GetMPICHVersion\") failed, ");
	printf("%s\n", err_msg);fflush(stdout);
	*str = '\0';
	FreeLibrary(hModule);
	return;
    }

    pGetMPICHVersion(str, length);
    //printf("%s\n", version);
    FreeLibrary(hModule);
}

void SignalExit()
{
    PUSH_FUNC("SignalExit");
    g_nSignalCount--;
    if (g_nSignalCount == 0)
	ServiceStop();
    POP_FUNC();
}

void DoReadSet(int bfd)
{
    if (!BFD_ISSET(bfd, &g_ReadSet))
    {
	BFD_SET(bfd, &g_ReadSet);
	g_nActiveR++;
    }
}

void DoWriteSet(int bfd)
{
    if (!BFD_ISSET(bfd, &g_WriteSet))
    {
	BFD_SET(bfd, &g_WriteSet);
	g_nActiveW++;
    }
}

void PrintState(FILE *fout)
{
    MPD_Context *p;

    fprintf(fout, "STATE------------------------------------------------\n");
    fprintf(fout, "g_pList of contexts:\n");
    p = g_pList;
    while (p)
    {
	p->Print(fout);
	p = p->pNext;
    }
    fprintf(fout, "g_pRightContext:");
    if (g_pRightContext == NULL)
	fprintf(fout, " NULL\n");
    else
    {
	fprintf(fout, "\n");
	g_pRightContext->Print(fout);
    }
    fprintf(fout, "g_pLeftContext:");
    if (g_pLeftContext == NULL)
	fprintf(fout, " NULL\n");
    else
    {
	fprintf(fout, "\n");
	g_pLeftContext->Print(fout);
    }
    fprintf(fout, "g_nIP: %d, g_pszIP: %s\n", g_nIP, g_pszIP);
    fprintf(fout, "g_nPort: %d\n", g_nPort);
    fprintf(fout, "g_pszHost:        '%s'\n", g_pszHost);
    fprintf(fout, "g_pszLeftHost:    '%s'\n", g_pszLeftHost);
    fprintf(fout, "g_pszRightHost:   '%s'\n", g_pszRightHost);
    fprintf(fout, "g_pszInsertHost:  '%s'\n", g_pszInsertHost);
    fprintf(fout, "g_pszInsertHost2: '%s'\n", g_pszInsertHost2);
    fprintf(fout, "STATE------------------------------------------------\n");
}

static void printSet(char *str, bfd_set *set)
{
    MPD_Context *p;

    printf(str);
    p = g_pList;
    while (p)
    {
	if (p->nState == MPD_IDLE)
	{
	    if (BFD_ISSET(p->bfd, set))
	    {
		switch(p->nType)
		{
		case MPD_SOCKET:
		    printf("socket %d\n", p->bfd);
		    break;
		case MPD_LISTENER:
		    printf("listener %d\n", p->bfd);
		    break;
		case MPD_SIGNALLER:
		    printf("signaller %d\n", p->bfd);
		    break;
		}
	    }
	    else
	    {
		//printf("NOT set %d\n", p->bfd);
	    }
	}
	else
	{
	    printf("non-idle socket %d\n", p->bfd);
	}
	p = p->pNext;
    }
    fflush(stdout);
}

void HandleRightRead(MPD_Context *p)
{
    PUSH_FUNC("HandleRightRead");
    dbg_printf("RightRead[%d]: '%s'\n", p->bfd, p->pszIn);
    switch (p->nLLState)
    {
    case MPD_READING_CMD:
	if (stricmp(p->pszIn, "new right") == 0)
	{
	    g_pRightContext = p;
	    strncpy(g_pszRightHost, p->pszHost, MAX_HOST_LENGTH);
	}
	else if (stricmp(p->pszIn, "done") == 0)
	{
	    p->nState = MPD_INVALID;
	    p->bDeleteMe = true;
	}
	else
	{
	    err_printf("right socket %d read unknown command '%s'\n", p->bfd, p->pszIn);
	}
	break;
    case MPD_READING_LEFT_HOST:
	dbg_printf("right context[%d] read host, %s, from host %s\n", p->bfd, p->pszIn, p->pszHost);
	EnqueueWrite(g_pRightContext, "connect left", MPD_WRITING_CONNECT_LEFT);
	EnqueueWrite(g_pRightContext, p->pszIn, MPD_WRITING_NEW_LEFT_HOST);
	g_pRightContext = p;
	strncpy(g_pszRightHost, p->pszHost, MAX_HOST_LENGTH);
	p->nLLState = MPD_READING_CMD;
	break;
    default:
	err_printf("unhandled low level state %d after reading on right socket %d, '%s'\n", p->nLLState, p->bfd, p->pszIn);
	break;
    }
    POP_FUNC();
}

void HandleRightWritten(MPD_Context *p)
{
    PUSH_FUNC("HandleRightWritten");
    dbg_printf("RightWritten[%d]: '%s'\n", p->bfd, p->pszOut);
    switch (p->nLLState)
    {
    case MPD_WRITING_CMD:
    case MPD_WRITING_LAUNCH_CMD:
    case MPD_WRITING_HOSTS_CMD:
    case MPD_WRITING_FIRST_EXITALL_CMD:
    case MPD_WRITING_LAUNCH_RESULT:
    case MPD_WRITING_EXITCODE:
	break;
    case MPD_WRITING_EXITALL_CMD:
	RemoveContext(g_pLeftContext);
	p->nState = MPD_INVALID;
	p->bDeleteMe = true;
	SignalExit();
	SignalExit(); // Signal twice to get the service to stop
	break;
    case MPD_WRITING_NEW_LEFT:
	//dbg_printf("right[%d] state: MPD_READING, llstate: MPD_READING_LEFT_HOST\n", p->bfd);
	p->nState = MPD_READING;
	p->nLLState = MPD_READING_LEFT_HOST;
	BFD_CLR(p->bfd, &g_WriteSet);
	g_nActiveW--;
	POP_FUNC();
	return;
	break;
    case MPD_WRITING_CONNECT_LEFT:
	// Do nothing because the "new left host" has been enqueued for writing already
	break;
    case MPD_WRITING_NEW_LEFT_HOST_EXIT:
	SignalExit();
    case MPD_WRITING_NEW_LEFT_HOST:
	//dbg_printf("closing right[%d]\n", p->bfd);
	p->bDeleteMe = true;
	p->nState = MPD_INVALID;
	break;
    default:
	err_printf("unhandled low level state %d after write on right socket %d\n", p->nLLState, p->bfd);
	break;
    }
    DequeueWrite(p);
    POP_FUNC();
}

void HandleSocketRead(MPD_Context *p)
{
    char pszStr[256];
    char *phrase_out;
    char phrase_internal[MPD_PASSPHRASE_MAX_LENGTH+1];

    PUSH_FUNC("SocketRead");

    dbg_printf("HandleSocketRead[%d]: '%s'\n", p->bfd, p->pszIn);
    switch (p->nLLState)
    {
    case MPD_AUTHENTICATE_READING_APPEND:
	if (!ReadMPDRegistry("phrase", phrase_internal, false))
	{
	    err_printf("unable to read the passphrase\n");
	    phrase_internal[0] = '\0';
	}
	else
	{
	    if (strlen(p->pszIn) < MPD_PASSPHRASE_MAX_LENGTH)
	    {
		strncat(phrase_internal, p->pszIn, MPD_PASSPHRASE_MAX_LENGTH - strlen(p->pszIn));
	    }
	    else
	    {
		phrase_internal[0] = '\0';
	    }
	}
	//dbg_printf("calling crypt on '%s'\n", phrase_internal);
	phrase_out = crypt(phrase_internal, MPD_SALT_VALUE);
	memset(phrase_internal, 0, MPD_PASSPHRASE_MAX_LENGTH);
	EnqueueWrite(p, phrase_out, MPD_AUTHENTICATE_WRITING_CRYPTED);
	break;
    case MPD_AUTHENTICATE_READING_CRYPTED:
	EnqueueWrite(p, (strcmp(p->pszIn, p->pszCrypt) == 0) ? "SUCCESS" : "FAIL", MPD_AUTHENTICATE_WRITING_RESULT);
	break;
    case MPD_AUTHENTICATE_READING_RESULT:
	if (strcmp(p->pszIn, "SUCCESS") == 0)
	{
	    // Figure out who is requesting authentication and signal success
	    switch (p->nConnectingState)
	    {
	    case MPD_INSERTING:
		_snprintf(pszStr, 256, "left %s", g_pszHost);
		if (beasy_send(p->bfd, pszStr, strlen(pszStr)+1) == SOCKET_ERROR)
		{
		    p->nState = MPD_INVALID;
		    p->bDeleteMe = true;
		    return;
		}
		p->nType = MPD_RIGHT_SOCKET;
		p->nState = MPD_IDLE;
		g_maxfds = BFD_MAX(p->bfd, g_maxfds);
		DoReadSet(p->bfd);
		EnqueueWrite(p, "new left", MPD_WRITING_NEW_LEFT);
		break;
	    case MPD_CONNECTING_LEFT:
		_snprintf(pszStr, 256, "right %s", g_pszHost);
		if (beasy_send(p->bfd, pszStr, strlen(pszStr)+1) == SOCKET_ERROR)
		{
		    err_printf("beasy_send '%s' failed, error %d\n", pszStr, WSAGetLastError());
		    p->nState = MPD_INVALID;
		    p->bDeleteMe = true;
		    Extract(true);
		    break;
		}
		p->nType = MPD_LEFT_SOCKET;
		g_pLeftContext = p;
		dbg_printf("connected to '%s', new left context - bfd[%d]\n", p->pszHost, p->bfd);
		//EnqueueWrite(p, "new right", MPD_WRITING_NEW_RIGHT);
		if (beasy_send(p->bfd, "new right", strlen("new right")+1) == SOCKET_ERROR)
		    // This could fail if there isn't enough buffer space for "new right"
		{
		    err_printf("send 'new right' failed, error %d\n", WSAGetLastError());
		    p->nState = MPD_INVALID;
		    p->bDeleteMe = true;
		    Extract(true);
		    break;
		}
		DoReadSet(p->bfd);
		g_maxfds = BFD_MAX(p->bfd, g_maxfds);
		p->nLLState = MPD_READING_CMD;
		break;
	    default:
		err_printf("HandleSocketRead invalid connecting state %d after reading 'SUCCESS'\n", p->nConnectingState);
		break;
	    }
	}
	else
	{
	    // Figure out who is requesting authentication and signal failure
	    switch (p->nConnectingState)
	    {
	    case MPD_INSERTING:
	    case MPD_CONNECTING_LEFT:
		dbg_printf("left host denied authentication.\n");
		Extract(true);
		break;
	    default:
		err_printf("HandleSocketRead invalid connecting state %d after reading '%s'\n", p->nConnectingState, p->pszIn);
		break;
	    }
	    p->nState = MPD_INVALID;
	    p->bDeleteMe = true;
	}
	break;
    case MPD_AUTHENTICATED:
	if (stricmp(p->pszIn, "console") == 0)
	{
	    dbg_printf("bfd[%d] nType: MPD_CONSOLE_SOCKET\n", p->bfd);
	    p->nType = MPD_CONSOLE_SOCKET;
	    p->nLLState= MPD_READING_CMD;
	}
	else if (strnicmp(p->pszIn, "left ", 5) == 0)
	{
	    dbg_printf("bfd[%d] nType: MPD_LEFT_SOCKET\n", p->bfd);
	    p->nType = MPD_LEFT_SOCKET;
	    p->nLLState= MPD_READING_CMD;
	    strncpy(p->pszHost, &p->pszIn[5], MAX_HOST_LENGTH);
	    p->pszHost[MAX_HOST_LENGTH-1] = '\0';
	}
	else if (strnicmp(p->pszIn, "right ", 6) == 0)
	{
	    dbg_printf("bfd[%d] nType: MPD_RIGHT_SOCKET\n", p->bfd);
	    p->nType = MPD_RIGHT_SOCKET;
	    p->nLLState= MPD_READING_CMD;
	    strncpy(p->pszHost, &p->pszIn[6], MAX_HOST_LENGTH);
	    p->pszHost[MAX_HOST_LENGTH-1] = '\0';
	}
	else
	{
	    err_printf("unknown socket type read: '%s'\n", p->pszIn);
	}
	break;
    default:
	break;
    }
    POP_FUNC();
}

void HandleSocketWritten(MPD_Context *p)
{
    int nLLState;

    PUSH_FUNC("HandleSocketWritten");

    dbg_printf("SocketWritten[%d]: '%s'\n", p->bfd, p->pszOut);
    nLLState = p->nLLState;
    DequeueWrite(p);
    switch (nLLState)
    {
    case MPD_AUTHENTICATE_WRITING_APPEND:
	p->nLLState = MPD_AUTHENTICATE_READING_CRYPTED;
	break;
    case MPD_AUTHENTICATE_WRITING_CRYPTED:
	p->nLLState = MPD_AUTHENTICATE_READING_RESULT;
	break;
    case MPD_AUTHENTICATE_WRITING_RESULT:
	if (strcmp(p->pszOut, "SUCCESS") == 0)
	{
	    p->nLLState = MPD_AUTHENTICATED;
	}
	else
	{
	    p->nState = MPD_INVALID;
	    p->bDeleteMe = true;
	}
	break;
    default:
	err_printf("invalid low level state %d in bfd[%d] after writing '%s'\n", nLLState, p->bfd, p->pszOut);
	break;
    }
    POP_FUNC();
}

void StringRead(MPD_Context *p)
{
    PUSH_FUNC("StringRead");
    switch(p->nType)
    {
    case MPD_SOCKET:
	//dbg_printf("socket read '%s'\n", p->pszIn);
	HandleSocketRead(p);
	break;
    case MPD_LEFT_SOCKET:
	//dbg_printf("left socket read '%s'\n", p->pszIn);
	HandleLeftRead(p);
	break;
    case MPD_RIGHT_SOCKET:
	//dbg_printf("right socket read '%s'\n", p->pszIn);
	HandleRightRead(p);
	break;
    case MPD_CONSOLE_SOCKET:
	//dbg_printf("console socket read '%s'\n", p->pszIn);
	HandleConsoleRead(p);
	break;
    default:
	err_printf("string '%s' read on socket %d of unknown type %d\n", p->pszIn, p->bfd, p->nType);
	break;
    }
    POP_FUNC();
}

void StringWritten(MPD_Context *p)
{
    PUSH_FUNC("StringWritten");
    switch(p->nType)
    {
    case MPD_SOCKET:
	HandleSocketWritten(p);
	break;
    case MPD_LEFT_SOCKET:
	HandleLeftWritten(p);
	break;
    case MPD_RIGHT_SOCKET:
	HandleRightWritten(p);
	break;
    case MPD_CONSOLE_SOCKET:
	HandleConsoleWritten(p);
	break;
    default:
	err_printf("string '%s' written on socket %d of unknown type %d\n", p->pszOut, p->bfd, p->nType);
	break;
    }
    POP_FUNC();
}

void ResetSelect()
{
    PUSH_FUNC("ResetSelect");
    if (!beasy_send(g_bfdReSelect, "x", 1))
    {
	err_printf("beasy_send('x') failed, error %d\n", WSAGetLastError());
    }
    POP_FUNC();
}
