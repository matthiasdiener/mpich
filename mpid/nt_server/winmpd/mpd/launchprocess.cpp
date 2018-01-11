#include "mpdimpl.h"

// Function name	: SetEnvironmentVariables
// Description	    : 
// Return type		: void 
// Argument         : char *bEnv
static void SetEnvironmentVariables(char *bEnv)
{
    char name[MAX_PATH]="", value[MAX_PATH]="";
    char *pChar;
    
    pChar = name;
    while (*bEnv != '\0')
    {
	if (*bEnv == '=')
	{
	    *pChar = '\0';
	    pChar = value;
	}
	else
	{
	    if (*bEnv == '|')
	    {
		*pChar = '\0';
		pChar = name;
		SetEnvironmentVariable(name, value);
	    }
	    else
	    {
		*pChar = *bEnv;
		pChar++;
	    }
	}
	bEnv++;
    }
    *pChar = '\0';
    SetEnvironmentVariable(name, value);
}

// Function name	: RemoveEnvironmentVariables
// Description	    : 
// Return type		: void 
// Argument         : char *bEnv
static void RemoveEnvironmentVariables(char *bEnv)
{
    char name[MAX_PATH]="", value[MAX_PATH]="";
    char *pChar;
    
    pChar = name;
    while (*bEnv != '\0')
    {
	if (*bEnv == '=')
	{
	    *pChar = '\0';
	    pChar = value;
	}
	else
	{
	    if (*bEnv == '|')
	    {
		*pChar = '\0';
		pChar = name;
		SetEnvironmentVariable(name, NULL);
	    }
	    else
	    {
		*pChar = *bEnv;
		pChar++;
	    }
	}
	bEnv++;
    }
    *pChar = '\0';
    SetEnvironmentVariable(name, NULL);
}

