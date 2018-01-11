#include "mpijob.h"
#include "Translate_Error.h"

void GetAndPrintState(int bfd, char *dbname)
{
    char str[256];
    int error;

    sprintf(str, "dbget %s:state", dbname);
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
	printf("%s\n", str);
    }
    else
    {
	printf("Unable to read the job state.\n");
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
	return;
    }
}

void ListJobs(char *host, int port, char *altphrase)
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
	printf("Unable to connect to the mpd on %s\n", host);
	return;
    }

    printf("Jobs on %s:\n", host);
    //printf("start time : user@jobid\n");
    printf("yyyy.mm.dd<hh .mm .ss > : user@jobid : state\n");
    printf("--------------------------------------------\n");
    strcpy(str, "dbfirst jobs");
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
	printf("%s : %s : ", key, value);
	GetAndPrintState(bfd, strstr(value, "@")+1);
    }
    else
    {
	printf("Unable to read the jobs on %s.\n", host);
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
	    printf("%s : %s : ", key, value);
	    GetAndPrintState(bfd, strstr(value, "@")+1);
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
