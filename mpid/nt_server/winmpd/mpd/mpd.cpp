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
//bool g_bRshMode = true;
//bool g_bRingMode = false;
bool g_bStartAlone = false;
int g_nActiveW = 0;
int g_nActiveR = 0;

HANDLE g_hBombDiffuseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hBombThread = NULL;

// Constructors and Destructors

MPD_Context::MPD_Context()
{
    nType = MPD_SOCKET; 
    bfd = BFD_INVALID_SOCKET; 
    pszHost[0] = '\0'; 
    pszIn[0] = '\0';
    pszOut[0] = '\0';
    nCurPos = 0; 
    nState = MPD_INVALID; 
    nLLState = MPD_INVALID_LOWLEVEL;
    bDeleteMe = false;
    pWriteList = NULL;
    bPassChecked = false;
    nConnectingState = MPD_INVALID_CONNECTING_STATE;
    bFileInitCalled = false;
    pszFileAccount[0] = '\0';
    pszFilePassword[0] = '\0';
    pNext = NULL; 
}

WriteNode::WriteNode()
{ 
    pString = NULL; 
    nState = MPD_INVALID_LOWLEVEL; 
    pNext = NULL; 
}

WriteNode::WriteNode(char *p, MPD_LowLevelState n)
{ 
    pString = new char[strlen(p)+1]; 
    strcpy(pString, p); 
    nState = n; 
    pNext = NULL; 
}

WriteNode::~WriteNode() 
{ 
    if (pString) 
	delete pString; 
    pString = NULL; 
}

// Function definitions

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

void printLLState(FILE *fout, MPD_LowLevelState nLLState)
{
    switch (nLLState)
    {
    case MPD_WRITING_CMD:
	fprintf(fout, "MPD_WRITING_CMD");
	break;
    case MPD_WRITING_LAUNCH_CMD:
	fprintf(fout, "MPD_WRITING_LAUNCH_CMD");
	break;
    case MPD_WRITING_LAUNCH_RESULT:
	fprintf(fout, "MPD_WRITING_LAUNCH_RESULT");
	break;
    case MPD_WRITING_EXITCODE:
	fprintf(fout, "MPD_WRITING_EXITCODE");
	break;
    case MPD_WRITING_FIRST_EXITALL_CMD:
	fprintf(fout, "MPD_WRITING_FIRST_EXITALL_CMD");
	break;
    case MPD_WRITING_EXITALL_CMD:
	fprintf(fout, "MPD_WRITING_EXITALL_CMD");
	break;
    case MPD_WRITING_KILL_CMD:
	fprintf(fout, "MPD_WRITING_KILL_CMD");
	break;
    case MPD_WRITING_HOSTS_CMD:
	fprintf(fout, "MPD_WRITING_HOSTS_CMD");
	break;
    case MPD_WRITING_HOSTS_RESULT:
	fprintf(fout, "MPD_WRITING_HOSTS_RESULT");
	break;
    case MPD_WRITING_RESULT:
	fprintf(fout, "MPD_WRITING_RESULT");
	break;
    case MPD_READING_CMD:
	fprintf(fout, "MPD_READING_CMD");
	break;
    case MPD_READING_NEW_LEFT:
	fprintf(fout, "MPD_READING_NEW_LEFT");
	break;
    case MPD_WRITING_OLD_LEFT_HOST:
	fprintf(fout, "MPD_WRITING_OLD_LEFT_HOST");
	break;
    case MPD_WRITING_DONE_EXIT:
	fprintf(fout, "MPD_WRITING_DONE_EXIT");
	break;
    case MPD_WRITING_DONE:
	fprintf(fout, "MPD_WRITING_DONE");
	break;
    case MPD_WRITING_NEW_LEFT:
	fprintf(fout, "MPD_WRITING_NEW_LEFT");
	break;
    case MPD_READING_LEFT_HOST:
	fprintf(fout, "MPD_READING_LEFT_HOST");
	break;
    case MPD_WRITING_CONNECT_LEFT:
	fprintf(fout, "MPD_WRITING_CONNECT_LEFT");
	break;
    case MPD_WRITING_NEW_LEFT_HOST_EXIT:
	fprintf(fout, "MPD_WRITING_NEW_LEFT_HOST_EXIT");
	break;
    case MPD_WRITING_NEW_LEFT_HOST:
	fprintf(fout, "MPD_WRITING_NEW_LEFT_HOST");
	break;
    case MPD_READING_CONNECT_LEFT:
	fprintf(fout, "MPD_READING_CONNECT_LEFT");
	break;
    case MPD_READING_NEW_LEFT_HOST:
	fprintf(fout, "MPD_READING_NEW_LEFT_HOST");
	break;
    case MPD_WRITING_NEW_RIGHT:
	fprintf(fout, "MPD_WRITING_NEW_RIGHT");
	break;
    case MPD_READING_NEW_RIGHT:
	fprintf(fout, "MPD_READING_NEW_RIGHT");
	break;
    case MPD_AUTHENTICATE_READING_APPEND:
	fprintf(fout, "MPD_AUTHENTICATE_READING_APPEND");
	break;
    case MPD_AUTHENTICATE_WRITING_APPEND:
	fprintf(fout, "MPD_AUTHENTICATE_WRITING_APPEND");
	break;
    case MPD_AUTHENTICATE_READING_CRYPTED:
	fprintf(fout, "MPD_AUTHENTICATE_READING_CRYPTED");
	break;
    case MPD_AUTHENTICATE_WRITING_CRYPTED:
	fprintf(fout, "MPD_AUTHENTICATE_WRITING_CRYPTED");
	break;
    case MPD_AUTHENTICATE_READING_RESULT:
	fprintf(fout, "MPD_AUTHENTICATE_READING_RESULT");
	break;
    case MPD_AUTHENTICATE_WRITING_RESULT:
	fprintf(fout, "MPD_AUTHENTICATE_WRITING_RESULT");
	break;
    case MPD_AUTHENTICATED:
	fprintf(fout, "MPD_AUTHENTICATED");
	break;
    case MPD_INVALID_LOWLEVEL:
	fprintf(fout, "MPD_INVALID_LOWLEVEL");
	break;
    default:
	fprintf(fout, "%d - invalid state", nLLState);
	break;
    }
}

