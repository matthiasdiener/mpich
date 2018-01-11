#include "mpijob.h"
#include "Translate_Error.h"

void DeleteJob(int bfd, char *pszJob)
{
    char str[256];
    int error;

    sprintf(str, "dbdestroy %s", pszJob);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("Error: DeleteJob, writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	beasy_closesocket(bfd);
	return;
    }
    if (!ReadStringTimeout(bfd, str, 10))
    {
	printf("Error, DeleteJob, unable to delete the job '%s'.\n", pszJob);
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
	return;
    }
}

void DeleteKey(int bfd, char *key)
{
    char str[256];
    int error;

    sprintf(str, "dbdelete jobs:%s", key);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("Error: DeleteKey, writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	beasy_closesocket(bfd);
	return;
    }
    if (!ReadStringTimeout(bfd, str, 10))
    {
	printf("Error, DeleteKey, unable to delete the job entry '%s'.\n", key);
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
	return;
    }
}

struct KeyNode
{
    char key[256];
    KeyNode *next;
};

static KeyNode *s_pKeyList = NULL;
void SaveKeyToDelete(char *key)
{
    KeyNode *n = new KeyNode;

    strcpy(n->key, key);
    n->next = s_pKeyList;
    s_pKeyList = n;
}

void ClearJobs(char *option, char *host, int port, char *altphrase)
{
    int bfd;
    char str[CONSOLE_STR_LENGTH+1];
    int error;
    char key[100];
    char value[CONSOLE_STR_LENGTH];
    char localhost[100];
    int year, month, day, hour, minute, second;
    bool bAll=false, bTimeStamp=false, bJob=false;
    char *pszJob;
    int relation;

    if (host == NULL)
    {
	gethostname(localhost, 100);
	host = localhost;
    }

    if (stricmp(option, "all") == 0)
    {
	bAll = true;
    } else if (ParseTimeStamp(option, year, month, day, hour, minute, second))
    {
	bTimeStamp = true;
    } else
    {
	bJob = true;
    }

    if (ConnectToMPD(host, port, (altphrase == NULL) ? MPD_DEFAULT_PASSPHRASE : altphrase, &bfd) != 0)
    {
	printf("Error: KillJob, unable to connect to the mpd on %s\n", host);
	return;
    }

    strcpy(str, "dbfirst jobs");
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("Error: JobsToFile, writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	beasy_closesocket(bfd);
	return;
    }
    if (ReadStringTimeout(bfd, str, 10))
    {
	if (strcmp(str, "DBS_FAIL") == 0)
	{
	    printf("no jobs on %s\n", host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
	if (strcmp(str, "DBS_END") == 0)
	{
	    printf("no jobs on %s\n", host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
	GetKeyAndValue(str, key, value);
	pszJob = strstr(value, "@")+1;
	if (bAll)
	{
	    printf("%s : %s\n", key, value);
	    DeleteJob(bfd, pszJob);
	}
	else if (bTimeStamp)
	{
	    if (CompareTimeStamps(key, option, relation))
	    {
		if (relation < 0)
		{
		    printf("%s : %s\n", key, value);
		    DeleteJob(bfd, pszJob);
		    SaveKeyToDelete(key);
		}
	    }
	}
	else if (bJob)
	{
	    if (strcmp(pszJob, option) == 0)
	    {
		printf("%s : %s\n", key, value);
		DeleteJob(bfd, pszJob);
		SaveKeyToDelete(key);
	    }
	}
    }
    else
    {
	printf("Error, JobsToFile, unable to read the jobs on %s.\n", host);
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
	return;
    }

    while (true)
    {
	strcpy(str, "dbnext jobs");
	if (WriteString(bfd, str) == SOCKET_ERROR)
	{
	    error = WSAGetLastError();
	    printf("writing '%s' failed, %d\n", str, error);
	    Translate_Error(error, str);
	    printf("%s\n", str);
	    beasy_closesocket(bfd);
	    return;
	}
	if (ReadStringTimeout(bfd, str, 10))
	{
	    if (strcmp(str, "DBS_FAIL") == 0)
	    {
		printf("unexpected error reading the next job\n");
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		return;
	    }
	    if (strcmp(str, "DBS_END") == 0)
	    {
		break;
	    }
	    GetKeyAndValue(str, key, value);
	    pszJob = strstr(value, "@")+1;
	    if (bAll)
	    {
		printf("%s : %s\n", key, value);
		DeleteJob(bfd, pszJob);
	    }
	    else if (bTimeStamp)
	    {
		if (CompareTimeStamps(key, option, relation))
		{
		    if (relation < 0)
		    {
			printf("%s : %s\n", key, value);
			DeleteJob(bfd, pszJob);
			SaveKeyToDelete(key);
		    }
		}
	    }
	    else if (bJob)
	    {
		if (strcmp(pszJob, option) == 0)
		{
		    printf("%s : %s\n", key, value);
		    DeleteJob(bfd, pszJob);
		    SaveKeyToDelete(key);
		}
	    }
	}
	else
	{
	    printf("Unable to read the jobs on %s.\n", host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
    }

    if (bAll)
    {
	if (WriteString(bfd, "dbdestroy jobs") == SOCKET_ERROR)
	{
	    error = WSAGetLastError();
	    Translate_Error(error, str);
	    printf("WriteString failed: %d\n%s\n", error, str);
	    fflush(stdout);
	}
	if (ReadStringTimeout(bfd, str, 10))
	{
	    if (strcmp(str, "DBS_FAIL") == 0)
	    {
		printf("Error: Unable to read the result of deleting the jobs database\n");
	    }
	}
	else
	{
	    printf("Error: Unable to read the result of deleting the jobs database\n");
	}
    }
    else
    {
	if (s_pKeyList == NULL)
	{
	    if (bJob)
	    {
		printf("The specified job, %s, does not exist on %s\n", pszJob, host);
	    }
	    else
	    {
		printf("No jobs on %s are earlier than %s\n", host, option);
	    }
	}
	KeyNode *n;
	while (s_pKeyList)
	{
	    n = s_pKeyList;
	    s_pKeyList = s_pKeyList->next;
	    DeleteKey(bfd, n->key);
	    delete n;
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
}
