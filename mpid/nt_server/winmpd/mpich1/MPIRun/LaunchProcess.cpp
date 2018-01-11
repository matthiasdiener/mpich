#include "LaunchProcess.h"
#include <stdio.h>
#include "global.h"
#include "..\Common\MPIJobDefs.h"
#include "Translate_Error.h"
#include "bsocket.h"
#include "mpdutil.h"
#include "mpd.h"
#include "RedirectIO.h"
#include <stdlib.h>

// Function name	: LaunchProcess
// Description	    : 
// Return type		: void 
// Argument         : LaunchProcessArg *arg
void MPIRunLaunchProcess(MPIRunLaunchProcessArg *arg)
{
    DWORD length = 100;
    HANDLE hRIThread = NULL;
    long error;
    int nPid;
    int nPort = MPD_DEFAULT_PORT;
    int bfd, launchid;
    char pszFileName[MAX_PATH];
    char pszStr[MAX_CMD_LENGTH+1];
    char pszIOE[10];
    
    //printf("connecting to %s:%d rank %d\n", arg->pszHost, nPort, arg->i);fflush(stdout);
    if ((error = ConnectToMPD(arg->pszHost, nPort, arg->pszPassPhrase, &bfd)) == 0)
    {
	if (arg->i == 0 && !g_bNoMPI)
	{
	    // write "createtmpfile host = host"
	    sprintf(pszStr, "createtmpfile host=%s", arg->pszHost);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    // read result
	    if (!ReadString(bfd, pszFileName))
	    {
		printf("ERROR: ReadString failed to read the temporary file name\n");
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszFileName, "FAIL ", 5) == 0)
	    {
		printf("Unable to create a temporary file on '%s'\n%s", arg->pszHost, pszFileName);fflush(stdout);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    strcat(arg->pszEnv, "|MPICH_EXTRA=");
	    strcat(arg->pszEnv, pszFileName);
	}
	
	if (arg->i == 0)
	    strcpy(pszIOE, "012"); // only redirect stdin to the root process
	else
	    strcpy(pszIOE, "12");

	if (g_nNproc > FORWARD_NPROC_THRESHOLD)
	{
	    if (arg->i > 0)
	    {
		while (g_pForwardHost[(arg->i - 1)/2].nPort == 0)
		    Sleep(100);
		sprintf(arg->pszIOHostPort, "%s:%d", g_pForwardHost[(arg->i - 1)/2].pszHost, g_pForwardHost[(arg->i - 1)/2].nPort);
		if (g_nNproc/2 > arg->i)
		{
		    strcpy(g_pForwardHost[arg->i].pszHost, arg->pszHost);
		    sprintf(pszStr, "createforwarder host=%s forward=%s", arg->pszHost, arg->pszIOHostPort);
		    WriteString(bfd, pszStr);
		    ReadString(bfd, pszStr);
		    int nTempPort = atoi(pszStr);
		    if (nTempPort == -1)
		    {
			// If creating the forwarder fails, redirect output to the root instead
			g_pForwardHost[arg->i] = g_pForwardHost[0];
		    }
		    else
			g_pForwardHost[arg->i].nPort = nTempPort;
		    //printf("forwarder %s:%d\n", g_pForwardHost[arg->i].pszHost, g_pForwardHost[arg->i].nPort);fflush(stdout);
		}
	    }
	}

	if (g_pDriveMapList && !g_bNoDriveMapping)
	{
	    MapDriveNode *pNode = g_pDriveMapList;
	    while (pNode)
	    {
		/*
		if (strlen(arg->pszAccount))
		{
		    sprintf(pszStr, "map drive=%c share=%s account=%s password=%s", 
			pNode->cDrive, pNode->pszShare, g_pszAccount, g_pszPassword);
		}
		else
		{
		    sprintf(pszStr, "map drive=%c share=%s", pNode->cDrive, pNode->pszShare);
		}
		*/
		sprintf(pszStr, "map drive=%c share=%s account=%s password=%s", 
		    pNode->cDrive, pNode->pszShare, g_pszAccount, g_pszPassword);
		if (WriteString(bfd, pszStr) == SOCKET_ERROR)
		{
		    printf("ERROR: Unable to send map command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    return;
		}
		if (!ReadString(bfd, pszStr))
		{
		    printf("ERROR: Unable to read the result of a map command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    return;
		}
		if (stricmp(pszStr, "SUCCESS"))
		{
		    printf("ERROR: Unable to map %c: to %s on %s\r\n%s", pNode->cDrive, pNode->pszShare, arg->pszHost, pszStr);
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    return;
		}
		pNode = pNode->pNext;
	    }
	}

	// LaunchProcess
	//printf("launching on %s, %s\n", arg->pszHost, arg->pszCmdLine);fflush(stdout);
	if (arg->bLogon)
	{
	    if (strlen(arg->pszDir) > 0)
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d d=%s", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, arg->pszPassword, 
		    pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir);
	    }
	    else
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, arg->pszPassword, 
		    pszIOE, arg->pszIOHostPort, arg->i);
	    }
	}
	else
	{
	    if (strlen(arg->pszDir) > 0)
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' %s=%s k=%d d=%s",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, 
		    pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir);
	    }
	    else
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' %s=%s k=%d",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, 
		    pszIOE, arg->pszIOHostPort, arg->i);
	    }
	}
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to send launch command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    beasy_closesocket(bfd);
	    SetEvent(g_hAbortEvent);
	    delete arg;
	    return;
	}
	if (!ReadString(bfd, pszStr))
	{
	    printf("ERROR: Unable to read the result of the launch command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    beasy_closesocket(bfd);
	    SetEvent(g_hAbortEvent);
	    delete arg;
	    return;
	}
	launchid = atoi(pszStr);
	// save the launch id, get the pid
	sprintf(pszStr, "getpid %d", launchid);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to send getpid command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    beasy_closesocket(bfd);
	    SetEvent(g_hAbortEvent);
	    delete arg;
	    return;
	}
	if (!ReadString(bfd, pszStr))
	{
	    printf("ERROR: Unable to read the result of the getpid command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    beasy_closesocket(bfd);
	    SetEvent(g_hAbortEvent);
	    delete arg;
	    return;
	}
	nPid = atoi(pszStr);
	if (nPid == -1)
	{
	    sprintf(pszStr, "geterror %d", launchid);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to send geterror command after an unsuccessful launch on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to read the result of the geterror command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strcmp(pszStr, "ERROR_SUCCESS"))
	    {
		if (arg->i == 0 && !g_bNoMPI)
		{
		    printf("Failed to launch the root process:\n%s\n%s\n", arg->pszCmdLine, pszStr);fflush(stdout);
		    // Cleanup
		    sprintf(pszStr, "deletetmpfile host=%s file=%s", arg->pszHost, pszFileName);
		    WriteString(bfd, pszStr);
		    ReadString(bfd, pszStr); // Ignore the result
		}
		else
		{
		    printf("Failed to launch process %d:\n%s\n%s\n", arg->i, arg->pszCmdLine, pszStr);fflush(stdout);
		}

		if (!UnmapDrives(bfd))
		{
		    printf("Drive unmappings failed on %s\n", arg->pszHost);
		}

		sprintf(pszStr, "freeprocess %d", launchid);
		WriteString(bfd, pszStr);
		WriteString(bfd, "done");
		//beasy_closesocket(bfd);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	}
	
	// Get the port number and redirect input to the first process
	if (arg->i == 0 && !g_bNoMPI)
	{
	    // write mpich1readint
	    sprintf(pszStr, "mpich1readint host=%s pid=%d file=%s", arg->pszHost, nPid, pszFileName);
	    //printf("%s\n", pszStr);fflush(stdout);
	    WriteString(bfd, pszStr);
	    // read result
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to read the result of the mpich1readint command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStr, "FAIL", 4) == 0)
	    {
		printf("root process failed to write port to file\n");
		if (strlen(pszStr) > 4)
		    printf("%s\n", pszStr);
		fflush(stdout);
		sprintf(pszStr, "kill host=%s pid=%d", arg->pszHost, nPid);
		WriteString(bfd, pszStr);
		if (!UnmapDrives(bfd))
		{
		    printf("Drive unmappings failed on %s\n", arg->pszHost);
		}
		sprintf(pszStr, "freeprocess %d", launchid);
		WriteString(bfd, pszStr);
		sprintf(pszStr, "deletetmpfile host=%s file=%s", arg->pszHost, pszFileName);
		WriteString(bfd, pszStr);
		ReadString(bfd, pszStr); // Ignore the result
		WriteString(bfd, "done");
		//beasy_closesocket(bfd);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    g_nRootPort = atoi(pszStr);
	}
	
	// Wait for the process to exit
	sprintf(pszStr, "getexitcodewait %d", launchid);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("Error: Unable to send a getexitcodewait command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());fflush(stdout);
	    beasy_closesocket(bfd);
	    SetEvent(g_hAbortEvent);
	    delete arg;
	    //if (arg->i == 0)
		//ExitProcess(0);
	    return;
	}

	int i = InterlockedIncrement(&g_nNumProcessSockets) - 1;
	g_pProcessSocket[i] = bfd;
	g_pProcessLaunchId[i] = launchid;
	g_pLaunchIdToRank[i] = arg->i;

	//printf("(P:%d)", launchid);fflush(stdout);
    }
    else
    {
	printf("MPIRunLaunchProcess: Connect to %s failed, error %d\n", arg->pszHost, error);fflush(stdout);
	//ExitProcess(0);
	SetEvent(g_hAbortEvent);
	delete arg;
	return;
    }
    
    memset(arg->pszPassword, 0, 100);
    delete arg;
}