void MPD_Context::Print(FILE *fout)
{
    fprintf(fout, "{\n");
    fprintf(fout, " nType: ");
    switch (nType)
    {
    case MPD_SOCKET:
	fprintf(fout, "MPD_SOCKET\n");
	break;
    case MPD_LISTENER:
	fprintf(fout, "MPD_LISTENER\n");
	break;
    case MPD_SIGNALLER:
	fprintf(fout, "MPD_SIGNALLER\n");
	break;
    case MPD_RESELECTOR:
	fprintf(fout, "MPD_RESELECTOR\n");
	break;
    case MPD_LEFT_SOCKET:
	fprintf(fout, "MPD_LEFT_SOCKET\n");
	break;
    case MPD_RIGHT_SOCKET:
	fprintf(fout, "MPD_RIGHT_SOCKET\n");
	break;
    case MPD_CONSOLE_SOCKET:
	fprintf(fout, "MPD_CONSOLE_SOCKET\n");
	break;
    default:
	fprintf(fout, "%d - invalid type\n", nType);
	break;
    }
    if (bfd == BFD_INVALID_SOCKET)
	fprintf(fout, " bfd: INVALID_SOCKET, ");
    else
	fprintf(fout, " bfd: %d, ", bfd);
    fprintf(fout, "pszHost: '%s', ", pszHost);
    fprintf(fout, "nCurPos: %d, ", nCurPos);
    if (bDeleteMe)
	fprintf(fout, "bDeleteMe: true\n");
    else
	fprintf(fout, "bDeleteMe: false\n");
    fprintf(fout, " pszIn: '%s'\n", pszIn);
    fprintf(fout, " pszOut: '%s'\n", pszOut);
    fprintf(fout, " states: ");
    switch (nState)
    {
    case MPD_IDLE:
	fprintf(fout, "MPD_IDLE, ");
	break;
    case MPD_READING:
	fprintf(fout, "MPD_READING, ");
	break;
    case MPD_WRITING:
	fprintf(fout, "MPD_WRITING, ");
	break;
    case MPD_INVALID:
	fprintf(fout, "MPD_INVALID, ");
	break;
    default:
	fprintf(fout, "%d - invalid state, ", nState);
	break;
    }
    printLLState(fout, nLLState);
    fprintf(fout, "\n");
    if (pWriteList == NULL)
	fprintf(fout, " pWriteList: NULL\n");
    else
    {
	WriteNode *pNode;
	fprintf(fout, " pWriteList:\n");
	pNode = pWriteList;
	while (pNode)
	{
	    fprintf(fout, "  (");
	    printLLState(fout, pNode->nState);
	    fprintf(fout, ", '%s')\n", pNode->pString);
	    pNode = pNode->pNext;
	}
    }
    if (nConnectingState != MPD_INVALID_CONNECTING_STATE)
    {
	switch (nConnectingState)
	{
	case MPD_INSERTING:
	    fprintf(fout, " nConnectingState: MPD_INSERTING\n");
	    break;
	case MPD_CONNECTING_LEFT:
	    fprintf(fout, " nConnectingState: MPD_CONNECTING_LEFT\n");
	    break;
	default:
	    fprintf(fout, " nConnectingState: invalid - %d\n", nConnectingState);
	    break;
	}
    }
    fprintf(fout, "}\n");
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

void CreateMPDRegistry()
{
    HKEY tkey;
    DWORD result;

    PUSH_FUNC("CreateMPDRegistry");

    // Open the root key
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &tkey, &result) != ERROR_SUCCESS)
    {
	int error = GetLastError();
	//err_printf("Unable to create the mpd registry key, error %d\n", error);
	POP_FUNC();
	return;
    }
    RegCloseKey(tkey);
    POP_FUNC();
}

bool ReadMPDRegistry(char *name, char *value, bool bPrintError /*= true*/ )
{
    HKEY tkey;
    DWORD len, result;

    PUSH_FUNC("ReadMPDRegistry");

    // Open the root key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, 
	KEY_READ,
	&tkey) != ERROR_SUCCESS)
    {
	if (bPrintError)
	    err_printf("Unable to open SOFTWARE\\MPICH\\MPD registry key, error %d\n", GetLastError());
	POP_FUNC();
	return false;
    }

    len = MAX_CMD_LENGTH;
    result = RegQueryValueEx(tkey, name, 0, NULL, (unsigned char *)value, &len);
    if (result != ERROR_SUCCESS)
    {
	if (bPrintError)
	    //warning_printf("Unable to read the mpd registry key '%s', error %d\n", name, GetLastError());
	    dbg_printf("Unable to read the mpd registry key '%s', error %d\n", name, GetLastError());
	RegCloseKey(tkey);
	POP_FUNC();
	return false;
    }

    RegCloseKey(tkey);
    POP_FUNC();
    return true;
}

void MPDRegistryToString(char *pszStr)
{
    HKEY tkey;
    DWORD len, result, len2;
    DWORD nMaxKeyLen, nMaxValueLen, nNumKeys, dwType;
    char *pszKey, *pszValue;
    char pszNum[10];

    PUSH_FUNC("MPDRegistryToString");

    // Open the root key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, KEY_ALL_ACCESS, &tkey) != ERROR_SUCCESS)
    {
	err_printf("Unable to open SOFTWARE\\MPICH\\MPD registry key, error %d\n", GetLastError());
	POP_FUNC();
	return;
    }

    result = RegQueryInfoKey(tkey, NULL, NULL, NULL, NULL, NULL, NULL, &nNumKeys, &nMaxKeyLen, &nMaxValueLen, NULL, NULL);
    if (result != ERROR_SUCCESS)
    {
	err_printf("Unable to query the mpd registry key, error %d\n", GetLastError());
	RegCloseKey(tkey);
	POP_FUNC();
	return;
    }

    pszKey = new char[nMaxKeyLen+1];
    pszValue = new char[nMaxValueLen+1];
    pszStr[0] = '\0';

    for (DWORD i=0; i<nNumKeys; i++)
    {
	len = nMaxKeyLen+1;
	len2 = nMaxValueLen+1;
	result = RegEnumValue(tkey, i, pszKey, &len, NULL, &dwType, (unsigned char *)pszValue, &len2); 
	if (result != ERROR_SUCCESS)
	{
	    err_printf("RegEnumKeyEx failed, error %d\n", result);
	}
	else
	{
	    //dbg_printf("key = %s, ", pszKey);
	    // Should I check here if key=phrase and not print out the value?
	    switch (dwType)
	    {
	    case REG_SZ:
		//dbg_printf("value = %s\n", pszValue);
		strcat(pszStr, pszKey);
		strcat(pszStr, "=");
		strcat(pszStr, pszValue);
		strcat(pszStr, "\n");
		break;
	    case REG_DWORD:
		//dbg_printf("value = %s\n", pszValue);
		strcat(pszStr, pszKey);
		strcat(pszStr, "=");
		result = *((DWORD*)pszValue);
		sprintf(pszNum, "%d\n", result);
		strcat(pszStr, pszNum);
		break;
	    default:
		err_printf("unhandled registry type: %d\n", dwType);
		break;
	    }
	}
    }

    delete pszKey;
    delete pszValue;

    RegCloseKey(tkey);
    POP_FUNC();
}

void WriteMPDRegistry(char *name, char *value)
{
    HKEY tkey;
    DWORD result;

    PUSH_FUNC("WriteMPDRegistry");

    // Open the root key
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &tkey, &result) != ERROR_SUCCESS)
    {
	POP_FUNC();
	return;
    }
    if (RegSetValueEx(tkey, name, 0, REG_SZ, (const unsigned char *)value, strlen(value)+1) != ERROR_SUCCESS)
    {
	if (stricmp(name, "phrase") == 0)
	{
	    err_printf("WriteMPDRegistry failed to write '%s: ***', error %d\n", name, GetLastError());
	}
	else
	{
	    err_printf("WriteMPDRegistry failed to write '%s:%s', error %d\n", name, value, GetLastError());
	}
    }
    else
    {
	/*
	if (stricmp(name, "phrase") == 0)
	{
	    dbg_printf("WriteMPDRegistry: %s = ***\n", name); // don't show the passphrase
	}
	else
	{
	    dbg_printf("WriteMPDRegistry: %s = %s\n", name, value);
	}
	*/
    }
    RegCloseKey(tkey);
    POP_FUNC();
}

void DeleteMPDRegistry(char *name)
{
    HKEY tkey;
    DWORD result;

    PUSH_FUNC("DeleteMPDRegistry");

    // Open the root key
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &tkey, &result) != ERROR_SUCCESS)
    {
	POP_FUNC();
	return;
    }
    // Delete the entry
    if (RegDeleteValue(tkey, name) != ERROR_SUCCESS)
    {
	err_printf("DeleteMPDRegistry failed to delete '%s', error %d\n", name, GetLastError());
    }
    /*
    else
    {
	dbg_printf("DeleteMPDRegistry: %s\n", name);
    }
    */
    RegCloseKey(tkey);
    POP_FUNC();
}

