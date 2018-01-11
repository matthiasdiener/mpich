#include "mpdutil.h"
#include "bsocket.h"
#include "mpd.h"

#define err_printf printf
#define PUSH_FUNC(a)
#define POP_FUNC()

bool ReadStringMax(int bfd, char *str, int max)
{
    int n;
    char *str_orig = str;
    int count = 0;

    PUSH_FUNC("ReadString");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	/*
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	}
	*/
	n = beasy_receive(bfd, str, 1);
	if (n == SOCKET_ERROR)
	{
	    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	    POP_FUNC();
	    return false;
	}
	if (n == 0)
	{
	    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
	    POP_FUNC();
	    return false;
	}
	count++;
	if (count == max && *str != '\0')
	{
	    *str = '\0';
	    // truncate, read and discard all further characters of the string
	    char ch;
	    do {
		/*
		n = 0;
		while (!n)
		{
		    n = bread(bfd, &ch, 1);
		    if (n == SOCKET_ERROR)
		    {
			err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
			POP_FUNC();
			return false;
		    }
		}
		*/
		n = beasy_receive(bfd, &ch, 1);
		if (n == SOCKET_ERROR)
		{
		    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		    POP_FUNC();
		    return false;
		}
		if (n == 0)
		{
		    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
		    POP_FUNC();
		    return false;
		}
	    } while (ch != '\0');
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

bool ReadString(int bfd, char *str)
{
    int n;
    char *str_orig = str;

    PUSH_FUNC("ReadString");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	/*
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	}
	*/
	n = beasy_receive(bfd, str, 1);
	if (n == SOCKET_ERROR)
	{
	    err_printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
	    POP_FUNC();
	    return false;
	}
	if (n == 0)
	{
	    err_printf("ReadString[%d] failed, socket closed\n", bget_fd(bfd));
	    POP_FUNC();
	    return false;
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

bool ReadStringTimeout(int bfd, char *str, int timeout)
{
    int n;
    char *str_orig = str;

    PUSH_FUNC("ReadStringTimeout");
    //dbg_printf("reading from %d\n", bget_fd(bfd));
    do {
	n = 0;
	while (!n)
	{
	    n = beasy_receive_timeout(bfd, str, 1, timeout);
	    if (n == SOCKET_ERROR)
	    {
		err_printf("ReadStringTimeout failed, error %d\n", WSAGetLastError());
		POP_FUNC();
		return false;
	    }
	    if (n == 0)
	    {
		POP_FUNC();
		return false;
	    }
	}
    } while (*str++ != '\0');
    //dbg_printf("read <%s>\n", str_orig);
    //return strlen(str_orig);
    POP_FUNC();
    return true;
}

int WriteString(int bfd, char *str)
{
    int ret_val;
    PUSH_FUNC("WriteString");
    if (strlen(str) >= MAX_CMD_LENGTH)
    {
	err_printf("WriteString: command too long, %d\n", strlen(str));
	POP_FUNC();
	return SOCKET_ERROR;
    }
    //dbg_printf("writing to %d, <%s>\n", bget_fd(bfd), str);
    ret_val = beasy_send(bfd, str, strlen(str)+1);
    POP_FUNC();
    return ret_val;
}
