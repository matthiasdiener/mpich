#include "bsocket.h"
#include "GetStringOpt.h"
#include "mpdimpl.h"
#include <stdio.h>

/*#define dbg_printf err_printf*/

struct ForwarderEntry
{
    char pszFwdHost[MAX_HOST_LENGTH];
    int nFwdPort;
    int nPort;
    int bfdStop;
    ForwarderEntry *pNext;
};

struct ForwardIOThreadArg
{
    int bfdStop;
    int bfdListen;
    int bfdForward;
    int nPort;
};

ForwarderEntry *g_pForwarderList = NULL;

static void ForwarderToString(ForwarderEntry *p, char *pszStr, int length)
{
    if (!snprintf_update(pszStr, length, "FORWARDER:\n"))
	return;
    if (!snprintf_update(pszStr, length, " inport: %d\n outhost: %s:%d\n stop socket: %d\n",
	p->nPort, p->pszFwdHost, p->nFwdPort, p->bfdStop))
	return;
}

void statForwarders(char *pszOutput, int length)
{
    ForwarderEntry *p;

    *pszOutput = '\0';
    length--; // leave room for the null character

    if (g_pForwarderList == NULL)
	return;

    p = g_pForwarderList;
    while (p)
    {
	ForwarderToString(p, pszOutput, length);
	length = length - strlen(pszOutput);
	pszOutput = &pszOutput[strlen(pszOutput)];
	p = p->pNext;
    }
}

