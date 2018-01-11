#include "bsocket.h"
#include "mpdimpl.h"
#include <stdio.h>
#include "GetStringOpt.h"
#include "Translate_Error.h"

//#define CONSOLE_STR_LENGTH 10*MAX_CMD_LENGTH

bool ReadStringMax(int bfd, char *str, int max)
{
    int n;
    char *str_orig = str;
    int count = 0;

    PUSH_FUNC("ReadString");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	/*
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	}
	*/
	n = beasy_receive(bfd, str, 1);
	if (n == SOCKET_ERROR)
	{
	    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	    POP_FUNC();
	    return false;
	}
	if (n == 0)
	{
	    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
	    POP_FUNC();
	    return false;
	}
	count++;
	if (count == max && *str != '\0')
	{
	    *str = '\0';
	    // truncate, read and discard all further characters of the string
	    char ch;
	    do {
		/*
		n = 0;
		while (!n)
		{
		    n = bread(bfd, &ch, 1);
		    if (n == SOCKET_ERROR)
		    {
			err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
			POP_FUNC();
			return false;
		    }
		}
		*/
		n = beasy_receive(bfd, &ch, 1);
		if (n == SOCKET_ERROR)
		{
		    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		    POP_FUNC();
		    return false;
		}
		if (n == 0)
		{
		    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
		    POP_FUNC();
		    return false;
		}
	    } while (ch != '\0');
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

bool ReadString(int bfd, char *str)
{
    int n;
    char *str_orig = str;

    PUSH_FUNC("ReadString");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	/*
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	}
	*/
	n = beasy_receive(bfd, str, 1);
	if (n == SOCKET_ERROR)
	{
	    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	    POP_FUNC();
	    return false;
	}
	if (n == 0)
	{
	    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
	    POP_FUNC();
	    return false;
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

bool ReadStringTimeout(int bfd, char *str, int timeout)
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

int WriteString(int bfd, char *str)
{
    int ret_val;
    PUSH_FUNC("WriteString");
    if (strlen(str) >= MAX_CMD_LENGTH)
    {
	err_printf("WriteString: command too long, %d\n", strlen(str));
	POP_FUNC();
	return SOCKET_ERROR;
    }
    //dbg_printf("writing to %d, <%s>\n", bget_fd(bfd), str);
    ret_val = beasy_send(bfd, str, strlen(str)+1);
    POP_FUNC();
    return ret_val;
}

#define TRANSFER_BUFFER_SIZE 20*1024

bool PutFile(int bfd, char *pszInputStr)
{
    char pszFileName[MAX_PATH];
    char pszRemoteFileName[MAX_PATH];
    char pszReplace[10] = "yes";
    char pszCreateDir[10] = "yes";
    char pszStr[MAX_CMD_LENGTH];
    FILE *fin;
    int error;
    int nLength;

    // Parse the input string
    if (!GetStringOpt(pszInputStr, "local", pszFileName))
    {
	printf("Error: no local file name specified (local=filename).\n");
	return false;
    }

    if (!GetStringOpt(pszInputStr, "remote", pszRemoteFileName))
    {
	strcpy(pszRemoteFileName, pszFileName);
    }

    GetStringOpt(pszInputStr, "replace", pszReplace);
    GetStringOpt(pszInputStr, "createdir", pszCreateDir);

    // Open the file
    fin = fopen(pszFileName, "rb");
    if (fin == NULL)
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	printf("Unable to open local file:\nFile: '%s'\nError: %s\n", pszFileName, pszStr);
	return false;
    }

    // Get the size
    fseek(fin, 0, SEEK_END);
    nLength = ftell(fin);
    if (nLength == -1)
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	printf("Unable to determine the size of the local file:\nFile: '%s'\nError: %s\n", pszFileName, pszStr);
	return false;
    }

    // Rewind back to the beginning
    fseek(fin, 0, SEEK_SET);

    // Send the putfile command
    sprintf(pszStr, "putfile name=%s length=%d replace=%s createdir=%s", pszRemoteFileName, nLength, pszReplace, pszCreateDir);
    WriteString(bfd, pszStr);

    // Get the response
    ReadString(bfd, pszStr);
    if (strcmp(pszStr, "SEND") == 0)
    {
	// Send the data
	char pBuffer[TRANSFER_BUFFER_SIZE];
	int nNumRead;
	while (nLength)
	{
	    nNumRead = min(nLength, TRANSFER_BUFFER_SIZE);
	    nNumRead = fread(pBuffer, 1, nNumRead, fin);
	    if (nNumRead < 1)
	    {
		printf("fread failed, %d\n", ferror(fin));
		beasy_closesocket(bfd);
		ExitProcess(0);
	    }
	    beasy_send(bfd, pBuffer, nNumRead);
	    //printf("%d bytes sent\n", nNumRead);fflush(stdout);
	    nLength -= nNumRead;
	}

	ReadString(bfd, pszStr);
	//printf("%s\n", pszStr);
	if (strcmp(pszStr, "SUCCESS") == 0)
	{
	    fclose(fin);
	    return true;
	}
    }
    else
    {
	printf("%s\n", pszStr);
    }

    fclose(fin);
    return false;
}

static void GetFile(int bfd, char *pszInputStr)
{
    bool bReplace = true, bCreateDir = false;
    char pszFileName[MAX_PATH];
    char pszRemoteFileName[MAX_PATH];
    char pszStr[256];
    int nLength;
    FILE *fout;
    char pBuffer[TRANSFER_BUFFER_SIZE];
    int nNumRead;
    int nNumWritten;
    bool bLocal = true, bRemote = true;

    // Parse the string for parameters
    if (GetStringOpt(pszInputStr, "replace", pszStr))
    {
	bReplace = (stricmp(pszStr, "yes") == 0);
    }
    if (GetStringOpt(pszInputStr, "createdir", pszStr))
    {
	bCreateDir = (stricmp(pszStr, "yes") == 0);
    }
    bLocal = GetStringOpt(pszInputStr, "local", pszFileName);
    bRemote = GetStringOpt(pszInputStr, "remote", pszRemoteFileName);

    if (!bLocal && !bRemote)
    {
	printf("Error: no file name provided\n");
	return;
    }
    if (!bRemote)
	strcpy(pszRemoteFileName, pszFileName);
    if (!bLocal)
	strcpy(pszFileName, pszRemoteFileName);

    // Create the local file
    //dbg_printf("creating file '%s'\n", pszFileName);
    if (bCreateDir)
    {
	if (!TryCreateDir(pszFileName, pszStr))
	{
	    printf("Error: unable to create the directory, %s\n", pszStr);
	    return;
	}
    }

    if (!bReplace)
    {
	fout = fopen(pszFileName, "r");
	if (fout != NULL)
	{
	    printf("Error: file exists\n");
	    fclose(fout);
	    return;
	}
	fclose(fout);
    }

    fout = fopen(pszFileName, "wb");

    if (fout == NULL)
    {
	Translate_Error(GetLastError(), pszStr, "Error: Unable to open the file, ");
	printf("%s\n", pszStr);
	return;
    }

    // Send the getfile command
    sprintf(pszStr, "getfile name=%s", pszRemoteFileName);
    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
    {
	Translate_Error(WSAGetLastError(), pszStr, "Error: Writing getfile command failed, ");
	printf("%s\n", pszStr);
	fclose(fout);
	return;
    }

    if (!ReadString(bfd, pszStr))
    {
	printf("Error: failed to read the response from the getfile command.\n");
	fclose(fout);
	return;
    }

    nLength = atoi(pszStr);
    if (nLength == -1)
    {
	if (!ReadString(bfd, pszStr))
	{
	    printf("Error: failed to read the error message from the getfile command.\n");
	    fclose(fout);
	    return;
	}
	printf("Error: %s\n", pszStr);
	fclose(fout);
	return;
    }

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

    printf("SUCCESS\n");
}

static void GetDirectoryContents(int bfd, char *pszInputStr)
{
    int nFolders, nFiles;
    char pszStr[MAX_PATH], pszLength[50];
    int i;

    if (WriteString(bfd, pszInputStr) == SOCKET_ERROR)
    {
	printf("writing '%s' command failed\n", pszInputStr);
	return;
    }

    if (!ReadString(bfd, pszStr))
    {
	printf("Error: reading nFolders failed\n");
	return;
    }
    if (strnicmp(pszStr, "ERROR", 5) == 0)
    {
	printf("%s\n", pszStr);
	return;
    }
    nFolders = atoi(pszStr);

    //printf("Folders:\n");
    for (i=0; i<nFolders; i++)
    {
	if (!ReadString(bfd, pszStr))
	{
	    printf("Error: reading folder name failed\n");
	    return;
	}
	printf("            %s\n", pszStr);
    }

    if (!ReadString(bfd, pszStr))
    {
	printf("Error: reading nFiles failed\n");
	return;
    }
    nFiles = atoi(pszStr);

    //printf("Files:\n");
    for (i=0; i<nFiles; i++)
    {
	if (!ReadString(bfd, pszStr))
	{
	    printf("Error: reading file name failed\n");
	    return;
	}
	if (!ReadString(bfd, pszLength))
	{
	    printf("Error: reading file length failed\n");
	    return;
	}
	//printf("%s %s\n", pszStr, pszLength);
	printf("%11s %s\n", pszLength, pszStr);
    }
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

void DoConsole(char *host, int port, bool bAskPwd, char *altphrase)
{
    int bfd;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
    char str[CONSOLE_STR_LENGTH+1];
    char *result;
    int error;

    PUSH_FUNC("DoConsole");

    bsocket_init();
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
    if (altphrase != NULL)
    {
	strcpy(phrase, altphrase);
    }
    else if (bAskPwd || !ReadMPDRegistry("phrase", phrase, false))
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
    if (altphrase != NULL)
	memset(altphrase, 0, strlen(altphrase)); // zero out the passphrase
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
    while (gets(str))
    {
	if ((strnicmp(str, "launch ", 7) == 0) ||
	    (strnicmp(str, "getpid ", 7) == 0) || 
	    (strnicmp(str, "geterror ", 9) == 0) ||
	    (strnicmp(str, "getexitcode ", 12) == 0) ||
	    (strnicmp(str, "getexitcodewait ", 16) == 0) ||
	    (stricmp(str, "version") == 0) ||
	    (stricmp(str, "config") == 0) ||
	    (strnicmp(str, "dbput ", 6) == 0) ||
	    (strnicmp(str, "dbget ", 6) == 0) ||
	    (stricmp(str, "dbcreate") == 0) ||
	    (strnicmp(str, "dbcreate ", 9) == 0) ||
	    (strnicmp(str, "dbdestroy ", 10) == 0) ||
	    (strnicmp(str, "dbfirst ", 8) == 0) ||
	    (strnicmp(str, "dbnext ", 7) == 0) ||
	    (stricmp(str, "dbfirstdb") == 0) ||
	    (stricmp(str, "dbnextdb") == 0) ||
	    (strnicmp(str, "dbdelete ", 9) == 0) ||
	    (stricmp(str, "ps") == 0) ||
	    (stricmp(str, "forwarders") == 0) ||
	    (strnicmp(str, "createtmpfile ", 14) == 0) ||
	    (strnicmp(str, "deletetmpfile ", 14) == 0) ||
	    (strnicmp(str, "mpich1readint ", 14) == 0) ||
	    (strnicmp(str, "lget ", 5) == 0))
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadStringTimeout(bfd, str, 10))
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    else
	    {
		printf("timeout waiting for result to return.\n");fflush(stdout);
	    }
	}
	else if (strnicmp(str, "barrier ", 8) == 0)
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadString(bfd, str))
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    else
	    {
		printf("error waiting for result to return.\n");fflush(stdout);
	    }
	}
	else if (stricmp(str, "hosts") == 0)
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		Translate_Error(error, str);
		printf("writing hosts request failed, %d\n%s\n", error, str);fflush(stdout);
		break;
	    }
	    if (ReadStringTimeout(bfd, str, 10))
	    {
		char *p = strstr(str, "result=");
		if (p != NULL)
		{
		    printf("%s\n", &p[7]);fflush(stdout);
		}
		else
		{
		    printf("%s\n", str);fflush(stdout);
		}
	    }
	    else
	    {
		printf("timeout waiting for result to return\n");fflush(stdout);
	    }
	}
	else if (strnicmp(str, "next ", 5) == 0)
	{
	    int n = atoi(&str[5]);
	    if ((n < 1) || (n > 16384))
	    {
		printf("invalid number of hosts requested\n");fflush(stdout);
		continue;
	    }
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		Translate_Error(error, str);
		printf("writing 'next' command failed, %d\n%s\n", error, str);fflush(stdout);
		break;
	    }
	    for (int i=0; i<n; i++)
	    {
		if (!ReadString(bfd, str))
		{
		    printf("Error reading host name\n");
		    break;
		}
		printf("%s\n", str);
	    }
	    fflush(stdout);
	}
	else if (strnicmp(str, "getexitcodewaitmultiple ", 24) == 0)
	{
	    int n = 0;
	    char str2[100];
	    char *token = strtok(&str[24], ",");
	    while (token != NULL)
	    {
		sprintf(str2, "getexitcodewait %s", token);
		if (WriteString(bfd, str2) == SOCKET_ERROR)
		{
		    error = WSAGetLastError();
		    Translate_Error(error, str);
		    printf("writing 'getexitcodewaitmultiple' failed, %d\n%s\n", error, str);fflush(stdout);
		    n = 0;
		    break;
		}
		n++;
		token = strtok(NULL, ",");
	    }
	    for (int i=0; i<n; i++)
	    {
		if (!ReadString(bfd, str))
		{
		    error = WSAGetLastError();
		    Translate_Error(error, str);
		    printf("reading exitcode failed, %d\n%s\n", error, str);fflush(stdout);
		    break;
		}
		printf("%s\n", str);
	    }
	}
	else if ((stricmp(str, "extract") == 0) ||
	    (strnicmp(str, "freeprocess ", 12) == 0) ||
	    (strnicmp(str, "insert ", 7) == 0) ||
	    (strnicmp(str, "set ", 4) == 0) ||
	    (strnicmp(str, "lset ", 5) == 0) ||
	    (strnicmp(str, "ldelete ", 8) == 0) ||
	    (strnicmp(str, "update ", 7) == 0) ||
	    (strnicmp(str, "stopforwarder ", 14) == 0) ||
	    (stricmp(str, "killforwarders") == 0))
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' request failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	}
	else if ((stricmp(str, "exit") == 0) || 
	    (stricmp(str, "quit") == 0) || 
	    (stricmp(str, "done") == 0))
	{
	    break;
	}
	else if (stricmp(str, "shutdown") == 0)
	{
	    if (WriteString(bfd, "shutdown") == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		Translate_Error(error, str);
		printf("writing shutdown request failed, %d\n%s", error, str);fflush(stdout);
	    }
	    break;
	}
	else if ((stricmp(str, "exitall") == 0) || (stricmp(str, "shutdownall") == 0))
	{
	    if (WriteString(bfd, "exitall") == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing %s request failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
	    }
	    break;
	}
	else if ((strnicmp(str, "kill ", 5) == 0) || (stricmp(str, "killall") == 0))
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' request failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
	    }
	}
	else if (strnicmp(str, "fileinit ", 9) == 0)
	{
	    char pszPassword[100];
	    if (!GetStringOpt(str, "password", pszPassword))
	    {
		char pszAccount[100];
		if (!GetStringOpt(str, "account", pszAccount))
		{
		    printf("no account and password specified\n");
		    fflush(stdout);
		    break;
		}
		GetPassword(NULL, pszAccount, pszPassword);
		sprintf(str, "fileinit account=%s password=%s", pszAccount, pszPassword);
	    }
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' request failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	}
	/*
	else if ((strnicmp(str, "map ", 4) == 0) ||
	    	 (strnicmp(str, "unmap ", 6) == 0))
	{
	    char pszPassword[100];
	    if (!GetStringOpt(str, "password", pszPassword))
	    {
		char pszAccount[100];
		char pszStrTemp[200];
		if (!GetStringOpt(str, "account", pszAccount))
		{
		    printf("no account and password specified\n");
		    fflush(stdout);
		    break;
		}
		GetPassword(NULL, pszAccount, pszPassword);
		sprintf(pszStrTemp, " account=%s password=%s", pszAccount, pszPassword);
		strcat(str, pszStrTemp);
	    }
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing map command failed, %d\n", error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadStringTimeout(bfd, str, 30)) // logon requests can take a long time
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    else
	    {
		printf("timeout waiting for result to return.\n");fflush(stdout);
	    }
	}
	*/
	else if (strnicmp(str, "map ", 4) == 0)
	{
	    char pszPassword[100];
	    if (!GetStringOpt(str, "password", pszPassword))
	    {
		char pszAccount[100];
		char pszStrTemp[200];
		if (!GetStringOpt(str, "account", pszAccount))
		{
		    printf("no account and password specified\n");
		    fflush(stdout);
		    break;
		}
		GetPassword(NULL, pszAccount, pszPassword);
		sprintf(pszStrTemp, " account=%s password=%s", pszAccount, pszPassword);
		strcat(str, pszStrTemp);
	    }
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing map command failed, %d\n", error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadStringTimeout(bfd, str, 30)) // logon requests can take a long time
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    else
	    {
		printf("timeout waiting for result to return.\n");fflush(stdout);
	    }
	}
	else if (strnicmp(str, "unmap ", 6) == 0)
	{
	    char pszDrive[10];
	    if (!GetStringOpt(str, "drive", pszDrive))
	    {
		char pszStrTemp[40];
		sprintf(pszStrTemp, "unmap drive=%s", &str[6]);
		strcpy(str, pszStrTemp);
	    }
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing unmap command failed, %d\n", error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadStringTimeout(bfd, str, 10))
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    else
	    {
		printf("timeout waiting for result to return.\n");fflush(stdout);
	    }
	}
	else if (strnicmp(str, "putfile ", 8) == 0)
	{
	    if (PutFile(bfd, &str[8]))
	    {
		printf("SUCCESS\n");fflush(stdout);
	    }
	}
	else if (strnicmp(str, "getfile ", 8) == 0)
	{
	    GetFile(bfd, &str[8]);
	}
	else if (strnicmp(str, "getdir ", 7) == 0)
	{
	    GetDirectoryContents(bfd, str);
	}
	else if (stricmp(str, "restart") == 0)
	{
	    //printf("writing 'restart'\n");fflush(stdout);
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    //printf("waiting for result\n");fflush(stdout);
	    if (ReadStringTimeout(bfd, str, 10))
	    {
		printf("%s\n", str);fflush(stdout);
	    }
	    break;
	}
	else if (stricmp(str, "print") == 0)
	{
	    if (WriteString(bfd, str) == SOCKET_ERROR)
	    {
		error = WSAGetLastError();
		printf("writing '%s' failed, %d\n", str, error);
		Translate_Error(error, str);
		printf("%s\n", str);
		fflush(stdout);
		break;
	    }
	    if (ReadStringMax(bfd, str, CONSOLE_STR_LENGTH))
	    {
		printf("%s", str);fflush(stdout);
	    }
	    else
	    {
		printf("reading result failed\n");fflush(stdout);
		break;
	    }
	}
	else
	{
	    printf("unknown command\n");fflush(stdout);
	}
    }
    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);
    POP_FUNC();
}
