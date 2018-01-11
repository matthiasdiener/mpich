#include <stdio.h>
#include "mpdutil.h"
#include "bsocket.h"
#include "mpd.h"
#include "crypt.h"
#include "Translate_Error.h"

#define MAX_FILENAME MAX_PATH * 2

bool TryCreateDir(char *pszFileName, char *pszError)
{
    char pszTemp[MAX_FILENAME];
    char *token, *next_token;
    int error;

    if (pszFileName[1] == ':')
    {
	strncpy(pszTemp, pszFileName, 3);
	pszTemp[3] = '\0';
	//dbg_printf("changing into directory '%s'\n", pszTemp);
	if (!SetCurrentDirectory(pszTemp))
	{
	    sprintf(pszError, "unable to change to '%s' directory", pszTemp);
	    return false;
	}
	strncpy(pszTemp, &pszFileName[3], MAX_FILENAME);
    }
    else
    {
	sprintf(pszError, "full path not provided");
	// full path not provided
	return false;
    }

    token = strtok(pszTemp, "\\/");
    while (token)
    {
	next_token = strtok(NULL, "\\/");
	if (next_token == NULL)
	    return true;
	//dbg_printf("creating directory '%s'\n", token);
	if (!CreateDirectory(token, NULL))
	{
	    error = GetLastError();
	    if (error != ERROR_ALREADY_EXISTS)
	    {
		sprintf(pszError, "unable to create directory '%s', error %d\n", token, error);
		return false;
	    }
	}
	SetCurrentDirectory(token);
	token = next_token;
    }
    strcpy(pszError, "unknown error");
    return false;
}

#define MPD_CONNECT_READ_TIMEOUT 10

int ConnectToMPD(const char *host, int port, const char *inphrase, int *pbfd)
{
    int bfd;
    char str[512];
    char err_msg[1024];
    char *result;
    int error;
#ifdef USE_LINGER_SOCKOPT
    struct linger linger;
#endif
    BOOL b;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+20];

    if (host == NULL || host[0] == '\0' || port < 1 || inphrase == NULL || pbfd == NULL)
	return -1;

    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: beasy_create failed: %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	return error;
    }
#ifdef USE_LINGER_SOCKOPT
    linger.l_onoff = 1;
    linger.l_linger = 60;
    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: bsetsockopt failed: %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
#endif
    b = TRUE;
    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
    //printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect(bfd, (char*)host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: beasy_connect failed: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("Error: ConnectToMPD: reading prepend string failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    //strcat(phrase, str);
    _snprintf(phrase, MPD_PASSPHRASE_MAX_LENGTH+20, "%s%s", inphrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH); // zero out local copy of the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: WriteString of the crypt string failed: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("Error: ConnectToMPD: reading authentication result failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("Error: ConnectToMPD: authentication request failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: WriteString failed after attempting passphrase authentication: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    //printf("connected to %s\n", host);fflush(stdout);
    *pbfd = bfd;
    return 0;
}

int ConnectToMPDquick(const char *host, int port, const char *inphrase, int *pbfd)
{
    int bfd;
    char str[512];
    char err_msg[1024];
    char *result;
    int error;
#ifdef USE_LINGER_SOCKOPT
    struct linger linger;
#endif
    BOOL b;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+20];

    if (host == NULL || host[0] == '\0' || port < 1 || inphrase == NULL || pbfd == NULL)
	return -1;

    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: beasy_create failed: %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	return error;
    }
#ifdef USE_LINGER_SOCKOPT
    linger.l_onoff = 1;
    linger.l_linger = 60;
    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: bsetsockopt failed: %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
#endif
    b = TRUE;
    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
    //printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect_quick(bfd, (char*)host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: beasy_connect failed: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("Error: ConnectToMPD: reading prepend string failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    //strcat(phrase, str);
    _snprintf(phrase, MPD_PASSPHRASE_MAX_LENGTH+20, "%s%s", inphrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH); // zero out local copy of the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: WriteString of the crypt string failed: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("Error: ConnectToMPD: reading authentication result failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("Error: ConnectToMPD: authentication request failed.\n");
	fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	sprintf(str, "Error: ConnectToMPD: WriteString failed after attempting passphrase authentication: error %d, ", error);
	Translate_Error(error, err_msg, str);
	printf("%s\n", err_msg);
	fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    //printf("connected to %s\n", host);fflush(stdout);
    *pbfd = bfd;
    return 0;
}

void MakeLoop(int *pbfdRead, int *pbfdWrite)
{
    int bfd;
    char host[100];
    int port;
    sockaddr addr;
    int len;

    // Create a listener
    if (beasy_create(&bfd, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	*pbfdRead = BFD_INVALID_SOCKET;
	*pbfdWrite = BFD_INVALID_SOCKET;
	return;
    }
    blisten(bfd, 5);
    beasy_get_sock_info(bfd, host, &port);
    
    // Connect to myself
    if (beasy_create(pbfdWrite, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	beasy_closesocket(bfd);
	*pbfdRead = BFD_INVALID_SOCKET;
	*pbfdWrite = BFD_INVALID_SOCKET;
	return;
    }
    if (beasy_connect(*pbfdWrite, host, port) == SOCKET_ERROR)
    {
	beasy_closesocket(*pbfdWrite);
	beasy_closesocket(bfd);
	*pbfdRead = BFD_INVALID_SOCKET;
	*pbfdWrite = BFD_INVALID_SOCKET;
	return;
    }

    // Accept the connection from myself
    len = sizeof(addr);
    *pbfdRead = baccept(bfd, &addr, &len);

    beasy_closesocket(bfd);
}
