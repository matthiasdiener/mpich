#ifndef MPDIMPL_H
#define MPDIMPL_H

#include "mpd.h"
#include "mpdutil.h"
#include "bsocket.h"
#include <stdio.h>

#define USE_LINGER_SOCKOPT

// Debug and error macros

/*
void dbg_printf(char *str, ...);
void warning_printf(char *str, ...);
void err_printf(char *str, ...);
*/
/*
//#define dbg_printf(str) { printf str ; fflush(stdout); }
//#define dbg_printf(str) 
extern char g_pszDbgString[4096];
extern char g_pszDbgCheckString[100];
#define dbg_printf(str) { sprintf str ; \
			    if (GetStringOpt(g_pszDbgString, "p", g_pszDbgCheckString)) \
				printf("STRING HAS PASSWORD\n"); \
			    else printf(g_pszDbgString); }
#define err_printf(str) { \
    printf("--------------------\n"); \
    printf str ; \
    PRINT_STACK(); \
    printf("--------------------\n\n"); \
    fflush(stdout); }
*/
//#define ferr_printf(str) { FILE *fout; fout = fopen("error.out", "a"); fprintf str ; fclose(fout); }

#define FUNC_STR_LENGTH 20
extern __declspec(thread) char g_call_stack[100][FUNC_STR_LENGTH+1];
extern __declspec(thread) int g_call_index;
#define PUSH_FUNC(name) { \
    strncpy(g_call_stack[g_call_index], name, FUNC_STR_LENGTH); \
    g_call_stack[g_call_index][FUNC_STR_LENGTH] = '\0'; \
    g_call_index++; }
#define POP_FUNC() g_call_index--
//#define PRINT_STACK() { for (int i=0; i<g_call_index; i++) { printf("%s\n", g_call_stack[i]); } }

// Defines

#define INVALID_HOSTNAME    "nohost"
#define BLOCKING_TIMEOUT    2000
#define INSERT1		    "insert1"
#define INSERT2		    "insert2"
#define ACK_STRING	    "zzz"

// Enums and Structs

enum MPD_Type {
    MPD_SOCKET,
    MPD_LISTENER,
    MPD_SIGNALLER,
    MPD_RESELECTOR,
    MPD_LEFT_SOCKET,
    MPD_RIGHT_SOCKET,
    MPD_CONSOLE_SOCKET
};

enum MPD_State { 
    MPD_IDLE, 
    MPD_READING, 
    MPD_WRITING, 
    MPD_INVALID 
};

enum MPD_LowLevelState {
    MPD_WRITING_CMD,
    MPD_WRITING_LAUNCH_CMD,
    MPD_WRITING_LAUNCH_RESULT,
    MPD_WRITING_EXITCODE,
    MPD_WRITING_HOSTS_CMD,
    MPD_WRITING_KILL_CMD,
    MPD_WRITING_FIRST_EXITALL_CMD,
    MPD_WRITING_EXITALL_CMD,
    MPD_WRITING_HOSTS_RESULT,
    MPD_WRITING_RESULT,
    MPD_READING_CMD,
    MPD_READING_NEW_LEFT,
    MPD_WRITING_OLD_LEFT_HOST,
    MPD_WRITING_DONE_EXIT,
    MPD_WRITING_DONE,
    MPD_WRITING_NEW_LEFT,
    MPD_READING_LEFT_HOST,
    MPD_WRITING_CONNECT_LEFT,
    MPD_WRITING_NEW_LEFT_HOST_EXIT,
    MPD_WRITING_NEW_LEFT_HOST,
    MPD_READING_CONNECT_LEFT,
    MPD_READING_NEW_LEFT_HOST,
    MPD_WRITING_NEW_RIGHT,
    MPD_READING_NEW_RIGHT,
    MPD_AUTHENTICATE_READING_APPEND,
    MPD_AUTHENTICATE_WRITING_APPEND,
    MPD_AUTHENTICATE_READING_CRYPTED,
    MPD_AUTHENTICATE_WRITING_CRYPTED,
    MPD_AUTHENTICATE_READING_RESULT,
    MPD_AUTHENTICATE_WRITING_RESULT,
    MPD_AUTHENTICATED,
    MPD_INVALID_LOWLEVEL
};

enum MPD_ConnectingState {
    MPD_INSERTING,
    MPD_CONNECTING_LEFT,
    MPD_INVALID_CONNECTING_STATE
};

struct WriteNode
{
    WriteNode();
    WriteNode(char *p, MPD_LowLevelState n);
    ~WriteNode();
    char *pString;
    MPD_LowLevelState nState;
    WriteNode *pNext;
};

struct MPD_Context
{
    MPD_Context();
    void Print(FILE *fout);
    MPD_Type nType;
    int bfd;
    char pszHost[MAX_HOST_LENGTH];
    char pszIn[MAX_CMD_LENGTH];
    char pszOut[MAX_CMD_LENGTH];
    int nCurPos;
    MPD_State nState;
    MPD_LowLevelState nLLState;
    bool bDeleteMe;
    WriteNode *pWriteList;
    bool bPassChecked;
    char pszCrypt[14];
    MPD_ConnectingState nConnectingState;
    bool bFileInitCalled;
    char pszFileAccount[50];
    char pszFilePassword[50];
    MPD_Context *pNext;
};

struct RedirectSocketArg
{
    bool bReadisPipe;
    HANDLE hRead;
    int bfdRead;
    bool bWriteisPipe;
    HANDLE hWrite;
    int bfdWrite;
    HANDLE hProcess;
    DWORD dwPid;
    HANDLE hMutex;
    bool bFreeMutex;
    int nRank;
    char cType;
    HANDLE hOtherThread;
};

