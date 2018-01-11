#include "mpijob.h"
#include "Translate_Error.h"

void JobsToFile(char *filename, char *option, char *host, int port, char *altphrase)
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
    FILE *fout;
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

    // make sure the filename is valid and erase the contents
    fout = fopen(filename, "w");
    if (fout == NULL)
    {
	printf("Error: JobsToFile, unable to open file %s\n", filename);
	return;
    }
    fclose(fout);

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
	    if (bJob)
		printf("no jobs on %s\n", host);
	    WriteString(bfd, "done");
	    beasy_closesocket(bfd);
	    return;
	}
	if (strcmp(str, "DBS_END") == 0)
	{
	    if (bJob)
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
	    DisplayJob(pszJob, host, port, altphrase, true, true, filename);
	}
	else if (bTimeStamp)
	{
	    if (CompareTimeStamps(key, option, relation))
	    {
		if (relation < 0)
		{
		    printf("%s : %s\n", key, value);
		    DisplayJob(pszJob, host, port, altphrase, true, true, filename);
		}
	    }
	}
	else if (bJob)
	{
	    if (strcmp(pszJob, option) == 0)
	    {
		printf("%s : %s\n", key, value);
		DisplayJob(pszJob, host, port, altphrase, true, true, filename);
		WriteString(bfd, "done");
		beasy_closesocket(bfd);
		return;
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
		DisplayJob(pszJob, host, port, altphrase, true, true, filename);
	    }
	    else if (bTimeStamp)
	    {
		if (CompareTimeStamps(key, option, relation))
		{
		    if (relation < 0)
		    {
			printf("%s : %s\n", key, value);
			DisplayJob(pszJob, host, port, altphrase, true, true, filename);
		    }
		}
	    }
	    else if (bJob)
	    {
		if (strcmp(pszJob, option) == 0)
		{
		    printf("%s : %s\n", key, value);
		    DisplayJob(pszJob, host, port, altphrase, true, true, filename);
		    WriteString(bfd, "done");
		    beasy_closesocket(bfd);
		    return;
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

    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);
}
