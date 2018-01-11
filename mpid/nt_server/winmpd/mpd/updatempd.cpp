#include "mpdimpl.h"
#include "Translate_Error.h"
#include "Service.h"

static int ConnectToMPD(char *host, int port, char *phrase, int *pbfd)
{
    int bfd;
    char str[256];
    char *result;
    int error;
    struct linger linger;
    BOOL b;

    if (host == NULL || host[0] == '\0' || port < 1 || phrase == NULL || pbfd == NULL)
	return -1;
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("beasy_create failed: %d\n", error);fflush(stdout);
	return error;
    }
    linger.l_onoff = 1;
    linger.l_linger = 60;
    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("bsetsockopt failed: %d\n", error);
	beasy_closesocket(bfd);
	return error;
    }
    b = TRUE;
    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
    //printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect(bfd, host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("beasy_connect failed: error %d\n", error);fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading prepend string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    strcat(phrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    //memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH); // zero out the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("WriteString of the crypt string failed: error %d\n", error);fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("authentication request failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("WriteString failed after attempting passphrase authentication: error %d\n", error);fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    //printf("connected to %s\n", host);fflush(stdout);
    *pbfd = bfd;
    return 0;
}

void UpdateMPD(char *pszHost, char *pszAccount, char *pszPassword, int nPort, char *pszPhrase, char *pszFileName)
{
    int bfd;
    char pszStr[MAX_CMD_LENGTH];
    char pszTempFileName[MAX_PATH];
    int ret_val;

    // Connect to the mpd on pszHost
    ret_val = ConnectToMPD(pszHost, nPort, pszPhrase, &bfd);
    if (ret_val != 0)
    {
	printf("Unable to connect to %s\n", pszHost);fflush(stdout);
	return;
    }

    // Initialize the file operations
    sprintf(pszStr, "fileinit account=%s password=%s", pszAccount, pszPassword);

    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	printf("Writing the fileinit command failed, error %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return;
    }

    // Create a temporary file
    sprintf(pszStr, "createtmpfile host=%s delete=no", pszHost);
    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	printf("Writing the createtempfile command failed on %s, error %d\n", pszHost, WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return;
    }
    if (!ReadString(bfd, pszTempFileName))
    {
	printf("Reading the temporary file name failed\n");fflush(stdout);
	beasy_closesocket(bfd);
	return;
    }

    // Copy the new mpd executable into this temporary file
    sprintf(pszStr, "local=%s remote=%s", pszFileName, pszTempFileName);
    if (!PutFile(bfd, pszStr))
    {
	sprintf(pszStr, "deletetmpfile host=%s file=%s", pszHost, pszTempFileName);
	WriteString(bfd, pszStr);
	ReadString(bfd, pszStr);
	WriteString(bfd, "done");
	printf("Unable to put the new mpd file on host %s", pszHost);fflush(stdout);
	beasy_closesocket(bfd);
	return;
    }

    // Update the mpd
    sprintf(pszStr, "update %s", pszTempFileName);
    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	printf("Writing the update command failed, error %d", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return;
    }

    // Close the console session
    WriteString(bfd, "done");
    beasy_closesocket(bfd);
}

void UpdateMPD(char *pszFileName)
{
    int error;
    char szExe[1024], szExeCopy[1024];
    char pszStr[2048];

    if (!GetModuleFileName(NULL, szExe, 1024))
    {
	Translate_Error(GetLastError(), pszStr);
	dbg_printf("GetModuleFileName failed.\nError: %s\n", pszStr);
	return;
    }

    strcpy(szExeCopy, szExe);
    strcpy(&szExeCopy[strlen(szExeCopy)-4], "2.exe");

    dbg_printf("copying '%s' to '%s'\n", szExe, szExeCopy);
    if (!CopyFile(szExe, szExeCopy, FALSE))
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	err_printf("Unable to copy '%s' to '%s'\nError: %s\n", szExe, szExeCopy, pszStr);
	return;
    }

    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    GetStartupInfo(&sInfo);

    sprintf(pszStr, "\"%s\" -iupdate -old \"%s\" -new \"%s\" -pid %d", szExeCopy, szExe, pszFileName, GetCurrentProcessId());
    //dbg_printf("launching '%s'\n", pszStr);

    if (!CreateProcess(NULL, 
	    pszStr,
	    NULL, NULL, FALSE, 
	    DETACHED_PROCESS,
	    NULL, NULL, 
	    &sInfo, &pInfo))
    {
	error = GetLastError();
	err_printf("CreateProcess failed for '%s'\n", pszStr);
	Translate_Error(error, pszStr);
	err_printf("Error: %s\n", pszStr);
	return;
    }
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
}