// Global variables
extern int g_nPort;
extern char g_pszHost[MAX_HOST_LENGTH];
extern char g_pszLeftHost[MAX_HOST_LENGTH];
extern char g_pszRightHost[MAX_HOST_LENGTH];
extern char g_pszInsertHost[MAX_HOST_LENGTH];
extern char g_pszInsertHost2[MAX_HOST_LENGTH];
extern char g_pszIP[25];
extern unsigned long g_nIP;
extern char g_pszTempDir[MAX_PATH];

extern MPD_Context *g_pList;
extern MPD_Context *g_pRightContext;
extern MPD_Context *g_pLeftContext;
extern bfd_set g_ReadSet, g_WriteSet;
extern int g_bfdSignal;
extern int g_bfdReSelect;
extern int g_maxfds;
extern int g_nSignalCount;
extern bool g_bExitAllRoot;
extern bool g_bSingleUser;

extern int g_nActiveW;
extern int g_nActiveR;
extern bool g_bStartAlone;
extern HANDLE g_hBombDiffuseEvent;
extern HANDLE g_hBombThread;

// Function prototypes
void ConnectAndRestart(int *argc, char ***argv, char *host);
void GetMPDVersion(char *str, int length);
void GetMPICHVersion(char *str, int length);
bool snprintf_update(char *&pszStr, int &length, char *pszFormat, ...);
void statMPD(char *pszParam, char *pszStr, int length);
void statLaunchList(char *pszOutput, int length);
void statProcessList(char *pszOutput, int length);
void statConfig(char *pszOutput, int length);
void statContext(char *pszOutput, int length);
void statTmp(char *pszOutput, int length);
void statBarrier(char *pszOutput, int length);
void statForwarders(char *pszOutput, int length);
void RedirectSocketThread(RedirectSocketArg *arg);
void RedirectLockedSocketThread(RedirectSocketArg *arg);
HANDLE BecomeUser(char *domainaccount, char *password, int *pnError);
void LoseTheUser(HANDLE hUser);
bool MapDrive(char *pszDrive, char *pszShare, char *pszAccount, char *pszPassword, char *pszError);
bool UnmapDrive(char *pszDrive, char *pszError);
void FinalizeDriveMaps();
int ConsoleGetExitCode(int nPid);
void SetBarrier(char *pszName, int nCount, int bfd);
void ConcatenateForwardersToString(char *pszStr);
int CreateIOForwarder(char *pszFwdHost, int nFwdPort);
void StopIOForwarder(int nPort, bool bWaitForEmpty = true);
void AbortAllForwarders();
void RemoveAllTmpFiles();
void GetDirectoryContents(int bfd, char *pszInputStr);
bool ReadString(int bfd, char *str);
bool ReadStringMax(int bfd, char *str, int max);
bool ReadStringTimeout(int bfd, char *str, int timeout);
int WriteString(int bfd, char *str);
//void UpdateMPD(char *pszHost, char *pszAccount, char *pszPassword, int nPort, char *pszPhrase, char *pszFileName);
void UpdateMPD(char *pszFileName);
void UpdateMPD(char *pszOldFileName, char *pszNewFileName, int nPid);
void RestartMPD();
void UpdateMPICH(char *pszFileName);
void UpdateMPICHd(char *pszFileName);
void ConcatenateProcessesToString(char *pszStr);
void GetNameKeyValue(char *str, char *name, char *key, char *value);
HANDLE LaunchProcess(char *cmd, char *env, char *dir, HANDLE *hIn, HANDLE *hOut, HANDLE *hErr, int *pdwPid, int *nError, char *pszError);
HANDLE LaunchProcessLogon(char *domainaccount, char *password, char *cmd, char *env, char *dir, HANDLE *hIn, HANDLE *hOut, HANDLE *hErr, int *pdwPid, int *nError, char *pszError);
void MPD_KillProcess(int nPid);
void ShutdownAllProcesses();
void SavePid(int nId, int nPid);
void SaveError(int nId, char *pszError);
void SaveExitCode(int nId, int nExitCode);
void DoReadSet(int bfd);
void DoWriteSet(int bfd);
void SignalExit();
MPD_Context* GetContext(int bfd);
void Launch(char *pszStr);
void HandleConsoleRead(MPD_Context *p);
void HandleConsoleWritten(MPD_Context *p);
void HandleLeftRead(MPD_Context *p);
void HandleLeftWritten(MPD_Context *p);
void StringRead(MPD_Context *p);
void StringWritten(MPD_Context *p);
bool ConnectToSelf();
bool InsertIntoRing(char *pszHost);

#define RUN_EXIT    0
#define RUN_RESTART 1
int Run();

void RemoveContext(MPD_Context *p);
bool Extract(bool bReConnect);
void EnqueueWrite(MPD_Context *p, char *pszStr, MPD_LowLevelState nState);
void DequeueWrite(MPD_Context *p);
void ResetSelect();
void CreateMPDRegistry();
void CleanMPDRegistry();
bool ReadMPDRegistry(char *name, char *value, bool bPrintError = true);
void WriteMPDRegistry(char *name, char *value);
void DeleteMPDRegistry(char *name);
void MPDRegistryToString(char *pszStr, int length);
void ParseRegistry(bool bSetDefaults);
void DoConsole(char *host, int port, bool bAskPwd, char *altphrase);
void PrintState(FILE *fout);

#if defined(__cplusplus)
extern "C" {
#endif
char *crypt(const char *buf,const char *salt);
#if defined(__cplusplus)
}
#endif

#endif