void ParseRegistry(bool bSetDefaults)
{
    HKEY tkey;
    DWORD result, len;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH];
    char port[10];
    char str[100];
    DWORD dwAccess;

    PUSH_FUNC("ParseRegistry");

    // Set the defaults.
    g_nPort = MPD_DEFAULT_PORT;
    gethostname(g_pszHost, 100);
    strcpy(g_pszLeftHost, g_pszHost);
    
    // Open the root key
    dwAccess =  (bSetDefaults) ? KEY_ALL_ACCESS : KEY_READ;
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, dwAccess, &tkey);
    if (result != ERROR_SUCCESS)
    {
	if (bSetDefaults)
	{
	    err_printf("Unable to open SOFTWARE\\MPICH\\MPD registry key, error %d\n", result);
	}
	POP_FUNC();
	return;
    }
    
    // Read the port
    len = 10;
    result = RegQueryValueEx(tkey, "port", 0, NULL, (unsigned char *)port, &len);
    if (result == ERROR_SUCCESS)
    {
	g_nPort = atoi(port);
    }
    else if (bSetDefaults)
    {
	sprintf(port, "%d", MPD_DEFAULT_PORT);
	RegSetValueEx(tkey, "port", 0, REG_SZ, (const unsigned char *)port, strlen(port)+1);
    }

    // Read the insert point
    len = 100;
    g_pszInsertHost[0] = '\0';
    RegQueryValueEx(tkey, INSERT1, 0, NULL, (unsigned char *)g_pszInsertHost, &len);

    // Read the second insert point
    len = 100;
    g_pszInsertHost2[0] = '\0';
    RegQueryValueEx(tkey, INSERT2, 0, NULL, (unsigned char *)g_pszInsertHost2, &len);

    // Read the temp directory
    len = MAX_PATH;
    result = RegQueryValueEx(tkey, "temp", 0, NULL, (unsigned char *)g_pszTempDir, &len);
    if (result != ERROR_SUCCESS && bSetDefaults)
	RegSetValueEx(tkey, "temp", 0, REG_SZ, (const unsigned char *)"C:\\", strlen("C:\\")+1);

    // Check to see if a passphrase has been set and set it to the default if necessary.
    len = MPD_PASSPHRASE_MAX_LENGTH;
    result = RegQueryValueEx(tkey, "phrase", 0, NULL, (unsigned char *)phrase, &len);
    if (result != ERROR_SUCCESS && bSetDefaults)
	RegSetValueEx(tkey, "phrase", 0, REG_SZ, (const unsigned char *)MPD_DEFAULT_PASSPHRASE, strlen(MPD_DEFAULT_PASSPHRASE)+1);

    len = 100;
    result = RegQueryValueEx(tkey, "SingleUser", 0, NULL, (unsigned char *)str, &len);
    if (result != ERROR_SUCCESS)
    {
	if (bSetDefaults)
	    RegSetValueEx(tkey, "SingleUser", 0, REG_SZ, (const unsigned char *)"no", 4);
	g_bSingleUser = false;
    }
    else
    {
	g_bSingleUser = (stricmp(str, "yes") == 0) ? true : false;
    }

    /*
    // Check to see what mode we are in.  The default is RshMode, not RingMode
    // Check rshmode first
    len = 100;
    result = RegQueryValueEx(tkey, "rshmode", 0, NULL, (unsigned char *)str, &len);
    if (result != ERROR_SUCCESS && bSetDefaults)
    {
	RegSetValueEx(tkey, "rshmode", 0, REG_SZ, (const unsigned char *)"yes", 4);
	g_bRshMode = true;
	// If there are no hosts names set, insert the local host name
	if (RegQueryValueEx(tkey, "hosts", 0, NULL, NULL, &len) != ERROR_SUCCESS)
	{
	    RegSetValueEx(tkey, "hosts", 0, REG_SZ, (const unsigned char *)g_pszHost, strlen(g_pszHost)+1);
	}
    }
    else
    {
	g_bRshMode = (stricmp(str, "yes") == 0) ? true : false;
    }
    // Check ringmode second
    len = 100;
    result = RegQueryValueEx(tkey, "ringmode", 0, NULL, (unsigned char *)str, &len);
    if (result != ERROR_SUCCESS && bSetDefaults)
    {
	RegSetValueEx(tkey, "ringmode", 0, REG_SZ, (const unsigned char *)"no", 4);
	g_bRingMode = false;
    }
    else
    {
	g_bRingMode = (stricmp(str, "yes") == 0) ? true : false;
    }
    */

    RegCloseKey(tkey);

    //dbg_printf("ParseRegistry: port %d, insert 1 '%s', insert 2 '%s', %s\n", g_nPort, g_pszInsertHost, g_pszInsertHost2, g_bRshMode ? "RshMode" : "RingMode");
    //dbg_printf("ParseRegistry: port %d, insert 1 '%s', insert 2 '%s'\n", g_nPort, g_pszInsertHost, g_pszInsertHost2);
    POP_FUNC();
}

bool GenAuthenticationStrings(char *append, char *crypted)
{
    int stamp;
    char *crypted_internal;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
    char phrase_internal[MPD_PASSPHRASE_MAX_LENGTH+1];

    PUSH_FUNC("GenAuthenticationStrings");

    srand(GetTickCount());
    stamp = rand();

    if (!ReadMPDRegistry("phrase", phrase))
    {
	POP_FUNC();
	return false;
    }

    sprintf(phrase_internal, "%s%d", phrase, stamp);
    sprintf(append, "%d", stamp);

    //dbg_printf("GenAuthenticationStrings: calling crypt on '%s'\n", phrase_internal);
    crypted_internal = crypt(phrase_internal, MPD_SALT_VALUE);
    strcpy(crypted, crypted_internal);

    memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH);
    memset(phrase_internal, 0, MPD_PASSPHRASE_MAX_LENGTH);

    POP_FUNC();
    return true;
}

void Clean()
{
    PUSH_FUNC("Clean");
    if (RegDeleteKey(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY) != ERROR_SUCCESS)
    {
	int error = GetLastError();
	if (error)
	    err_printf("Unable to remove the MPD registry key, error %d\n", error);
    }
    POP_FUNC();
}