void UpdateMPD(char *pszOldFileName, char *pszNewFileName, int nPid)
{
    int error;
    char pszStr[1024];
    HANDLE hMPD;
    
    //FILE *fout;
    //fout = fopen("c:\\temp\\update.out", "w");

    // Open a handle to the running service
    hMPD = OpenProcess(SYNCHRONIZE, FALSE, nPid);
    if (hMPD == NULL)
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	//fprintf(fout, "OpenProcess(%d) failed, %s\n", nPid, pszStr);
	//fclose(fout);
	CloseHandle(hMPD);
	return;
    }

    // Stop the service
    CmdStopService();

    // Wait for the service to exit
    if (WaitForSingleObject(hMPD, 20000) != WAIT_OBJECT_0)
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	//fprintf(fout, "Waiting for the old mpd to stop failed. %s\n", pszStr);
	//fclose(fout);
	CloseHandle(hMPD);
	return;
    }

    CloseHandle(hMPD);

    // Delete the old service
    if (!DeleteFile(pszOldFileName))
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	//fprintf(fout, "DeleteFile(%s) failed.\nError: %s\n", pszOldFileName, pszStr);
	//fclose(fout);
	return;
    }

    // Move the new service to the old service's spot
    if (!MoveFile(pszNewFileName, pszOldFileName))
    {
	error = GetLastError();
	Translate_Error(error, pszStr);
	//fprintf(fout, "MoveFile(%s,%s) failed.\nError: %s\n", pszNewFileName, pszOldFileName, pszStr);
	//fclose(fout);
	return;
    }

    char szExe[1024];

    if (!GetModuleFileName(NULL, szExe, 1024))
    {
	Translate_Error(GetLastError(), pszStr);
	//fprintf(fout, "GetModuleFileName failed.\nError: %s\n", pszStr);
	return;
    }

    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    GetStartupInfo(&sInfo);

    sprintf(pszStr, "\"%s\" -startdelete \"%s\"", pszOldFileName, szExe);
    //fprintf(fout, "launching '%s'\n", pszStr);

    if (!CreateProcess(NULL, 
	    pszStr,
	    NULL, NULL, FALSE, 
	    DETACHED_PROCESS,
	    NULL, NULL, 
	    &sInfo, &pInfo))
    {
	error = GetLastError();
	//fprintf(fout, "CreateProcess failed for '%s'\n", pszStr);
	Translate_Error(error, pszStr);
	//fprintf(fout, "Error: %s\n", pszStr);
	//fclose(fout);
	return;
    }
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);

    //fclose(fout);
}

void RestartMPD()
{
    int error;
    char szExe[1024];
    char pszStr[2048];

    if (!GetModuleFileName(NULL, szExe, 1024))
    {
	Translate_Error(GetLastError(), pszStr);
	dbg_printf("GetModuleFileName failed.\nError: %s\n", pszStr);
	return;
    }

    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    GetStartupInfo(&sInfo);

    sprintf(pszStr, "\"%s\" -restart", szExe);
    //dbg_printf("launching '%s'\n", pszStr);

    if (!CreateProcess(NULL, 
	    pszStr,
	    NULL, NULL, FALSE, 
	    DETACHED_PROCESS,
	    NULL, NULL, 
	    &sInfo, &pInfo))
    {
	error = GetLastError();
	err_printf("CreateProcess failed for '%s'\n", pszStr);
	Translate_Error(error, pszStr);
	err_printf("Error: %s\n", pszStr);
	return;
    }
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
}
