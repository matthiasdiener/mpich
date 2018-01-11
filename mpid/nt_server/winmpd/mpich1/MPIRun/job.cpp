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

static char s_pszRootHost[MAX_HOST_LENGTH];
static int s_nPort = MPD_DEFAULT_PORT;
static char s_pszPassPhrase[MPD_PASSPHRASE_MAX_LENGTH];
static char s_pszJobId[256];

void PutJobInDatabase(MPIRunLaunchProcessArg *arg)
{
    int error;
    int bfd;
    SYSTEMTIME stime;
    char pszStr[MAX_CMD_LENGTH+1];
    char pszResult[MAX_CMD_LENGTH+1];

    if (!g_bUseJobHost)
	return;

    // save the host
    strcpy(s_pszRootHost, g_pszJobHost);
    // save the passphrase
    if (g_bUseJobMPDPwd)
	strcpy(s_pszPassPhrase, g_pszJobHostMPDPwd);
    else
	strcpy(s_pszPassPhrase, MPD_DEFAULT_PASSPHRASE);
    // save the jobid
    strcpy(s_pszJobId, arg->pszJobID);

    if ((error = ConnectToMPD(s_pszRootHost, s_nPort, s_pszPassPhrase, &bfd)) == 0)
    {
	// open the jobs database
	sprintf(pszStr, "dbcreate jobs");
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the jobs database query: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("Unable to open the jobs database on '%s'\n%s", s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// create a database for this job with the jobid as its name
	sprintf(pszStr, "dbcreate %s", arg->pszJobID);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the job database creation request: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("Unable to create the job database(%s) on '%s'\n%s", arg->pszJobID, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// get the current time and put it in the jobs database with this jobid
	GetLocalTime(&stime);
	sprintf(pszStr, "dbput jobs:%d.%02d.%02d<%02dh.%02dm.%02ds>:%s@%s", stime.wYear, stime.wMonth, stime.wDay, stime.wHour, stime.wMinute, stime.wSecond, arg->pszAccount, arg->pszJobID);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the jobs timestamp put operation: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("Unable to put the job timestamp in the jobs database on '%s'\n%s", s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// put the user name in the job database
	sprintf(pszStr, "dbput %s:user:%s", arg->pszJobID, (arg->pszAccount[0] == '\0') ? "<single user mode>" : arg->pszAccount);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the job timestamp put operation: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}

	// put the size of the parallel process in the job database
	sprintf(pszStr, "dbput %s:nproc:%d", arg->pszJobID, g_nNproc);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the job nproc put operation: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}

	// put the state of the job database
	sprintf(pszStr, "dbput %s:state:LAUNCHING", arg->pszJobID, (arg->pszAccount[0] == '\0') ? "<single user mode>" : arg->pszAccount);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the job state put operation: error %d\n", WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}

	// close the session with the mpd
	if (WriteString(bfd, "done") == SOCKET_ERROR)
	{
	    printf("Error: Unable to write 'done' to socket[%d]\n", bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}

	beasy_closesocket(bfd);
    }
    else
    {
	printf("PutJobInDatabase: Connect to %s failed, error %d\n", s_pszRootHost, error);fflush(stdout);
    }
}

void PutJobProcessInDatabase(MPIRunLaunchProcessArg *arg, int pid)
{
    int bfd;
    char pszStr[MAX_CMD_LENGTH+1];
    char pszResult[MAX_CMD_LENGTH+1];
    int error;
    char pszRank[100];
    int extent;

    if (!g_bUseJobHost)
	return;

    if (arg->n < 10)
	extent = 1;
    else if (arg->n < 100)
	extent = 2;
    else if (arg->n < 1000)
	extent = 3;
    else extent = 4;

    sprintf(pszStr, "%%0%dd", extent);
    sprintf(pszRank, pszStr, arg->i);

    if ((error = ConnectToMPD(s_pszRootHost, s_nPort, s_pszPassPhrase, &bfd)) == 0)
    {
	// put host
	sprintf(pszStr, "dbput %s:%shost:%s", arg->pszJobID, pszRank, arg->pszHost);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// put command line
	sprintf(pszStr, "dbput name=%s key=%scmd value=%s", arg->pszJobID, pszRank, arg->pszCmdLine);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// put working directory
	sprintf(pszStr, "dbput name=%s key=%sdir value=%s", arg->pszJobID, pszRank, arg->pszDir);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// put environment variables
	sprintf(pszStr, "dbput %s:%senv:%s", arg->pszJobID, pszRank, arg->pszEnv);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}
	
	// put process id
	sprintf(pszStr, "dbput %s:%spid:%d", arg->pszJobID, pszRank, pid);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}

	// close the session with the mpd
	if (WriteString(bfd, "done") == SOCKET_ERROR)
	{
	    printf("Error: Unable to write 'done' to socket[%d]\n", bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}

	beasy_closesocket(bfd);
    }
    else
    {
	printf("PutJobProcessInRootMPD: Connect to %s failed, error %d\n", s_pszRootHost, error);fflush(stdout);
    }
}

void UpdateJobState(char *state)
{
    int bfd;
    char pszStr[MAX_CMD_LENGTH+1];
    char pszResult[MAX_CMD_LENGTH+1];
    int error;

    if (!g_bUseJobHost)
	return;

    if ((error = ConnectToMPD(s_pszRootHost, s_nPort, s_pszPassPhrase, &bfd)) == 0)
    {
	// put the state string
	sprintf(pszStr, "dbput %s:state:%s", s_pszJobId, state);
	if (WriteString(bfd, pszStr) == SOCKET_ERROR)
	{
	    printf("ERROR: Unable to write '%s' to socket[%d]\n", pszStr, bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}
	if (!ReadString(bfd, pszResult))
	{
	    printf("ERROR: ReadString failed to read the result of the put operation: '%s', error %d\n", pszStr, WSAGetLastError());
	    beasy_closesocket(bfd);
	    return;
	}
	if (strnicmp(pszResult, "DBS_SUCCESS", 11) != 0)
	{
	    printf("ERROR: put operation('%s') failed on '%s'\n%s", pszStr, s_pszRootHost, pszResult);fflush(stdout);
	    beasy_closesocket(bfd);
	    return;
	}

	// close the session with the mpd
	if (WriteString(bfd, "done") == SOCKET_ERROR)
	{
	    printf("Error: Unable to write 'done' to socket[%d]\n", bget_fd(bfd));
	    beasy_closesocket(bfd);
	    return;
	}

	beasy_closesocket(bfd);
    }
    else
    {
	printf("UpdateJobState(%s): Connect to %s failed, error %d\n", state, s_pszRootHost, error);fflush(stdout);
    }
}
