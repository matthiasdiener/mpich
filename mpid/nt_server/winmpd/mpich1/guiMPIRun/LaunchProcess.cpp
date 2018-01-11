#include "stdafx.h"
#include "guiMPIRun.h"
#include "guiMPIRunDoc.h"
#include "guiMPIRunView.h"
#include "LaunchProcess.h"
#include <stdio.h>
#include "global.h"
#include "RedirectIO.h"
#include "..\Common\MPIJobDefs.h"
#include "Translate_Error.h"
#include "bsocket.h"
#include "mpdutil.h"
#include "mpd.h"

bool UnmapDrives(int bfd, MapDriveNode *pList)
{
    char pszStr[256];
    if (pList)
    {
	MapDriveNode *pNode = pList;
	while (pNode)
	{
	    sprintf(pszStr, "unmap drive=%c", pNode->cDrive);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		//printf("ERROR: Unable to send unmap command, Error %d", WSAGetLastError());
		//beasy_closesocket(bfd);
		//SetEvent(g_hAbortEvent);
		return false;
	    }
	    if (!ReadString(bfd, pszStr))
	    {
		//printf("ERROR: Unable to read the result of unmap command, Error %d", WSAGetLastError());
		//beasy_closesocket(bfd);
		//SetEvent(g_hAbortEvent);
		return false;
	    }
	    if (stricmp(pszStr, "SUCCESS"))
	    {
		//printf("ERROR: Unable to unmap %c: %s\r\n%s", pNode->cDrive, pNode->pszShare, pszStr);
		//beasy_closesocket(bfd);
		//SetEvent(g_hAbortEvent);
		return false;
	    }
	    pNode = pNode->pNext;
	}
    }
    return true;
}

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
    char pszError[512];
    LONG i;

    //printf("connecting to %s:%d rank %d\n", arg->pszHost, nPort, arg->i);fflush(stdout);
    if ((error = ConnectToMPD(arg->pszHost, nPort, arg->pszPassPhrase, &bfd)) == 0)
    {
	if (arg->i == 0 && !arg->pDlg->m_bNoMPI)
	{
#ifdef MPICH_OLD_ROOT_STARTUP
	    // write "createtmpfile host = host"
	    sprintf(pszStr, "createtmpfile host=%s", arg->pszHost);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		sprintf(pszError, "Unable to send command to create a temporary file on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    // read result
	    if (!ReadString(bfd, pszFileName))
	    {
		sprintf(pszError, "Unable to read the result of the create temp file command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszFileName, "FAIL ", 5) == 0)
	    {
		sprintf(pszError, "Unable to create a temporary file on '%s'\r\n%s", arg->pszHost, pszFileName);
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
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
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    // read result
	    if (!ReadString(bfd, pszStartupDB))
	    {
		printf("ERROR: ReadString failed to read the database name: error %d\n", WSAGetLastError());
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStartupDB, "FAIL ", 5) == 0)
	    {
		printf("Unable to create a database on '%s'\n%s", arg->pszHost, pszStartupDB);fflush(stdout);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    sprintf(pszStr, "|MPICH_EXTRA=mpd:%s:%d:%s", pszStartupDB, nPort, arg->pszPassPhrase);
	    strncat(arg->pszEnv, pszStr, MAX_CMD_LENGTH - 1 - strlen(arg->pszEnv));
#endif
	}
	
	if (arg->i == 0)
	    strcpy(pszIOE, "012"); // only redirect stdin to the root process
	else
	    strcpy(pszIOE, "12");

	if (arg->pDlg->m_nproc > FORWARD_NPROC_THRESHOLD)
	{
	    if (arg->i > 0)
	    {
		while (arg->pDlg->m_pForwardHost[(arg->i - 1)/2].nPort == 0)
		    Sleep(100);
		sprintf(arg->pszIOHostPort, "%s:%d", arg->pDlg->m_pForwardHost[(arg->i - 1)/2].pszHost, arg->pDlg->m_pForwardHost[(arg->i - 1)/2].nPort);
		if (arg->pDlg->m_nproc/2 > arg->i)
		{
		    strncpy(arg->pDlg->m_pForwardHost[arg->i].pszHost, arg->pszHost, MAX_HOST_LENGTH);
		    arg->pDlg->m_pForwardHost[arg->i].pszHost[MAX_HOST_LENGTH-1] = '\0';
		    sprintf(pszStr, "createforwarder host=%s forward=%s", arg->pszHost, arg->pszIOHostPort);
		    WriteString(bfd, pszStr);
		    ReadString(bfd, pszStr);
		    int nTempPort = atoi(pszStr);
		    if (nTempPort == -1)
		    {
			// If creating the forwarder fails, redirect output to the root instead
			// This assignment isn't thread safe.  Who knows if the host part of the structure will be written before the port.
			arg->pDlg->m_pForwardHost[arg->i] = arg->pDlg->m_pForwardHost[0];
		    }
		    else
			arg->pDlg->m_pForwardHost[arg->i].nPort = nTempPort;
		    //printf("forwarder %s:%d\n", g_pForwardHost[arg->i].pszHost, g_pForwardHost[arg->i].nPort);fflush(stdout);
		}
	    }
	}

	if (arg->pDlg->m_pDriveMapList)
	{
	    MapDriveNode *pNode = arg->pDlg->m_pDriveMapList;
	    char *pszEncoded;
	    while (pNode)
	    {
		pszEncoded = EncodePassword(arg->pszPassword);
		sprintf(pszStr, "map drive=%c share=%s account=%s password=%s", 
		    pNode->cDrive, pNode->pszShare, arg->pszAccount, pszEncoded);
		if (pszEncoded != NULL) free(pszEncoded);
		if (WriteString(bfd, pszStr) == SOCKET_ERROR)
		{
		    sprintf(pszError, "Unable to send map command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		    MessageBox(NULL, pszError, "Critical Error", MB_OK);
		    beasy_closesocket(bfd);
		    SetEvent(arg->pDlg->m_hAbortEvent);
		    delete arg;
		    return;
		}
		if (!ReadString(bfd, pszStr))
		{
		    sprintf(pszError, "Unable to read the result of a map command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		    MessageBox(NULL, pszError, "Critical Error", MB_OK);
		    beasy_closesocket(bfd);
		    SetEvent(arg->pDlg->m_hAbortEvent);
		    delete arg;
		    return;
		}
		if (stricmp(pszStr, "SUCCESS"))
		{
		    sprintf(pszError, "Unable to map %c: to %s on %s\r\n%s", pNode->cDrive, pNode->pszShare, arg->pszHost, pszStr);
		    MessageBox(NULL, pszError, "Error", MB_OK);
		    beasy_closesocket(bfd);
		    SetEvent(arg->pDlg->m_hAbortEvent);
		    delete arg;
		    return;
		}
		pNode = pNode->pNext;
	    }
	}

	// LaunchProcess
	if (arg->bLogon)
	{
	    char *pszEncoded;
	    pszEncoded = EncodePassword(arg->pszPassword);
	    if (strlen(arg->pszDir) > 0)
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d d='%s'", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, pszEncoded, pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir);
	    }
	    else
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' a=%s p=%s %s=%s k=%d", 
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, arg->pszAccount, pszEncoded, pszIOE, arg->pszIOHostPort, arg->i);
	    }
	    if (pszEncoded != NULL) free(pszEncoded);
	}
	else
	{
	    if (strlen(arg->pszDir) > 0)
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' %s=%s k=%d d='%s'",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, pszIOE, arg->pszIOHostPort, arg->i, arg->pszDir);
	    }
	    else
	    {
		sprintf(pszStr, "launch h=%s c='%s' e='%s' %s=%s k=%d",
		    arg->pszHost, arg->pszCmdLine, arg->pszEnv, pszIOE, arg->pszIOHostPort, arg->i);
	    }
	}
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    sprintf(pszError, "Unable to send launch command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    MessageBox(NULL, pszError, "Critical Error", MB_OK);
	    beasy_closesocket(bfd);
	    SetEvent(arg->pDlg->m_hAbortEvent);
	    delete arg;
	    return;
	}
	if (!ReadString(bfd, pszStr))
	{
	    sprintf(pszError, "Unable to read the result of the launch command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    MessageBox(NULL, pszError, "Critical Error", MB_OK);
	    beasy_closesocket(bfd);
	    SetEvent(arg->pDlg->m_hAbortEvent);
	    delete arg;
	    return;
	}
	launchid = atoi(pszStr);
	// save the launch id, get the pid
	sprintf(pszStr, "getpid %d", launchid);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    sprintf(pszError, "Unable to send getpid command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    MessageBox(NULL, pszError, "Critical Error", MB_OK);
	    beasy_closesocket(bfd);
	    SetEvent(arg->pDlg->m_hAbortEvent);
	    delete arg;
	    return;
	}
	if (!ReadString(bfd, pszStr))
	{
	    sprintf(pszError, "Unable to read the result of the getpid command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    MessageBox(NULL, pszError, "Critical Error", MB_OK);
	    beasy_closesocket(bfd);
	    SetEvent(arg->pDlg->m_hAbortEvent);
	    delete arg;
	    return;
	}
	nPid = atoi(pszStr);
	if (nPid == -1)
	{
	    sprintf(pszStr, "geterror %d", launchid);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		sprintf(pszError, "Unable to send geterror command after an unsuccessful launch on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (!ReadString(bfd, pszStr))
	    {
		sprintf(pszError, "Unable to read the result of the geterror command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strcmp(pszStr, "ERROR_SUCCESS"))
	    {
		if (arg->i == 0 && !arg->pDlg->m_bNoMPI)
		{
		    sprintf(pszError, "Failed to launch the root process:\n%s\n%s\n", arg->pszCmdLine, pszStr);
		    // Cleanup
#ifdef MPICH_OLD_ROOT_STARTUP
		    sprintf(pszStr, "deletetmpfile host=%s file=%s", arg->pszHost, pszFileName);
		    WriteString(bfd, pszStr);
		    ReadString(bfd, pszStr); // Ignore the result
#endif
		}
		else
		{
		    sprintf(pszError, "Failed to launch process %d:\n'%s'\n%s\n", arg->i, arg->pszCmdLine, pszStr);
		}
		UnmapDrives(bfd, arg->pDlg->m_pDriveMapList);
		sprintf(pszStr, "freeprocess %d", launchid);
		WriteString(bfd, pszStr);
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
		//ExitProcess(0);
	    }
	}
	
	// Get the port number and redirect input to the first process
	if (arg->i == 0 && !arg->pDlg->m_bNoMPI)
	{
#ifdef MPICH_OLD_ROOT_STARTUP
	    // write mpich1readint
	    sprintf(pszStr, "mpich1readint host=%s pid=%d file=%s", arg->pszHost, nPid, pszFileName);
	    WriteString(bfd, pszStr);
	    // read result
	    if (!ReadString(bfd, pszStr))
	    {
		sprintf(pszError, "Unable to read the result of the mpich1readint command on '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStr, "FAIL", 4) == 0)
	    {
		sprintf(pszError, "root process failed to write port to file\n");
		if (strlen(pszStr) > 4)
		    strcat(pszError, pszStr);
		sprintf(pszStr, "kill host=%s pid=%d", arg->pszHost, nPid);
		WriteString(bfd, pszStr);
		UnmapDrives(bfd, arg->pDlg->m_pDriveMapList);
		sprintf(pszStr, "freeprocess %d", launchid);
		WriteString(bfd, pszStr);
		sprintf(pszStr, "deletetmpfile host=%s file=%s", arg->pszHost, pszFileName);
		WriteString(bfd, pszStr);
		ReadString(bfd, pszStr); // Ignore the result
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		MessageBox(NULL, pszError, "Critical Error", MB_OK);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    arg->pDlg->m_nRootPort = atoi(pszStr);
#else
	    // barrier to let the root process do the put
	    sprintf(pszStr, "barrier name=%s count=2", arg->pszJobID);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write the barrier command: error %d", WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to read the result of the barrier command on '%s': error %d", arg->pszHost, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strncmp(pszStr, "SUCCESS", 8))
	    {
		printf("ERROR: barrier failed on '%s':\n%s", arg->pszHost, pszStr);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }

	    // after the barrier, the data is available so do the get
	    sprintf(pszStr, "dbget name=%s key=port", pszStartupDB);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write '%s': error %d", pszStr, WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: Unable to get the root port: error %d", WSAGetLastError());
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strncmp(pszStr, DBS_FAIL_STR, strlen(DBS_FAIL_STR)+1) == 0)
	    {
		printf("ERROR: Unable to get the root port:\n%s", pszStr);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }

	    // save the gotten data
	    arg->pDlg->m_nRootPort = atoi(pszStr);

	    // destroy the database since it is no longer necessary
	    sprintf(pszStr, "dbdestroy name=%s", pszStartupDB);
	    if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	    {
		printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    // read result
	    if (!ReadString(bfd, pszStr))
	    {
		printf("ERROR: ReadString failed to read the result of dbdestroy: error %d\n", WSAGetLastError());
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
	    if (strnicmp(pszStr, DBS_FAIL_STR, strlen(DBS_FAIL_STR)+1) == 0)
	    {
		printf("Unable to destroy the database '%s' on '%s'\n%s", pszStartupDB, arg->pszHost, pszStr);fflush(stdout);
		//ExitProcess(0);
		beasy_closesocket(bfd);
		SetEvent(arg->pDlg->m_hAbortEvent);
		delete arg;
		return;
	    }
#endif
	}
	
	// Start to wait for the process to exit
	sprintf(pszStr, "getexitcodewait %d", launchid);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    sprintf(pszError, "Unable to send a getexitcodewait command to '%s'\r\nError %d", arg->pszHost, WSAGetLastError());
	    MessageBox(NULL, pszError, "Critical Error", MB_OK);
	    beasy_closesocket(bfd);
	    SetEvent(arg->pDlg->m_hAbortEvent);
	    delete arg;
	    return;
	}

	i = InterlockedIncrement(&arg->pDlg->m_nNumProcessSockets) - 1;
	arg->pDlg->m_pProcessSocket[i] = bfd;
	arg->pDlg->m_pProcessLaunchId[i] = launchid;
	arg->pDlg->m_pLaunchIdToRank[i] = arg->i;
    }
    else
    {
	sprintf(pszError, "MPIRunLaunchProcess: Connect to %s failed, error %d\n", arg->pszHost, error);
	MessageBox(NULL, pszError, "Critical Error", MB_OK);
	SetEvent(arg->pDlg->m_hAbortEvent);
    }

    memset(arg->pszPassword, 0, 100);
    delete arg;
}
