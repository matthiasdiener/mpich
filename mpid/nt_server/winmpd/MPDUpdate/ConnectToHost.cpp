#include "stdafx.h"
#include "ConnectToHost.h"
#include "mpd.h"
#include "bsocket.h"
#include "crypt.h"
#include "mpdutil.h"

HANDLE g_hMutex = CreateMutex(NULL, FALSE, NULL);

bool ConnectToHost(const char *host, int port, char *pwd, int *pbfd, bool fast/* = false*/)
{
    int bfd;
    char str[100];
    char phrase[100];
    char *result;
    
    strcpy(phrase, pwd);
    
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	printf("beasy_create failed: %d\n", WSAGetLastError());fflush(stdout);
	return false;
    }
    //printf("connecting to %s:%d\n", host, arg->port);fflush(stdout);
    //if (beasy_connect_timeout(bfd, (char*)host, port, 10) == SOCKET_ERROR)
    if (fast)
    {
	if (beasy_connect_quick(bfd, (char*)host, port) == SOCKET_ERROR)
	{
	    printf("beasy_connect failed: %d\n", WSAGetLastError());fflush(stdout);
	    beasy_closesocket(bfd);
	    return false;
	}
    }
    else
    {
	if (beasy_connect_timeout(bfd, (char*)host, port, 10) == SOCKET_ERROR)
	{
	    printf("beasy_connect failed: %d\n", WSAGetLastError());fflush(stdout);
	    beasy_closesocket(bfd);
	    return false;
	}
    }
    if (!ReadStringTimeout(bfd, str, 10))
    {
	printf("reading prepend string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    strcat(phrase, str);
    WaitForSingleObject(g_hMutex, INFINITE);
    result = crypt(phrase, MPD_SALT_VALUE);
    strcpy(str, result);
    ReleaseMutex(g_hMutex);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	printf("WriteString of the crypt string failed: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (!ReadStringTimeout(bfd, str, 10))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("authentication request failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	printf("WriteString failed after attempting passphrase authentication: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    //printf("connected\n");fflush(stdout);
    *pbfd = bfd;
    return true;
}
