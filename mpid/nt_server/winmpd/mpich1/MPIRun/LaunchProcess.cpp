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
#ifdef MPICH_OLD_ROOT_STARTUP
    char pszFileName[MAX_PATH];
#else
    char pszStartupDB[100];
#endif
    char pszStr[MAX_CMD_LENGTH+1];
    char pszIOE[10];
    
    //printf("connecting to %s:%d rank %d\n", arg->pszHost, nPort, arg->i);fflush(stdout);
    if ((error = ConnectToMPD(arg->pszHost, nPort, arg->pszPassPhrase, &bfd)) == 0)
    {
	if (arg->i == 0 && !g_bNoMPI)
	{
#ifdef MPICH_OLD_ROOT_STARTUP
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
	    if (strlen(arg->pszEnv) + strlen(pszFileName) + 13 >= MAX_CMD_LENGTH)
	    {
		printf("Warning: environment variables truncated.\n");
	    }
	    strncat(arg->pszEnv, "|MPICH_EXTRA=", MAX_CMD_LENGTH - 1 - strlen(arg->pszEnv));
	    strncat(arg->pszEnv, pszFileName, MAX_CMD_LENGTH - 1 - strlen(arg->pszEnv));
#else
	    sprintf(pszStr, "dbcreate");
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
	    if (!ReadString(bfd, pszStartupDB))
	    {
		printf("ERROR: ReadString failed to read the database name: error %d\n", WSAGetLastError());
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStartupDB, "FAIL ", 5) == 0)
	    {
		printf("Unable to create a database on '%s'\n%s", arg->pszHost, pszStartupDB);fflush(stdout);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    sprintf(pszStr, "|MPICH_EXTRA=mpd:%s:%d:%s", pszStartupDB, nPort, arg->pszPassPhrase);
	    strncat(arg->pszEnv, pszStr, MAX_CMD_LENGTH - 1 - strlen(arg->pszEnv));

	    if (g_bUseJobHost)
	    {
		PutJobInDatabase(arg);
	    }
#endif
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
		    strncpy(g_pForwardHost[arg->i].pszHost, arg->pszHost, MAX_HOST_LENGTH);
		    g_pForwardHost[arg->i].pszHost[MAX_HOST_LENGTH-1] = '\0';
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
	    char *pszEncoded;
	    while (pNode)
	    {
		/*
		if (strlen(arg->pszAccount))
		{
		    pszEncoded = EncodePassword(g_pszPassword);
		    sprintf(pszStr, "map drive=%c share=%s account=%s password=%s", 
			pNode->cDrive, pNode->pszShare, g_pszAccount, pszEncoded);
		    if (pszEncoded != NULL) free(pszEncoded);
		}
		else
		{
		    sprintf(pszStr, "map drive=%c share=%s", pNode->cDrive, pNode->pszShare);
		}
		*/
		pszEncoded = EncodePassword(g_pszPassword);
		sprintf(pszStr, "map drive=%c share=%s account=%s password=%s", 
		    pNode->cDrive, pNode->pszShare, g_pszAccount, pszEncoded);
		if (pszEncoded != NULL) free(pszEncoded);
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
	    char *pszEncoded;
	    pszEncoded = EncodePassword(arg->pszPassword);
	    if (strlen(arg->pszDir) > 0)
	    {
		if (_snprintf(pszStr, MAX_CMD_LENGTH, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d d='%s'", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, pszEncoded, 
		    pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir) < 0)
		{
		    printf("ERROR: command exceeds internal buffer size\n");
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    if (pszEncoded != NULL) free(pszEncoded);
		    return;
		}
	    }
	    else
	    {
		if (_snprintf(pszStr, MAX_CMD_LENGTH, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, pszEncoded, 
		    pszIOE, arg->pszIOHostPort, arg->i) < 0)
		{
		    printf("ERROR: command exceeds internal buffer size\n");
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    if (pszEncoded != NULL) free(pszEncoded);
		    return;
		}
	    }
	    if (pszEncoded != NULL) free(pszEncoded);
	}
	else
	{
	    if (strlen(arg->pszDir) > 0)
	    {
		if (_snprintf(pszStr, MAX_CMD_LENGTH, "launch h=%s c='%s' e='%s' %s=%s k=%d d='%s'",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, 
		    pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir) < 0)
		{
		    printf("ERROR: command exceeds internal buffer size\n");
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    return;
		}
	    }
	    else
	    {
		if (_snprintf(pszStr, MAX_CMD_LENGTH, "launch h=%s c='%s' e='%s' %s=%s k=%d",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, 
		    pszIOE, arg->pszIOHostPort, arg->i) < 0)
		{
		    printf("ERROR: command exceeds internal buffer size\n");
		    beasy_closesocket(bfd);
		    SetEvent(g_hAbortEvent);
		    delete arg;
		    return;
		}
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
#ifdef MPICH_OLD_ROOT_STARTUP
		    sprintf(pszStr, "deletetmpfile host=%s file=%s", arg->pszHost, pszFileName);
		    WriteString(bfd, pszStr);
		    ReadString(bfd, pszStr); // Ignore the result
#endif
		}
		else
		{
		    printf("Failed to launch process %d:\n'%s'\n%s\n", arg->i, arg->pszCmdLine, pszStr);fflush(stdout);
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
#ifdef MPICH_OLD_ROOT_STARTUP
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
#else
	    // barrier to let the root process do the put
	    sprintf(pszStr, "barrier name=%s count=2", arg->pszJobID);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write the barrier command: error %d", WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to read the result of the barrier command on '%s': error %d", arg->pszHost, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strncmp(pszStr, "SUCCESS", 8))
	    {
		printf("ERROR: barrier failed on '%s':\n%s", arg->pszHost, pszStr);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }

	    // after the barrier, the data is available so do the get
	    sprintf(pszStr, "dbget name=%s key=port", pszStartupDB);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write '%s': error %d", pszStr, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to get the root port: error %d", WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strncmp(pszStr, DBS_FAIL_STR, strlen(DBS_FAIL_STR)+1) == 0)
	    {
		printf("ERROR: Unable to get the root port:\n%s", pszStr);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }

	    // save the gotten data
	    g_nRootPort = atoi(pszStr);

	    // destroy the database since it is no longer necessary
	    sprintf(pszStr, "dbdestroy name=%s", pszStartupDB);
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
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: ReadString failed to read the result of dbdestroy: error %d\n", WSAGetLastError());
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStr, DBS_FAIL_STR, strlen(DBS_FAIL_STR)+1) == 0)
	    {
		printf("Unable to destroy the database '%s' on '%s'\n%s", pszStartupDB, arg->pszHost, pszStr);fflush(stdout);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(g_hAbortEvent);
		delete arg;
		return;
	    }
#endif
	}

	if (g_bUseJobHost)
	{
	    // Save this process's information to the job database
	    PutJobProcessInDatabase(arg, nPid);
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