// Function name	: LaunchProcess
// Description	    : 
// Return type		: HANDLE 
HANDLE LaunchProcess(char *cmd, char *env, char *dir, HANDLE *hIn, HANDLE *hOut, HANDLE *hErr, int *pdwPid, int *nError, char *pszError)
{
    HANDLE hStdin, hStdout, hStderr;
    HANDLE hPipeStdinR=NULL, hPipeStdinW=NULL;
    HANDLE hPipeStdoutR=NULL, hPipeStdoutW=NULL;
    HANDLE hPipeStderrR=NULL, hPipeStderrW=NULL;
    STARTUPINFO saInfo;
    PROCESS_INFORMATION psInfo;
    void *pEnv=NULL;
    char tSavedPath[MAX_PATH] = ".";
    HANDLE hRetVal = INVALID_HANDLE_VALUE;
    
    // Launching of the client processes must be synchronized because
    // stdin,out,err are redirected for the entire process, not just this thread.
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdLaunchMutex");
    
    WaitForSingleObject(hMutex, INFINITE);
    
    // Don't handle errors, just let the process die.
    // In the future this will be configurable to allow various debugging options.
    DWORD dwOriginalErrorMode;
    dwOriginalErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    
    // Save stdin, stdout, and stderr
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStderr = GetStdHandle(STD_ERROR_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE  || hStderr == INVALID_HANDLE_VALUE)
    {
	*nError = GetLastError();
	strcpy(pszError, "GetStdHandle failed, ");
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return INVALID_HANDLE_VALUE;
    }
    
    // Set the security attributes to allow handles to be inherited
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.lpSecurityDescriptor = NULL;
    saAttr.bInheritHandle = TRUE;
    
    // Create pipes for stdin, stdout, and stderr
    if (!CreatePipe(&hPipeStdinR, &hPipeStdinW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    if (!CreatePipe(&hPipeStdoutR, &hPipeStdoutW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    if (!CreatePipe(&hPipeStderrR, &hPipeStderrW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    
    // Make the ends of the pipes that this process will use not inheritable
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStdinW, GetCurrentProcess(), hIn, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	goto CLEANUP;
    }
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStdoutR, GetCurrentProcess(), hOut, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	goto CLEANUP;
    }
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStderrR, GetCurrentProcess(), hErr, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	goto CLEANUP;
    }
    
    // Set stdin, stdout, and stderr to the ends of the pipe the created process will use
    if (!SetStdHandle(STD_INPUT_HANDLE, hPipeStdinR))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	goto CLEANUP;
    }
    if (!SetStdHandle(STD_OUTPUT_HANDLE, hPipeStdoutW))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	goto RESTORE_CLEANUP;
    }
    if (!SetStdHandle(STD_ERROR_HANDLE, hPipeStderrW))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	goto RESTORE_CLEANUP;
    }
    
    // Create the process
    memset(&saInfo, 0, sizeof(STARTUPINFO));
    saInfo.cb = sizeof(STARTUPINFO);
    saInfo.hStdError = hPipeStderrW;
    saInfo.hStdInput = hPipeStdinR;
    saInfo.hStdOutput = hPipeStdoutW;
    saInfo.dwFlags = STARTF_USESTDHANDLES;
    //saInfo.lpDesktop = TEXT("WinSta0\\Default");
    
    SetEnvironmentVariables(env);
    pEnv = GetEnvironmentStrings();
    
    GetCurrentDirectory(MAX_PATH, tSavedPath);
    SetCurrentDirectory(dir);
    
    if (CreateProcess(
	NULL,
	cmd,
	NULL, NULL, TRUE,
	//DETACHED_PROCESS | IDLE_PRIORITY_CLASS, 
	CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS,
	//CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
	//DETACHED_PROCESS | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
	//CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_SUSPENDED, 
	pEnv,
	NULL,
	&saInfo, &psInfo))
    {
	hRetVal = psInfo.hProcess;
	*pdwPid = psInfo.dwProcessId;
	
	CloseHandle(psInfo.hThread);
    }
    else
    {
	*nError = GetLastError();
	strcpy(pszError, "CreateProcess failed, ");
    }
    
    FreeEnvironmentStrings((TCHAR*)pEnv);
    SetCurrentDirectory(tSavedPath);
    RemoveEnvironmentVariables(env);
    
RESTORE_CLEANUP:
    // Restore stdin, stdout, stderr
    SetStdHandle(STD_INPUT_HANDLE, hStdin);
    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStderr);
    
CLEANUP:
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    CloseHandle(hPipeStdinR);
    CloseHandle(hPipeStdoutW);
    CloseHandle(hPipeStderrW);
    
    SetErrorMode(dwOriginalErrorMode);

    return hRetVal;
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
	*pCh2 = L'\0';
    }
    else
    {
	strcpy(tAccount, DomainAccount);
	tDomain[0] = '\0';
    }
}

