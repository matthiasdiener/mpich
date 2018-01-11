#include "mpijob.h"
#include "Translate_Error.h"

struct KillHostNode
{
    int rank;
    int pid;
    char host[MAX_HOST_LENGTH];
    KillHostNode *next;
};

static KillHostNode *g_pKillList = NULL;

static KillHostNode *GetKillNode(int rank)
{
    KillHostNode *pNode;

    if (g_pKillList == NULL)
    {
	pNode = new KillHostNode;
	pNode->host[0] = '\0';
	pNode->pid = -1;
	pNode->rank = rank;
	pNode->next = NULL;
	
	g_pKillList = pNode;
	return pNode;
    }

    pNode = g_pKillList;
    while (pNode)
    {
	if (pNode->rank == rank)
	    return pNode;
	pNode = pNode->next;
    }

    pNode = new KillHostNode;
    pNode->host[0] = '\0';
    pNode->pid = -1;
    pNode->rank = rank;
    pNode->next = NULL;
    
    pNode->next = g_pKillList;
    g_pKillList = pNode;
    return pNode;
}

static void InsertHost(int rank, char *host)
{
    KillHostNode *pNode;

    pNode = GetKillNode(rank);
    strcpy(pNode->host, host);
}

static void InsertPid(int rank, int pid)
{
    KillHostNode *pNode;

    pNode = GetKillNode(rank);
    pNode->pid = pid;
}

static void FindSaveHostPid(char *key, char *value)
{
    int rank;
    char option[20];

    if (GetRankAndOption(key, rank, option))
    {
	if (strcmp(option, "host") == 0)
	    InsertHost(rank, value);
	else if (strcmp(option, "pid") == 0)
	    InsertPid(rank, atoi(value));
    }
}

void KillJobProcess(char *host, int port, char *altphrase, int pid)
{
    int bfd;
    char str[256];
    int error;

    if (ConnectToMPD(host, port, (altphrase == NULL) ? MPD_DEFAULT_PASSPHRASE : altphrase, &bfd) != 0)
    {
	printf("Error: KillJobProcess(%s:%d) unable to connect to the mpd on %s\n", host, pid, host);
	return;
    }

    sprintf(str, "kill host=%s pid=%d", host, pid);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("Error: KillJobProcess, writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	beasy_closesocket(bfd);
	return;
    }

    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("Error: KillJobProcess, WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);
}

void KillJobProcesses(int port, char *altphrase)
{
    KillHostNode *pNode;
    while (g_pKillList)
    {
	pNode = g_pKillList;
	g_pKillList = g_pKillList->next;

	KillJobProcess(pNode->host, port, altphrase, pNode->pid);
	delete pNode;
    }
}

void KillJob(char *job, char *host, int port, char *altphrase)
{
    int bfd;
    char str[CONSOLE_STR_LENGTH+1];
    int error;
    char key[100];
    char value[CONSOLE_STR_LENGTH];
    char localhost[100];

    if (host == NULL)
    {
	gethostname(localhost, 100);
	host = localhost;
    }

    if (ConnectToMPD(host, port, (altphrase == NULL) ? MPD_DEFAULT_PASSPHRASE : altphrase, &bfd) != 0)
    {
	printf("Error: KillJob, unable to connect to the mpd on %s\n", host);
	return;
    }

    sprintf(str, "dbfirst %s", job);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("Error: KillJob, writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	beasy_closesocket(bfd);
	return;
    }
    if (ReadStringTimeout(bfd, str, 10))
    {
	if (strcmp(str, "DBS_FAIL") == 0)
	{
	    printf("job %s does not exist on %s\n", job, host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
	if (strcmp(str, "DBS_END") == 0)
	{
	    printf("job %s does not exist on %s\n", job, host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
	GetKeyAndValue(str, key, value);
	FindSaveHostPid(key, value);
    }
    else
    {
	printf("Unable to read the job on %s.\n", host);
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
	return;
    }

    while (true)
    {
	sprintf(str, "dbnext %s", job);
	if (WriteString(bfd, str) == SOCKET_ERROR)
	{
	    error = WSAGetLastError();
	    printf("Error: KillJob, writing '%s' failed, %d\n", str, error);
	    Translate_Error(error, str);
	    printf("%s\n", str);
	    beasy_closesocket(bfd);
	    return;
	}
	if (ReadStringTimeout(bfd, str, 10))
	{
	    if (strcmp(str, "DBS_FAIL") == 0)
	    {
		printf("Error: KillJob, unexpected error reading the next key/value pair\n");
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		return;
	    }
	    if (strcmp(str, "DBS_END") == 0)
	    {
		break;
	    }
	    GetKeyAndValue(str, key, value);
	    FindSaveHostPid(key, value);
	}
	else
	{
	    printf("Error: KillJob, unable to read the next job key/value pair on %s.\n", host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
    }

    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("Error: KillJob, WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);

    KillJobProcesses(port, altphrase);
}