bool ConnectToSelf()
{
    int bfdListen, nPort;
    char host[MAX_HOST_LENGTH];
    sockaddr addr;
    int len = sizeof(addr);

    PUSH_FUNC("ConnectToSelf");

    // Create a bogus listener
    if (beasy_create(&bfdListen, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_create failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    if (blisten(bfdListen, 5) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: blisten failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    beasy_get_sock_info(bfdListen, host, &nPort);

    // Create a new right context
    g_pRightContext = new MPD_Context;
    if (beasy_create(&g_pRightContext->bfd, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_create failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    if (beasy_connect(g_pRightContext->bfd, host, nPort) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_connect failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }

    // Create a new left context
    g_pLeftContext = new MPD_Context;
    g_pLeftContext->bfd = baccept(bfdListen, &addr, &len);
    if (g_pLeftContext->bfd == BFD_INVALID_SOCKET)
    {
	err_printf("ConnectToSelf: baccept failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }

    // Close the listener
    beasy_closesocket(bfdListen);

    // Initialize the new contexts
    strcpy(g_pRightContext->pszHost, host);
    strcpy(g_pszRightHost, host);
    g_pRightContext->nCurPos = 0;
    g_pRightContext->nState = MPD_IDLE;
    g_pRightContext->nLLState = MPD_READING_CMD;
    g_pRightContext->nType = MPD_RIGHT_SOCKET;
    g_pRightContext->pNext = g_pList;
    g_pList = g_pRightContext;
    DoReadSet(g_pRightContext->bfd);
    g_maxfds = BFD_MAX(g_pRightContext->bfd, g_maxfds);

    strcpy(g_pLeftContext->pszHost, host);
    strcpy(g_pszLeftHost, host);
    g_pLeftContext->nCurPos = 0;
    g_pLeftContext->nState = MPD_IDLE;
    g_pLeftContext->nLLState = MPD_READING_CMD;
    g_pLeftContext->nType = MPD_LEFT_SOCKET;
    g_pLeftContext->pNext = g_pList;
    g_pList = g_pLeftContext;
    DoReadSet(g_pLeftContext->bfd);
    g_maxfds = BFD_MAX(g_pLeftContext->bfd, g_maxfds);

    //dbg_printf("ConnectToSelf succeeded\n");
    POP_FUNC();
    return true;
}

bool Extract(bool bReConnect)
{
    // Note: Calling Extract with bReConnect = false will cause the mpd to exit
    // after the extract operation finishes.

    PUSH_FUNC("Extract");
    if (g_pLeftContext == NULL || g_pRightContext == NULL)
    {
	POP_FUNC();
	return false;
    }
    if (strcmp(g_pLeftContext->pszHost, g_pszHost) == 0)
    {
	if (strcmp(g_pRightContext->pszHost, g_pszHost))
	{
	    err_printf("invalid state: g_pszHost = %s, pszLeftHost = %s, pszRightHost = %s\n", g_pszHost, 
		g_pLeftContext->pszHost, g_pRightContext->pszHost);
	    POP_FUNC();
	    return false;
	}
	if (!bReConnect)
	{
	    RemoveContext(g_pLeftContext);
	    RemoveContext(g_pRightContext);
	    SignalExit();
	    SignalExit();
	}
	POP_FUNC();
	return true;
    }
    if (strcmp(g_pRightContext->pszHost, g_pszHost) == 0)
    {
	err_printf("invalid state: g_pszHost = %s, pszLeftHost = %s, pszRightHost = %s\n", g_pszHost, 
	    g_pLeftContext->pszHost, g_pRightContext->pszHost);
	POP_FUNC();
	return false;
    }

    EnqueueWrite(g_pRightContext, "connect left", MPD_WRITING_CONNECT_LEFT);
    if (bReConnect)
    {
	EnqueueWrite(g_pRightContext, g_pLeftContext->pszHost, MPD_WRITING_NEW_LEFT_HOST);
	EnqueueWrite(g_pLeftContext, "done", MPD_WRITING_DONE);
    }
    else
    {
	EnqueueWrite(g_pRightContext, g_pLeftContext->pszHost, MPD_WRITING_NEW_LEFT_HOST_EXIT);
	EnqueueWrite(g_pLeftContext, "done", MPD_WRITING_DONE_EXIT);
    }

    if (bReConnect)
    {
	if (!ConnectToSelf())
	{
	    err_printf("ConnectToSelf failed\n");
	}
    }

    POP_FUNC();
    return true;
}

bool InsertIntoRing(char *pszHost)
{
    // Note: When this function returns true, it means that the insert operation
    // has successfully started, not that it has completed.  It could fail later.

    PUSH_FUNC("InsertIntoRing");

    if (pszHost == NULL || pszHost[0] == '\0')
    {
	//dbg_printf("InsertIntoRing called with no host name\n");
	POP_FUNC();
	return false;
    }

    dbg_printf("InsertIntoRing: inserting at '%s', please wait\n", pszHost);

    MPD_Context *pContext = new MPD_Context;
    beasy_create(&pContext->bfd, ADDR_ANY, INADDR_ANY);
    if (beasy_connect(pContext->bfd, pszHost, g_nPort) == SOCKET_ERROR)
    {
	err_printf("InsertIntoRing: beasy_connect(%d, %s:%d) failed, error %d\n", bget_fd(pContext->bfd), pszHost, g_nPort, WSAGetLastError());
	delete pContext;
	return false;
    }
    strcpy(pContext->pszHost, pszHost);
    pContext->pNext = g_pList;
    g_pList = pContext;

    pContext->nType = MPD_SOCKET;
    pContext->nState = MPD_IDLE;
    pContext->nLLState = MPD_AUTHENTICATE_READING_APPEND;
    pContext->nConnectingState = MPD_INSERTING;
    DoReadSet(pContext->bfd);

    POP_FUNC();
    return true;
}

MPD_Context* GetContext(int bfd)
{
    MPD_Context *p = g_pList;
    while (p)
    {
	if (p->bfd == bfd)
	    return p;
	p = p->pNext;
    }
    return NULL;
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

void RemoveContext(MPD_Context *p)
{
    PUSH_FUNC("RemoveContext");
    MPD_Context *pTrailer = g_pList;
    if (p == NULL)
    {
	dbg_printf("empty context passed to RemoveContext\n");
	POP_FUNC();
	return;
    }
    
    if (p->bfd != BFD_INVALID_SOCKET)
    {
	BFD_CLR(p->bfd, &g_ReadSet);
	g_nActiveR--;
	BFD_CLR(p->bfd, &g_WriteSet);
	g_nActiveW--;
	beasy_closesocket(p->bfd);
    }

    if (p == g_pList)
	g_pList = g_pList->pNext;
    else
    {
	while (pTrailer && pTrailer->pNext != p)
	    pTrailer = pTrailer->pNext;
	if (pTrailer)
	    pTrailer->pNext = p->pNext;
    }

    if (p == g_pRightContext)
	g_pRightContext = NULL;

    if (p == g_pLeftContext)
	g_pLeftContext = NULL;

    //dbg_printf("RemoveContext: context[%d] deleted\n", p->bfd);
    delete p;
    POP_FUNC();
}

HANDLE g_hEnqueueMutex = CreateMutex(NULL, FALSE, NULL);

void EnqueueWrite(MPD_Context *p, char *pszStr, MPD_LowLevelState nState)
{
    // Note: You cannot have a concurrent reading and writing sessions in a
    // single context.  You can have multiple writes enqueued, but not multiple
    // reads.  There is a potential bug here if a command is being read when
    // EnqueueWrite is called.  This function will switch the state to WRITING
    // and the remaining data will not be read.  Then when the write is finished
    // the rest of the data will be read and it will be misunderstood.

    PUSH_FUNC("EnqueueWrite");

    if (p == NULL)
    {
	err_printf("attempting to enqueue '%s' into a NULL context\n", pszStr);
	POP_FUNC();
	return;
    }

    WaitForSingleObject(g_hEnqueueMutex, INFINITE);
    //dbg_printf("EnqueueWrite[%d]: '%s'\n", p->bfd, pszStr);
    if (p->nState == MPD_READING)
    {
	dbg_printf(":::DANGER WILL ROGERS::: switching from MPD_READING to MPD_WRITING on bfd[%d]\n", p->bfd);
    }
    if (p->nState != MPD_WRITING)
    {
	p->nCurPos = 0;
	p->nLLState = nState;
	strcpy(p->pszOut, pszStr);
	DoWriteSet(p->bfd);
	//dbg_printf("write enqueued directly into context\n");
    }
    else
    {
	if (p->pWriteList == NULL)
	{
	    p->pWriteList = new WriteNode(pszStr, nState);
	}
	else
	{
	    WriteNode *e = p->pWriteList;
	    while (e->pNext)
		e = e->pNext;
	    e->pNext = new WriteNode(pszStr, nState);
	}
	//dbg_printf("write enqueued into pWriteList\n");
    }
    p->nState = MPD_WRITING;
    ReleaseMutex(g_hEnqueueMutex);
    POP_FUNC();
}

void DequeueWrite(MPD_Context *p)
{
    PUSH_FUNC("DequeueWrite");
    //dbg_printf("DequeueWrite[%d]: %s\n", p->bfd, p->pszOut);
    if (p->pWriteList == NULL)
    {
	//dbg_printf("bfd[%d] state: MPD_IDLE, llstate: MPD_READING_CMD\n", p->bfd);
	p->nLLState = MPD_READING_CMD;
	p->nState = MPD_IDLE;
	BFD_CLR(p->bfd, &g_WriteSet);
	g_nActiveW--;
	//dbg_printf("bfd %d removed from write set\n", p->bfd);
	POP_FUNC();
	return;
    }

    WriteNode *e = p->pWriteList;
    p->pWriteList = p->pWriteList->pNext;

    p->nCurPos = 0;
    p->nState = MPD_WRITING;
    p->nLLState = e->nState;
    strcpy(p->pszOut, e->pString);
    //dbg_printf("bfd[%d] currently set to write '%s'\n", p->bfd, p->pszOut);

    delete e;
    POP_FUNC();
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
	    strcpy(g_pszRightHost, p->pszHost);
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
	strcpy(g_pszRightHost, p->pszHost);
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
	    strcat(phrase_internal, p->pszIn);
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
		sprintf(pszStr, "left %s", g_pszHost);
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
		sprintf(pszStr, "right %s", g_pszHost);
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
	    strcpy(p->pszHost, &p->pszIn[5]);
	}
	else if (strnicmp(p->pszIn, "right ", 6) == 0)
	{
	    dbg_printf("bfd[%d] nType: MPD_RIGHT_SOCKET\n", p->bfd);
	    p->nType = MPD_RIGHT_SOCKET;
	    p->nLLState= MPD_READING_CMD;
	    strcpy(p->pszHost, &p->pszIn[6]);
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

// Run ///////////////////////////////////////////////////////////////////////
//
//
int Run()
{
    sockaddr addr;
    struct linger linger;
    int len;
    int bfd;
    BOOL b;
    int n;
    MPD_Context *p, *e, *pNext;
    bfd_set read_set, write_set;
    int ret_val;
    int nRunCount = 0;
    int error;
    
    PUSH_FUNC("Run");

    //dbg_printf("Run started\n");
    while(true)
    {
	read_set = g_ReadSet;
	write_set = g_WriteSet;

	//printSet("ReadSet:\n", &read_set);
	//printSet("WriteSet:\n", &write_set);
	nRunCount++;
	n = bselect(g_maxfds, &read_set, &write_set, NULL, NULL);
	if (n == SOCKET_ERROR)
	{
	    error = WSAGetLastError();
	    err_printf("Run: bselect failed, error %d\n", error);
	    POP_FUNC();
	    return RUN_RESTART;
	}
	if (n == 0)
	{
	    err_printf("Run: bselect returned zero sockets available\n");
	    POP_FUNC();
	    return RUN_RESTART;
	}

	p = g_pList;
	while (n)
	{
	    if (p)
	    {
		if (p->nState != MPD_INVALID)
		{
		    if (BFD_ISSET(p->bfd, &read_set))
		    {
			//dbg_printf("Run[%d]: bfd[%d] readable\n", nRunCount, p->bfd);
			switch(p->nType)
			{
			case MPD_SOCKET:
			    //dbg_printf("read on generic mpd socket %d\n", p->bfd);
			case MPD_LEFT_SOCKET:
			case MPD_RIGHT_SOCKET:
			case MPD_CONSOLE_SOCKET:
			    p->nState = MPD_READING;
			    ret_val = bread(p->bfd, &p->pszIn[p->nCurPos], 1);
			    if (ret_val == 1)
			    {
				while (ret_val == 1)
				{
				    if (p->pszIn[p->nCurPos] == '\0')
				    {
					//dbg_printf("read string '%s'\n", p->pszIn);
					p->nState = MPD_IDLE;
					p->nCurPos = 0;
					StringRead(p);
					/*
					// If writes were to be enqueued after reads, then I would need to check for
					// enqueued writes here.
					if (p->pWriteList)
					{
					    strcpy(p->pszOut, p->pWriteList->pString);
					    p->nLLState = p->pWriteList->nState;
					    p->nState = MPD_WRITING;
					    p->nCurPos = 0;
					    WriteNode *e = p->pWriteList;
					    p->pWriteList = p->pWriteList->pNext;
					    delete e;
					    DoWriteSet(p->bfd);
					}
					*/
					break;
				    }
				    else
				    {
					p->nCurPos++;
				    }
				    ret_val = bread(p->bfd, &p->pszIn[p->nCurPos], 1);
				}
			    }
			    else
			    {
				error = WSAGetLastError();
				char *pszSocket;
				switch (p->nType)
				{
				case MPD_LEFT_SOCKET:
				    pszSocket = "MPD_LEFT_SOCKET";
				    break;
				case MPD_RIGHT_SOCKET:
				    pszSocket = "MPD_RIGHT_SOCKET";
				    break;
				case MPD_CONSOLE_SOCKET:
				    pszSocket = "MPD_CONSOLE_SOCKET";
				    break;
				default:
				    pszSocket = "MPD_SOCKET";
				    break;
				}
				if (ret_val == SOCKET_ERROR)
				{
				    err_printf("Run: bread failed for %s, error %d\n", pszSocket, error);
				}
				else if (ret_val == 0)
				{
				    err_printf("Run: %s %d unexpectedly closed\n", pszSocket, p->bfd);
				}
				else
				{
				    err_printf("Run: bread on %s returned unknown value, %d\n", pszSocket, ret_val);
				}
				p->bDeleteMe = true;
				p->nState = MPD_INVALID;
			    }
			    break;
			case MPD_LISTENER:
			    //dbg_printf("listener[%d] accepting new connection\n", p->bfd);
			    len = sizeof(addr);
			    bfd = baccept(p->bfd, &addr, &len);
			    if (bfd == BFD_INVALID_SOCKET)
			    {
				int error = WSAGetLastError();
				err_printf("Run: baccept failed: %d\n", error);
				break;
			    }
			    dbg_printf("listener[%d] accepted new connection bfd[%d]\n", p->bfd, bfd);
			    linger.l_onoff = 1;
			    linger.l_linger = 60;
			    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
			    {
				int error = WSAGetLastError();
				err_printf("Run: bsetsockopt failed: %d\n", error);
				beasy_closesocket(bfd);
				break;
			    }
			    b = TRUE;
			    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
			    if ((g_nActiveR >= FD_SETSIZE - 3) || (g_nActiveW >= FD_SETSIZE - 3))
			    {
				// No more sockets available
				err_printf("Run: No sockets available, incoming connect rejected.\n");
				beasy_closesocket(bfd);
			    }
			    else
			    {
				e = new MPD_Context;
				e->bfd = bfd;
				e->nCurPos = 0;
				e->nState = MPD_IDLE;
				e->nLLState = MPD_AUTHENTICATE_WRITING_APPEND;
				e->nType = MPD_SOCKET;
				e->pNext = g_pList;
				g_pList = e;
				g_maxfds = BFD_MAX(bfd, g_maxfds);
				DoReadSet(bfd);
				DoWriteSet(bfd);
				if (!GenAuthenticationStrings(e->pszOut, e->pszCrypt))
				{
				    err_printf("Run: failed to generate the authentication strings\n");
				    RemoveContext(e);
				}
			    }
			    break;
			case MPD_SIGNALLER:
			    dbg_printf("read available on signal socket %d ... ", p->bfd);
			    ret_val = beasy_receive(p->bfd, p->pszIn, 1);

			    if (ret_val == SOCKET_ERROR)
			    {
				// If we lose the signaller, we cannot stop the service.
				// So, shut down here and abort.
				err_printf("Run: signaller socket failed, error %d\n", WSAGetLastError());
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_RESTART;
			    }

			    if (g_nSignalCount)
			    {
				dbg_printf("extracting\n");
				if (!Extract(false))
				{
				    // If we cannot start the extraction process, just exit
				    ShutdownAllProcesses();
				    AbortAllForwarders();
				    RemoveAllTmpFiles();
				    FinalizeDriveMaps();
				    warning_printf("mpd exiting without extracting itself from the ring\n");
				    POP_FUNC();
				    return RUN_EXIT;
				}
			    }
			    else
			    {
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_EXIT;
			    }
			    break;
			case MPD_RESELECTOR:
			    //dbg_printf("read available on re-selector socket %d\n", p->bfd);
			    if (beasy_receive(p->bfd, p->pszIn, 1) == SOCKET_ERROR)
			    {
				err_printf("Run: reselector socket failed, error %d\n", WSAGetLastError());
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_RESTART;
			    }
			    break;
			default:
			    err_printf("Run: Error, read available on socket %d of unknown type %d\n", p->bfd, p->nType);
			    break;
			}
			n--;
		    }
		    if (p->nState != MPD_INVALID && BFD_ISSET(p->bfd, &write_set))
		    {
			//dbg_printf("Run[%d]: bfd[%d] writeable\n", nRunCount, p->bfd);
			/*
			MPD_Context *pTemp = p->pNext;
			while (pTemp)
			{
			    if (pTemp->nState != MPD_INVALID && BFD_ISSET(pTemp->bfd, &write_set))
			    {
				dbg_printf("Run[%d]: bfd[%d] pending write '%s'\n", nRunCount, pTemp->bfd, pTemp->pszOut);
			    }
			    pTemp = pTemp->pNext;
			}
			*/
			switch(p->nType)
			{
			case MPD_SOCKET:
			    //dbg_printf("write on generic mpd socket %d\n", p->bfd);
			case MPD_LEFT_SOCKET:
			case MPD_RIGHT_SOCKET:
			case MPD_CONSOLE_SOCKET:
			    p->nState = MPD_WRITING;
			    ret_val = bwrite(p->bfd, &p->pszOut[p->nCurPos], strlen(&p->pszOut[p->nCurPos])+1);
			    if (ret_val > 0)
			    {
				p->nCurPos += ret_val;
				if (p->pszOut[p->nCurPos - 1] == '\0')
				{
				    //dbg_printf("wrote string '%s'\n", p->pszOut);
				    if (p->pWriteList == NULL)
					p->nState = MPD_IDLE;
				    p->nCurPos = 0;
				    StringWritten(p);
				}
			    }
			    else
			    {
				if (ret_val == SOCKET_ERROR)
				{
				    err_printf("Run: bwrite failed in MPD_SOCKET case: %d\n", WSAGetLastError());
				}
				else if (ret_val == 0)
				{
				    err_printf("Run: bwrite returned 0 bytes after being set for writing\n");
				}
			    }
			    break;
			case MPD_LISTENER:
			    err_printf("Run: Error, write available on listener socket %d\n", p->bfd);
			    break;
			case MPD_SIGNALLER:
			    err_printf("Run: Error, write available on signal socket %d\n", p->bfd);
			    break;
			default:
			    err_printf("Run: Error, write available on socket %d of unknown type %d\n", p->bfd, p->nType);
			    break;
			}
			n--;
		    }
		    else
		    {
			if (p->nState == MPD_INVALID && BFD_ISSET(p->bfd, &write_set))
			{
			    err_printf("Run: write available on invalid bfd[%d]\n", p->bfd);
			    n--;
			}
		    }
		}
		else
		{
		    if (BFD_ISSET(p->bfd, &read_set))
		    {
			err_printf("Run: read available on invalid bfd[%d]\n", p->bfd);
			n--;
		    }
		    if (BFD_ISSET(p->bfd, &write_set))
		    {
			err_printf("Run: write available on invalid bfd[%d]\n", p->bfd);
			n--;
		    }
		}
		pNext = p->pNext;
		if (p->bDeleteMe)
		    RemoveContext(p);
		p = pNext; // p may be deleted so we use pNext to access p->pNext
	    }
	    else
	    {
		err_printf("Run: n(%d) arbitrarily set to zero because p == NULL\n", n);
		n = 0;
		//Sleep(250);
	    }
	}
    }
    POP_FUNC();
    return RUN_EXIT;
}

static void GetPassword(char *question, char *account, char *password)
{
    PUSH_FUNC("GetPassword");

    if (question != NULL)
	printf(question);
    else
	printf("password for %s: ", account);
    fflush(stdout);
    
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwMode;
    if (!GetConsoleMode(hStdin, &dwMode))
	dwMode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, dwMode & ~ENABLE_ECHO_INPUT);
    gets(password);
    SetConsoleMode(hStdin, dwMode);
    
    printf("\n");
    fflush(stdout);

    POP_FUNC();
}


static bool GetNextHost(FILE *fin, char *pszHost)
{
    char buffer[1024] = "";
    char *pChar, *pChar2;

    while (fgets(buffer, 1024, fin))
    {
	pChar = buffer;
	
	// Advance over white space
	while (*pChar != '\0' && isspace(*pChar))
	    pChar++;
	if (*pChar == '#' || *pChar == '\0')
	    continue;
	
	// Trim trailing white space
	pChar2 = &buffer[strlen(buffer)-1];
	while (isspace(*pChar2) && (pChar >= pChar))
	{
	    *pChar2 = '\0';
	    pChar2--;
	}
	
	// If there is anything left on the line, consider it a host name
	if (strlen(pChar) > 0)
	{
	    // Copy the host name
	    pChar2 = pszHost;
	    while (*pChar != '\0' && !isspace(*pChar))
	    {
		*pChar2 = *pChar;
		pChar++;
		pChar2++;
	    }
	    *pChar2 = '\0';

	    return true;
	}
    }
    return false;
}

static bool ReadStringTimeout(int bfd, char *str, int timeout)
{
    int n;
    char *str_orig = str;

    PUSH_FUNC("ReadStringTimeout");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	n = 0;
	while (!n)
	{
	    n = beasy_receive_timeout(bfd, str, 1, timeout);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadStringTimeout failed, error %d\n", WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	    if (n == 0)
	    {
		POP_FUNC();
		return false;
	    }
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

void ConnectAndRestart(int *argc, char ***argv, char *host)
{
    int bfd;
    char str[CONSOLE_STR_LENGTH+1];
    char *result;
    int error;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
    int port = -1;
    bool bAskPwd;

    PUSH_FUNC("ConnectAndRestart");

    bsocket_init();
    GetOpt(*argc, *argv, "-port", &port);
    bAskPwd = GetOpt(*argc, *argv, "-getphrase");
    GetOpt(*argc, *argv, "-phrase", phrase);

    ParseRegistry(false);
    if (host == NULL || host[0] == '\0')
	host = g_pszHost;
    if (port == -1)
	port = g_nPort;
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("beasy_create failed: %d\n%s\n", error, str);
	fflush(stdout);
	POP_FUNC();
	return;
    }
    if (bAskPwd || !ReadMPDRegistry("phrase", phrase, false))
    {
	printf("please input the passphrase: ");fflush(stdout);
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwMode;
	if (!GetConsoleMode(hStdin, &dwMode))
		dwMode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin, dwMode & ~ENABLE_ECHO_INPUT);
	gets(phrase);
	SetConsoleMode(hStdin, dwMode);
	printf("\n");fflush(stdout);
    }
    printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect(bfd, host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("beasy_connect failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading challenge string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    strcat(phrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    memset(phrase, 0, strlen(phrase)); // zero out the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString of the encrypted response string failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("host authentication failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString('console') failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    printf("connected\n");fflush(stdout);

    // send restart request
    if (WriteString(bfd, "restart") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	fflush(stdout);
	POP_FUNC();
	return;
    }
    //printf("waiting for result\n");fflush(stdout);
    if (ReadStringTimeout(bfd, str, 10))
    {
	printf("%s\n", str);fflush(stdout);
    }

    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);

    bsocket_finalize();
    POP_FUNC();
}

// parseCommandLine ///////////////////////////////////////////////////////////
//
// check for command line parameters and set various flags 
//
void parseCommandLine (int *argc, char** argv[])
{
    PUSH_FUNC("parseCommandLine");

    /*
    // A little snipet of code to test the update feature.
    // Uncomment this code, compile, run mpd -update, then run mpd -loser to see if this new functionality exists
    if (GetOpt(*argc, *argv, "-loser"))
    {
	printf("you are a winner\n");
	ExitProcess(0);
    }
    //*/
    if (GetOpt(*argc, *argv, "-interact"))
    {
	interact = true;
    }
    if (GetOpt(*argc, *argv, "-remove") || GetOpt(*argc, *argv, "-unregserver") || GetOpt(*argc, *argv, "-uninstall"))
    {
	CmdRemoveService(TRUE);
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-install") || GetOpt(*argc, *argv, "-regserver"))
    {
	char account[100]="", password[100]="", phrase[MPD_PASSPHRASE_MAX_LENGTH+1]="", port[10]="";

	if (CmdRemoveService(FALSE) == FALSE)
	{
	    printf("Unable to remove the previous installation, install failed.\n");
	    ExitProcess(0);
	}
	
	bsocket_init();
	CreateMPDRegistry();
	if (GetOpt(*argc, *argv, "-phrase", phrase))
	    WriteMPDRegistry("phrase", phrase);
	if (GetOpt(*argc, *argv, "-getphrase"))
	{
	    GetPassword("passphrase for mpd: ", NULL, phrase);
	    WriteMPDRegistry("phrase", phrase);
	}
	if (GetOpt(*argc, *argv, "-port", port))
	    WriteMPDRegistry("port", port);
	if (GetOpt(*argc, *argv, "-account", account))
	{
	    if (!GetOpt(*argc, *argv, "-password", password))
		GetPassword(NULL, account, password);
	    WriteMPDRegistry("SingleUser", "yes");
	    ParseRegistry(true);
	    CmdInstallService(account, password);
	}
	else
	{
	    WriteMPDRegistry("SingleUser", "no");
	    ParseRegistry(true);
	    CmdInstallService(NULL, NULL);
	}
	bsocket_finalize();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-update"))
    {
	char account[100]="", password[100]="", phrase[MPD_PASSPHRASE_MAX_LENGTH+1]="", port[10]="";
	int nPort;
	char pszHost[MAX_HOST_LENGTH], pszHostFile[MAX_PATH];
	char pszFileName[MAX_PATH];

	if (!GetOpt(*argc, *argv, "-mpd", pszFileName))
	{
	    HMODULE hModule = GetModuleHandle(NULL);
	    if (!GetModuleFileName(hModule, pszFileName, MAX_PATH))
	    {
		printf("Please specify the location of the new mpd.exe with the -mpd option, (-mpd c:\\some\\path\\mpd.exe)\n");
		ExitProcess(0);
	    }
	    printf("updating mpd to '%s'\n", pszFileName);
	}
	if (!GetOpt(*argc, *argv, "-singleuser"))
	{
	    if (GetOpt(*argc, *argv, "-account", account))
	    {
		if (!GetOpt(*argc, *argv, "-password", password))
		    GetPassword(NULL, account, password);
	    }
	    else
	    {
		printf("Enter a user to connect to the remote machines as.\naccount: ");fflush(stdout);
		gets(account);
		GetPassword(NULL, account, password);
	    }
	}

	bsocket_init();
	nPort = MPD_DEFAULT_PORT;
	if (!ReadMPDRegistry("phrase", phrase, false))
	    strcpy(phrase, MPD_DEFAULT_PASSPHRASE);
	GetOpt(*argc, *argv, "-phrase", phrase);
	if (GetOpt(*argc, *argv, "-getphrase"))
	{
	    GetPassword("passphrase for mpd: ", NULL, phrase);
	}
	if (GetOpt(*argc, *argv, "-port", port))
	    nPort = atoi(port);
	if (GetOpt(*argc, *argv, "-hostfile", pszHostFile))
	{
	    FILE *fin = fopen(pszHostFile, "r");
	    if (fin == NULL)
	    {
		char pszStr[1024];
		Translate_Error(GetLastError(), pszStr);
		printf("Unable to open the host file '%s', %s\n", pszHostFile, pszStr);
		bsocket_finalize();
		ExitProcess(0);
	    }

	    while (GetNextHost(fin, pszHost))
	    {
		UpdateMPD(pszHost, account, password, nPort, phrase, pszFileName);
	    }
	    fclose(fin);
	}
	else
	{
	    if (!GetOpt(*argc, *argv, "-host", pszHost))
	    {
		printf("Enter the hostname where the mpd that you wish to update is running.\nhost: ");fflush(stdout);
		gets(pszHost);
	    }
	    UpdateMPD(pszHost, account, password, nPort, phrase, pszFileName);
	}

	bsocket_finalize();
	printf("Finished.\n");
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-iupdate"))
    {
	// This option is used internally by the update feature
	char pszOldFileName[MAX_PATH], pszNewFileName[MAX_PATH];
	char pszPid[10];
	if (GetOpt(*argc, *argv, "-pid", pszPid) && 
	    GetOpt(*argc, *argv, "-old", pszOldFileName) && 
	    GetOpt(*argc, *argv, "-new", pszNewFileName))
	{
	    UpdateMPD(pszOldFileName, pszNewFileName, atoi(pszPid));
	}
	ExitProcess(0);
    }
    char host[100];
    if (GetOpt(*argc, *argv, "-console", host))
    {
	char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
	int port = -1;
	GetOpt(*argc, *argv, "-port", &port);
	DoConsole(
	    host, port, 
	    GetOpt(*argc, *argv, "-getphrase"),
	    GetOpt(*argc, *argv, "-phrase", phrase) ? phrase : NULL);
	bsocket_finalize();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-console"))
    {
	char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
	int port = -1;
	GetOpt(*argc, *argv, "-port", &port);
	DoConsole(
	    NULL, port, 
	    GetOpt(*argc, *argv, "-getphrase"),
	    GetOpt(*argc, *argv, "-phrase", phrase) ? phrase : NULL);
	bsocket_finalize();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-start"))
    {
	CmdStartService();
	ExitProcess(0);
    }
    char pszFileName[MAX_PATH];
    if (GetOpt(*argc, *argv, "-startdelete", pszFileName))
    {
	// This option is used by the update feature to start the new service and delete the old one.
	CmdStartService();
	// Give the temporary mpd time to exit
	Sleep(1000);
	// Then delete it.
	DeleteFile(pszFileName);
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-stop"))
    {
	CmdStopService();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-restart", host))
    {
	ConnectAndRestart(argc, argv, host);
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-restart"))
    {
	CmdStopService();
	Sleep(1000);
	CmdStartService();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-clean"))
    {
	Clean();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-d"))
    {
	char account[100]="", password[100]="", phrase[MPD_PASSPHRASE_MAX_LENGTH+1]="", pszPort[10]="";
	char str_temp[10];
	bsocket_init();
	CreateMPDRegistry();
	if (GetOpt(*argc, *argv, "-phrase", phrase))
	    WriteMPDRegistry("phrase", phrase);
	if (GetOpt(*argc, *argv, "-getphrase"))
	{
	    GetPassword("passphrase for mpd: ", NULL, phrase);
	    WriteMPDRegistry("phrase", phrase);
	}
	if (GetOpt(*argc, *argv, "-port", pszPort))
	{
	    int g_nSavedPort = g_nPort;
	    g_nPort = atoi(pszPort);
	    if (g_nPort > 0)
	    {
		sprintf(pszPort, "%d", g_nPort);
		WriteMPDRegistry("port", pszPort);
		//printf("using port %d\n", g_nPort);
	    }
	    else
		g_nPort = g_nSavedPort;
	}
	g_bSingleUser = true;
	g_bStartAlone = GetOpt(*argc, *argv, "-startalone");
	if (ReadMPDRegistry("SingleUser", str_temp, false))
	{
	    if (stricmp(str_temp, "no") == 0)
	    {
		WriteMPDRegistry("RevertToMultiUser", "yes");
	    }
	}
	WriteMPDRegistry("SingleUser", "yes");

	ParseRegistry(true);
	CmdDebugService(*argc, *argv);
	bsocket_finalize();
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-v") || GetOpt(*argc, *argv, "-version"))
    {
	fprintf(stderr, "\nMPD - mpich daemon for Windows NT, version %d.%d.%d\n%s\n",
		    VERSION_RELEASE, VERSION_MAJOR, VERSION_MINOR, COPYRIGHT);
	ExitProcess(0);
    }
    if (GetOpt(*argc, *argv, "-h") || GetOpt(*argc, *argv, "-?") || GetOpt(*argc, *argv, "-help"))
    {
	fprintf(stderr, "\nMPD - mpich daemon for Windows NT, version %d.%d\n%s\n\n", VERSION_MAJOR, VERSION_MINOR, COPYRIGHT);
	fprintf(stderr, "Usage:\n  mpd [ -v -h -install -remove -console ]\n\nCommand line options:\n");
	fprintf(stderr, "  -install \t:install the service\n  -install -interact    :allows the mpd to interact with the desktop\n");
	fprintf(stderr, "  -remove\t:remove the service\n");
	fprintf(stderr, "  -v\t\t:display version\n");
	fprintf(stderr, "  -h\t\t:this help screen\n");
	fprintf(stderr, "  -console\t:start a console session with the mpd on the current host\n");
	fprintf(stderr, "  -console host [-port x] :start a console session with the mpd on 'host:port'\n");
	fprintf(stderr, "  -d\t\t:run the mpd from the console\n");
	ExitProcess(0);
    }
    POP_FUNC();
}

void StdinThread()
{
    char str[MAX_CMD_LENGTH];
    while (gets(str))
    {
	if (strcmp(str, "quit") == 0)
	{
	    if (ReadMPDRegistry("RevertToMultiUser", str, false))
	    {
		if (stricmp(str, "yes") == 0)
		    WriteMPDRegistry("SingleUser", "no");
		DeleteMPDRegistry("RevertToMultiUser");
	    }
	    ExitProcess(0);
	}
	if (strcmp(str, "stop") == 0)
	{
	    ServiceStop();
	}
	if (strcmp(str, "print") == 0)
	{
	    PrintState(stdout);
	}
    }
}

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)
{
    int run_retval = RUN_EXIT;
    HANDLE stdin_thread = NULL;
    PUSH_FUNC("ServiceStart");
    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
	POP_FUNC();
	return;
    }
    
    // Load the path to the service executable
    char szExe[1024];
    HMODULE hModule = GetModuleHandle(NULL);
    if (!GetModuleFileName(hModule, szExe, 1024)) 
	strcpy(szExe, "mpd.exe");
    WriteMPDRegistry("path", szExe);
    
    // Initialize
    dbs_init();

    if (!bDebug)
    {
	bsocket_init();
	ParseRegistry(false);
    }

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
	bsocket_finalize();
	dbs_finalize();
	POP_FUNC();
	return;
    }
    
    do
    {
	// Setup the listener
	if (g_nPort == 0)
	{
	    err_printf("the listening port cannot be zero.\n");
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(-1);
	}
	MPD_Context *pListenContext = new MPD_Context;
	if (beasy_create(&pListenContext->bfd, g_nPort, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create listen socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	blisten(pListenContext->bfd, 20);
	pListenContext->nCurPos = 0;
	pListenContext->nState = MPD_IDLE;
	pListenContext->nType = MPD_LISTENER;
	strcpy(pListenContext->pszHost, g_pszHost);
	pListenContext->pNext = g_pList;
	g_pList = pListenContext;
	beasy_get_ip(&g_nIP);
	beasy_get_ip_string(g_pszIP);
	
	// report the status to the service control manager.
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    return;
	}
	
	// Setup the signal socket
	if (beasy_create(&g_bfdSignal, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create signal socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	if (beasy_connect(g_bfdSignal, g_pszHost, g_nPort) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_connect signal socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	sockaddr addr;
	int addrlen = sizeof(addr);
	MPD_Context *pSignalContext = new MPD_Context;
	pSignalContext->bfd = baccept(pListenContext->bfd, &addr, &addrlen);
	if(pSignalContext->bfd == INVALID_SOCKET)
	{
	    int error = WSAGetLastError();
	    err_printf("ServiceStart: baccept failed: %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	pSignalContext->nCurPos = 0;
	pSignalContext->nState = MPD_IDLE;
	pSignalContext->nType = MPD_SIGNALLER;
	strcpy(pSignalContext->pszHost, g_pszHost);
	pSignalContext->pNext = g_pList;
	g_pList = pSignalContext;
	
	// Setup the ReSelect socket
	if (beasy_create(&g_bfdReSelect, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create reselect socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	if (beasy_connect(g_bfdReSelect, g_pszHost, g_nPort) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_connect reselect socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	addrlen = sizeof(addr);
	MPD_Context *pReSelectContext = new MPD_Context;
	pReSelectContext->bfd = baccept(pListenContext->bfd, &addr, &addrlen);
	if (pReSelectContext->bfd == INVALID_SOCKET)
	{
	    int error = WSAGetLastError();
	    err_printf("baccept failed: %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	pReSelectContext->nCurPos = 0;
	pReSelectContext->nState = MPD_IDLE;
	pReSelectContext->nType = MPD_RESELECTOR;
	strcpy(pReSelectContext->pszHost, g_pszHost);
	pReSelectContext->pNext = g_pList;
	g_pList = pReSelectContext;
	
	BFD_ZERO(&g_ReadSet);
	BFD_ZERO(&g_WriteSet);
	g_nActiveW = 0;
	g_nActiveR = 0;
	DoReadSet(pSignalContext->bfd);
	DoReadSet(pReSelectContext->bfd);
	DoReadSet(pListenContext->bfd);
	g_maxfds = BFD_MAX(pListenContext->bfd, pSignalContext->bfd);
	
	// report the status to the service control manager.
	if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0))
	{
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    return;
	}
	
	
	////////////////////////////////////////////////////////
	//
	// Service is now running, perform work until shutdown
	//
	
	AddInfoToMessageLog("MPICH_MPD Daemon service started.");
	
	if (stdin_thread == NULL)
	{
	    stdin_thread = CreateThread(
		NULL, 0,
		(LPTHREAD_START_ROUTINE)StdinThread,
		NULL, 0, NULL);
	}
	
	if (ConnectToSelf() == false)
	{
	    err_printf("ServiceStart: ConnectToSelf failed\n");
	    ExitProcess(0);
	}
	if (!g_bStartAlone)
	{
	    if (stricmp(g_pszHost, g_pszInsertHost))
	    {
		if (InsertIntoRing(g_pszInsertHost) == false)
		{
		    if (stricmp(g_pszHost, g_pszInsertHost2))
			InsertIntoRing(g_pszInsertHost2);
		}
	    }
	}
	
	run_retval = Run();
	if (run_retval == RUN_RESTART)
	{
	    warning_printf("Socket connections lost, restarting mpd.");
	}
	
	while (g_pList)
	    RemoveContext(g_pList);
	
    } while (run_retval == RUN_RESTART);
    
    TerminateThread(stdin_thread, 0);
    CloseHandle(stdin_thread);
    
    bsocket_finalize();
    dbs_finalize();
    AddInfoToMessageLog("MPICH_MPD Daemon service stopped.");

    SetEvent(g_hBombDiffuseEvent);
    if (g_hBombThread != NULL)
    {
	if (WaitForSingleObject(g_hBombThread, 5000) == WAIT_TIMEOUT)
	{
	    TerminateThread(g_hBombThread, 0);
	}
	CloseHandle(g_hBombThread);
    }
    CloseHandle(g_hBombDiffuseEvent);

    POP_FUNC();
}

void BombThread()
{
    if (WaitForSingleObject(g_hBombDiffuseEvent, 25000) == WAIT_TIMEOUT)
    {
	ExitProcess(-1);
    }
}

//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop()
{
    DWORD dwThreadID;
    char str[10] = "no";
    PUSH_FUNC("ServiceStop");
    if (ReadMPDRegistry("RevertToMultiUser", str, false))
    {
	if (stricmp(str, "yes") == 0)
	    WriteMPDRegistry("SingleUser", "no");
	DeleteMPDRegistry("RevertToMultiUser");
    }
    g_hBombThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BombThread, NULL, 0, &dwThreadID);
    if (beasy_send(g_bfdSignal, "x", 1) == SOCKET_ERROR)
    {
	err_printf("ServiceStop: beasy_send('x') failed, error %d\n", WSAGetLastError());
	ShutdownAllProcesses();
	RemoveAllTmpFiles();
	AbortAllForwarders();
	FinalizeDriveMaps();
	ExitProcess(0);
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