HANDLE LaunchProcessLogon(char *domainaccount, char *password, char *cmd, char *env, char *dir, HANDLE *hIn, HANDLE *hOut, HANDLE *hErr, int *pdwPid, int *nError, char *pszError)
{
    HANDLE hStdin, hStdout, hStderr;
    HANDLE hPipeStdinR=NULL, hPipeStdinW=NULL;
    HANDLE hPipeStdoutR=NULL, hPipeStdoutW=NULL;
    HANDLE hPipeStderrR=NULL, hPipeStderrW=NULL;
    STARTUPINFO saInfo;
    PROCESS_INFORMATION psInfo;
    void *pEnv=NULL;
    char tSavedPath[MAX_PATH] = ".";
    HANDLE hRetVal = INVALID_HANDLE_VALUE;
    char account[100], domain[100] = "", *pszDomain = NULL;
    HANDLE hUser;
    int num_tries;
    int error;
    
    // Launching of the client processes must be synchronized because
    // stdin,out,err are redirected for the entire process, not just this thread.
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdLaunchMutex");
    
    WaitForSingleObject(hMutex, INFINITE);
    
    // Don't handle errors, just let the process die.
    // In the future this will be configurable to allow various debugging options.
    DWORD dwOriginalErrorMode;
    dwOriginalErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    
    // Save stdin, stdout, and stderr
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStderr = GetStdHandle(STD_ERROR_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE  || hStderr == INVALID_HANDLE_VALUE)
    {
	*nError = GetLastError();
	strcpy(pszError, "GetStdHandle failed, ");
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	SetErrorMode(dwOriginalErrorMode);
	return INVALID_HANDLE_VALUE;
    }
    
    // Set the security attributes to allow handles to be inherited
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.lpSecurityDescriptor = NULL;
    saAttr.bInheritHandle = TRUE;
    
    // Create pipes for stdin, stdout, and stderr
    if (!CreatePipe(&hPipeStdinR, &hPipeStdinW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    if (!CreatePipe(&hPipeStdoutR, &hPipeStdoutW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    if (!CreatePipe(&hPipeStderrR, &hPipeStderrW, &saAttr, 0))
    {
	*nError = GetLastError();
	strcpy(pszError, "CreatePipe failed, ");
	goto CLEANUP;
    }
    
    // Make the ends of the pipes that this process will use not inheritable
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStdinW, GetCurrentProcess(), hIn, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	goto CLEANUP;
    }
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStdoutR, GetCurrentProcess(), hOut, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	CloseHandle(*hIn);
	goto CLEANUP;
    }
    if (!DuplicateHandle(GetCurrentProcess(), hPipeStderrR, GetCurrentProcess(), hErr, 
	0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
    {
	*nError = GetLastError();
	strcpy(pszError, "DuplicateHandle failed, ");
	CloseHandle(*hIn);
	CloseHandle(*hOut);
	goto CLEANUP;
    }
    
    // Set stdin, stdout, and stderr to the ends of the pipe the created process will use
    if (!SetStdHandle(STD_INPUT_HANDLE, hPipeStdinR))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	CloseHandle(*hIn);
	CloseHandle(*hOut);
	CloseHandle(*hErr);
	goto CLEANUP;
    }
    if (!SetStdHandle(STD_OUTPUT_HANDLE, hPipeStdoutW))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	CloseHandle(*hIn);
	CloseHandle(*hOut);
	CloseHandle(*hErr);
	goto RESTORE_CLEANUP;
    }
    if (!SetStdHandle(STD_ERROR_HANDLE, hPipeStderrW))
    {
	*nError = GetLastError();
	strcpy(pszError, "SetStdHandle failed, ");
	CloseHandle(*hIn);
	CloseHandle(*hOut);
	CloseHandle(*hErr);
	goto RESTORE_CLEANUP;
    }
    
    // Create the process
    memset(&saInfo, 0, sizeof(STARTUPINFO));
    saInfo.cb         = sizeof(STARTUPINFO);
    saInfo.hStdInput  = hPipeStdinR;
    saInfo.hStdOutput = hPipeStdoutW;
    saInfo.hStdError  = hPipeStderrW;
    saInfo.dwFlags    = STARTF_USESTDHANDLES;
    //saInfo.lpDesktop = TEXT("WinSta0\\Default");
    
    SetEnvironmentVariables(env);
    pEnv = GetEnvironmentStrings();
    
    ParseAccountDomain(domainaccount, account, domain);
    if (strlen(domain) < 1)
	pszDomain = NULL;
    else
	pszDomain = domain;

    /*
    if (!LogonUser(
	account,
	pszDomain, 
	password,
	LOGON32_LOGON_INTERACTIVE, 
	LOGON32_PROVIDER_DEFAULT, 
	&hUser))
    {
	*nError = GetLastError();
	FreeEnvironmentStrings((TCHAR*)pEnv);
	SetCurrentDirectory(tSavedPath);
	RemoveEnvironmentVariables(env);
	CloseHandle(*hIn);
	CloseHandle(*hOut);
	CloseHandle(*hErr);
	goto RESTORE_CLEANUP;
    }
    */
    num_tries = 3;
    while (!LogonUser(
	account,
	pszDomain, 
	password,
	LOGON32_LOGON_INTERACTIVE, 
	//LOGON32_LOGON_BATCH,
	LOGON32_PROVIDER_DEFAULT, 
	&hUser))
    {
	error = GetLastError();
	if (error == ERROR_NO_LOGON_SERVERS)
	{
	    if (num_tries)
		Sleep(250);
	    else
	    {
		*nError = error;
		strcpy(pszError, "LogonUser failed, ");
		FreeEnvironmentStrings((TCHAR*)pEnv);
		SetCurrentDirectory(tSavedPath);
		RemoveEnvironmentVariables(env);
		CloseHandle(*hIn);
		CloseHandle(*hOut);
		CloseHandle(*hErr);
		goto RESTORE_CLEANUP;
	    }
	    num_tries--;
	}
	else
	{
	    *nError = error;
	    strcpy(pszError, "LogonUser failed, ");
	    FreeEnvironmentStrings((TCHAR*)pEnv);
	    SetCurrentDirectory(tSavedPath);
	    RemoveEnvironmentVariables(env);
	    CloseHandle(*hIn);
	    CloseHandle(*hOut);
	    CloseHandle(*hErr);
	    goto RESTORE_CLEANUP;
	}
    }

    if (ImpersonateLoggedOnUser(hUser))
    {
	GetCurrentDirectory(MAX_PATH, tSavedPath);
	SetCurrentDirectory(dir);

	/*
	if (CreateProcessAsUser(
	    hUser,
	    NULL,
	    cmd,
	    NULL, NULL, TRUE,
	    //DETACHED_PROCESS | IDLE_PRIORITY_CLASS, 
	    CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS,
	    //CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
	    //DETACHED_PROCESS | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
	    //CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_SUSPENDED, 
	    pEnv,
	    NULL,
	    &saInfo, &psInfo))
	{
	    hRetVal = psInfo.hProcess;
	    *pdwPid = psInfo.dwProcessId;
	    
	    CloseHandle(psInfo.hThread);
	}
	else
	{
	    *nError = GetLastError();
	    strcpy(pszError, "CreateProcessAsUser failed, ");
	}
	RevertToSelf();
	*/
	num_tries = 4;
	do
	{
	    if (CreateProcessAsUser(
		hUser,
		NULL,
		cmd,
		NULL, NULL, TRUE,
		//DETACHED_PROCESS | IDLE_PRIORITY_CLASS, 
		CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS,
		//CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
		//DETACHED_PROCESS | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
		//CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_SUSPENDED, 
		pEnv,
		NULL,
		&saInfo, &psInfo))
	    {
		hRetVal = psInfo.hProcess;
		*pdwPid = psInfo.dwProcessId;
		
		CloseHandle(psInfo.hThread);
		num_tries = 0;
	    }
	    else
	    {
		error = GetLastError();
		if (error == ERROR_REQ_NOT_ACCEP)
		{
		    Sleep(1000);
		    num_tries--;
		    if (num_tries == 0)
		    {
			*nError = error;
			strcpy(pszError, "CreateProcessAsUser failed, ");
		    }
		}
		else
		{
		    *nError = error;
		    strcpy(pszError, "CreateProcessAsUser failed, ");
		    num_tries = 0;
		}
	    }
	} while (num_tries);
	RevertToSelf();
    }
    else
    {
	*nError = GetLastError();
	strcpy(pszError, "ImpersonateLoggedOnUser failed, ");
    }
    CloseHandle(hUser);
    
    FreeEnvironmentStrings((TCHAR*)pEnv);
    SetCurrentDirectory(tSavedPath);
    RemoveEnvironmentVariables(env);
    
RESTORE_CLEANUP:
    // Restore stdin, stdout, stderr
    SetStdHandle(STD_INPUT_HANDLE, hStdin);
    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStderr);
    
CLEANUP:
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    CloseHandle(hPipeStdinR);
    CloseHandle(hPipeStdoutW);
    CloseHandle(hPipeStderrW);

    SetErrorMode(dwOriginalErrorMode);

    return hRetVal;
}
