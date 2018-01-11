#include <stdio.h>
#include "mpdutil.h"
#include "bsocket.h"
#include "mpd.h"
#include "crypt.h"

bool ReadStringMax(int bfd, char *str, int max)
{
    int n;
    char *str_orig = str;
    int count = 0;

    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		return false;
	    }
	}
	count++;
	if (count == max && *str != '\0')
	{
	    // truncate, read and discard all further characters of the string
	    char ch;
	    do {
		n = 0;
		while (!n)
		{
		    n = bread(bfd, &ch, 1);
		    if (n == SOCKET_ERROR)
		    {
			printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
			return false;
		    }
		}
	    } while (ch != '\0');

	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    return true;
}

bool ReadString(int bfd, char *str)
{
    int n;
    char *str_orig = str;

    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	/*
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		return false;
	    }
	}
	*/
	n = beasy_receive(bfd, str, 1);
	if (n == SOCKET_ERROR)
	{
	    printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	    return false;
	}
	if (n == 0)
	{
	    printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
	    return false;
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    return true;
}

bool ReadStringTimeout(int bfd, char *str, int timeout)
{
    int n;
    char *str_orig = str;

    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	n = 0;
	while (!n)
	{
	    n = beasy_receive_timeout(bfd, str, 1, timeout);
	    if (n == SOCKET_ERROR)
	    {
		printf("ReadStringTimeout failed, error %d\n", WSAGetLastError());
		return false;
	    }
	    if (n == 0)
	    {
		return false;
	    }
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    return true;
}

int WriteString(int bfd, char *str)
{
    int ret_val;
    if (strlen(str) >= MAX_CMD_LENGTH)
    {
	printf("WriteString: command too long, %d\n", strlen(str));
	return SOCKET_ERROR;
    }
    //dbg_printf("writing to %d, <%s>\n", bget_fd(bfd), str);
    ret_val = beasy_send(bfd, str, strlen(str)+1);
    return ret_val;
}

#define MPD_CONNECT_READ_TIMEOUT 10

int ConnectToMPD(char *host, int port, char *inphrase, int *pbfd)
{
    int bfd;
    char str[256];
    char *result;
    int error;
    struct linger linger;
    BOOL b;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+20];

    if (host == NULL || host[0] == '\0' || port < 1 || inphrase == NULL || pbfd == NULL)
	return -1;
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("beasy_create failed: %d\n", error);fflush(stdout);
	return error;
    }
    linger.l_onoff = 1;
    linger.l_linger = 60;
    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("bsetsockopt failed: %d\n", error);
	beasy_closesocket(bfd);
	return error;
    }
    b = TRUE;
    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
    //printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect(bfd, host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("beasy_connect failed: error %d\n", error);fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("reading prepend string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    //strcat(phrase, str);
    sprintf(phrase, "%s%s", inphrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH); // zero out local copy of the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("WriteString of the crypt string failed: error %d\n", error);fflush(stdout);
	beasy_closesocket(bfd);
	return error;
    }
    if (!ReadStringTimeout(bfd, str, MPD_CONNECT_READ_TIMEOUT))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("authentication request failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return -1;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("WriteString failed after attempting passphrase authentication: error %d\n", error);fflush(stdout);
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