void ConcatenateForwardersToString(char *pszStr)
{
    char pszLine[100];
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdForwarderMutex");
    
    WaitForSingleObject(hMutex, INFINITE);
    ForwarderEntry *p = g_pForwarderList;
    while (p)
    {
	_snprintf(pszLine, 100, "%s:%d -> %s:%d\n", g_pszHost, p->nPort, p->pszFwdHost, p->nFwdPort);
	strncat(pszStr, pszLine, MAX_CMD_LENGTH - 1 - strlen(pszStr));
	p = p->pNext;
    }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

static void RemoveForwarder(int nPort)
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdForwarderMutex");
    WaitForSingleObject(hMutex, INFINITE);

    ForwarderEntry *pEntry = g_pForwarderList;
    if (pEntry != NULL)
    {
	if (pEntry->nPort == nPort)
	{
	    g_pForwarderList = g_pForwarderList->pNext;
	    delete pEntry;
	    ReleaseMutex(hMutex);
	    CloseHandle(hMutex);
	    return;
	}
	while (pEntry->pNext)
	{
	    if (pEntry->pNext->nPort == nPort)
	    {
		ForwarderEntry *pTemp = pEntry->pNext;
		pEntry->pNext = pEntry->pNext->pNext;
		delete pTemp;
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return;
	    }
	    pEntry = pEntry->pNext;
	}
    }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

static void MakeLoop(int *pbfdRead, int *pbfdWrite)
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

static int ReadWriteAlloc(int bfd, int bfdForward, int n)
{
    int num_to_receive, num_received;
    char *pBuffer;

    pBuffer = new char[n + sizeof(int) + sizeof(char) + sizeof(int)];
    *(int*)pBuffer = n;
    num_to_receive = n + sizeof(int) + sizeof(char);
    
    num_received = beasy_receive(bfd, &pBuffer[sizeof(int)], num_to_receive);
    if (num_received == SOCKET_ERROR || num_received == 0)
    {
	delete pBuffer;
	return SOCKET_ERROR;
    }
    if (beasy_send(bfdForward, pBuffer, num_received + sizeof(int)) == SOCKET_ERROR)
    {
	delete pBuffer;
	return SOCKET_ERROR;
    }

    delete pBuffer;
    return 0;
}

static int ReadWrite(int bfd, int bfdForward, int n)
{
    int num_to_receive, num_received;
    char pBuffer[1024+sizeof(int)+sizeof(char)+sizeof(int)];

    if (n > 1024)
	return ReadWriteAlloc(bfd, bfdForward, n);

    *(int*)pBuffer = n;
    num_to_receive = n + sizeof(int) + sizeof(char);
    
    num_received = beasy_receive(bfd, &pBuffer[sizeof(int)], num_to_receive);
    if (num_received == SOCKET_ERROR || num_received == 0)
    {
	return SOCKET_ERROR;
    }

    if (beasy_send(bfdForward, pBuffer, num_received + sizeof(int)) == SOCKET_ERROR)
    {
	return SOCKET_ERROR;
    }

    return 0;
}

void ForwardIOThread(ForwardIOThreadArg *pArg)
{
    int client_bfd, stop_bfd, listen_bfd, forward_bfd;
    int n, i;
    DWORD num_read;
    bfd_set total_set, readset;
    int bfdActive[FD_SETSIZE];
    int nActive = 0;
    int nDatalen;
    bool bDeleteOnEmpty = false;
    int nPort;
    
    listen_bfd = pArg->bfdListen;
    stop_bfd = pArg->bfdStop;
    forward_bfd = pArg->bfdForward;
    nPort = pArg->nPort;

    delete pArg;
    pArg = NULL;

    BFD_ZERO(&total_set);
    
    BFD_SET(listen_bfd, &total_set);
    BFD_SET(stop_bfd, &total_set);
    BFD_SET(forward_bfd, &total_set);

    while (true)
    {
	readset = total_set;
	dbg_printf("ForwardIOThread: bselect, nActive %d\n", nActive);
	n = bselect(0, &readset, NULL, NULL, NULL);
	if (n == SOCKET_ERROR)
	{
	    err_printf("ForwardIOThread: bselect failed, error %d\n", WSAGetLastError());
	    break;
	}
	if (n == 0)
	{
	    err_printf("ForwardIOThread: bselect returned zero sockets available\n");
	    break;
	}
	else
	{
	    if (BFD_ISSET(stop_bfd, &readset))
	    {
		char c;
		num_read = beasy_receive(stop_bfd, &c, 1);
		if (num_read == SOCKET_ERROR || num_read == 0)
		    break;
		if (c == 0)
		{
		    if (nActive == 0)
		    {
			dbg_printf("ForwardIOThread: %d breaking\n", nPort);
			break;
		    }
		    dbg_printf("ForwardIOThread: ------ %d signalled to exit on empty, %d sockets remaining\n", nPort, nActive);
		    if (total_set.fd_count == 3)
			err_printf("ForwardIOThread: ERROR: total_set is empty\n");
		    bDeleteOnEmpty = true;
		}
		else
		{
		    dbg_printf("ForwardIOThread: aborting forwarder %d\n", nPort);
		    break;
		}
		n--;
	    }
	    if (BFD_ISSET(listen_bfd, &readset))
	    {
		if ((nActive + 3) >= FD_SETSIZE)
		{
		    client_bfd = beasy_accept(listen_bfd);
		    closesocket(client_bfd);
		    dbg_printf("ForwardIOThread: too many clients connecting to the forwarder, connect rejected: nActive = %d\n", nActive);
		}
		else
		{
		    client_bfd = beasy_accept(listen_bfd);
		    if (client_bfd == BFD_INVALID_SOCKET)
		    {
			int error = WSAGetLastError();
			err_printf("ForwardIOThread: baccept failed: %d\n", error);
			break;
		    }
		    
		    char cType;
		    if (beasy_receive(client_bfd, &cType, sizeof(char)) == SOCKET_ERROR)
		    {
			int error = WSAGetLastError();
			err_printf("ForwardIOThread: beasy_receive failed, error %d\n", error);
			break;
		    }
		    
		    if (cType == 0)
		    {
			beasy_closesocket(client_bfd);
			err_printf("ForwardIOThread: stdin redirection not handled by forwarder thread, socket closed.\n");
		    }
		    else
		    {
			bfdActive[nActive] = client_bfd;
			BFD_SET(client_bfd, &total_set);
			nActive++;
			dbg_printf("ForwardIOThread: %d adding socket %d (+%d)\n", nPort, bget_fd(client_bfd), nActive);
		    }
		}
		n--;
	    }
	    if (BFD_ISSET(forward_bfd, &readset))
	    {
		beasy_closesocket(forward_bfd);
		err_printf("ForwardIOThread: forward socket unexpectedly closed\n");
		break;
	    }
	    if (n > 0)
	    {
		if (nActive < 1)
		{
		    err_printf("ForwardIOThread: Error, n=%d while nActive=%d\n", n, nActive);
		    break;
		}
		else
		{
		    for (i=0; n > 0; i++)
		    {
			if (BFD_ISSET(bfdActive[i], &readset))
			{
			    num_read = beasy_receive(bfdActive[i], (char*)&nDatalen, sizeof(int));
			    if (num_read == SOCKET_ERROR || num_read == 0)
			    {
				dbg_printf("ForwardIOThread: %d removing socket[%d] %d (-%d)\n", nPort, i, bget_fd(bfdActive[i]), nActive);
				BFD_CLR(bfdActive[i], &total_set);
				beasy_closesocket(bfdActive[i]);
				nActive--;
				bfdActive[i] = bfdActive[nActive];
				i--;
			    }
			    else
			    {
				if (ReadWrite(bfdActive[i], forward_bfd, nDatalen) == SOCKET_ERROR)
				{
				    dbg_printf("ForwardIOThread: %d abandoning socket[%d] %d (-%d)\n", nPort, i, bget_fd(bfdActive[i]), nActive);
				    BFD_CLR(bfdActive[i], &total_set);
				    beasy_closesocket(bfdActive[i]);
				    nActive--;
				    bfdActive[i] = bfdActive[nActive];
				    i--;
				}
			    }
			    n--;
			}
		    }
		}
	    }
	    if (nActive == 0 && bDeleteOnEmpty)
	    {
		dbg_printf("ForwardIOThread: %d breaking on empty\n", nPort);
		break;
	    }
	}
    }
    beasy_closesocket(forward_bfd);
    beasy_closesocket(stop_bfd);
    for (i=0; i<nActive; i++)
	beasy_closesocket(bfdActive[i]);
    beasy_closesocket(listen_bfd);
    RemoveForwarder(nPort);
    dbg_printf("ForwardIOThread: %d exiting\n", nPort);
    return;
}

int CreateIOForwarder(char *pszFwdHost, int nFwdPort)
{
    int error;
    char pszHost[100];
    HANDLE hThread;
    DWORD dwThreadId;
    ForwarderEntry *pEntry;
    ForwardIOThreadArg *pArg;
    int nPort;
    char ch = 1;

    pArg = new ForwardIOThreadArg;

    // Connect to the forwardee
    if (beasy_create(&pArg->bfdForward, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	err_printf("CreateIOForwarder: beasy_create failed: error %d\n", error);
	delete pArg;
	return BFD_INVALID_SOCKET;
    }
    if (beasy_connect(pArg->bfdForward, pszFwdHost, nFwdPort) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	err_printf("CreateIOForwarder: beasy_connect(%s:%d) failed: error %d\n", pszFwdHost, nFwdPort, error);
	delete pArg;
	return BFD_INVALID_SOCKET;
    }
    if (beasy_send(pArg->bfdForward, &ch, sizeof(char)) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	err_printf("CreateIOForwarder: beasy_send failed: error %d\n", error);
	delete pArg;
	return BFD_INVALID_SOCKET;
    }

    pEntry = new ForwarderEntry;

    // Save the forwardee stuff.  Used only by the forwarders command.
    strncpy(pEntry->pszFwdHost, pszFwdHost, MAX_HOST_LENGTH);
    pEntry->nFwdPort = nFwdPort;

    // Create a listener
    if (beasy_create(&pArg->bfdListen, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	err_printf("CreateIOForwarder: beasy_create listen socket failed: error %d\n", error);
	delete pEntry;
	delete pArg;
	return BFD_INVALID_SOCKET;
    }
    blisten(pArg->bfdListen, 10);
    beasy_get_sock_info(pArg->bfdListen, pszHost, &pEntry->nPort);
    nPort = pEntry->nPort;

    dbg_printf("create forwarder %s:%d -> %s:%d\n", pszHost, pEntry->nPort, pszFwdHost, nFwdPort);

    // Create a stop signal socket
    MakeLoop(&pArg->bfdStop, &pEntry->bfdStop);
    if (pArg->bfdStop == BFD_INVALID_SOCKET || pEntry->bfdStop == BFD_INVALID_SOCKET)
    {
	delete pEntry;
	delete pArg;
	return BFD_INVALID_SOCKET;
    }

    // Let the forward thread know what port it is connected to
    pArg->nPort = nPort;

    // Create the forwarder thread
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ForwardIOThread, pArg, 0, &dwThreadId);
    if (hThread == NULL)
    {
	error = GetLastError();
	err_printf("CreateIOForwarder: CreateThread failed, error %d\n", error);
	delete pEntry;
	delete pArg;
	return BFD_INVALID_SOCKET;
    }
    CloseHandle(hThread);
    
    // Add the new entry to the list
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdForwarderMutex");
    WaitForSingleObject(hMutex, INFINITE);
    pEntry->pNext = g_pForwarderList;
    g_pForwarderList = pEntry;
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return nPort;
}

void StopIOForwarder(int nPort, bool bWaitForEmpty)
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdForwarderMutex");
    WaitForSingleObject(hMutex, INFINITE);
    ForwarderEntry *pEntry = g_pForwarderList;

    while (pEntry)
    {
	if (pEntry->nPort == nPort)
	{
	    if (bWaitForEmpty)
	    {
		char ch = 0;
		beasy_send(pEntry->bfdStop, &ch, 1);
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	    }
	    else
	    {
		int nPort = pEntry->nPort;
		beasy_send(pEntry->bfdStop, "x", 1);
		beasy_closesocket(pEntry->bfdStop);
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		RemoveForwarder(nPort);
	    }
	    return;
	}
	pEntry = pEntry->pNext;
    }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    err_printf("StopIOForwarder: forwarder port %d not found\n", nPort);
}

void AbortAllForwarders()
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdForwarderMutex");
    int nPort;
    
    while (g_pForwarderList)
    {
	WaitForSingleObject(hMutex, INFINITE);
	if (g_pForwarderList)
	{
	    nPort = g_pForwarderList->nPort;
	    ReleaseMutex(hMutex);
	    StopIOForwarder(g_pForwarderList->nPort, false);
	}
	else
	    ReleaseMutex(hMutex);
    }

    CloseHandle(hMutex);
}
