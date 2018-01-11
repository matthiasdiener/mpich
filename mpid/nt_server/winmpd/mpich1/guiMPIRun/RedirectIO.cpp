#include "stdafx.h"
#include "guiMPIRun.h"
#include "guiMPIRunDoc.h"
#include "guiMPIRunView.h"
#include "RedirectIO.h"
#include "global.h"
#include <stdio.h>
#include "bsocket.h"
#include "mpdutil.h"

static CGuiMPIRunView *g_pDlg;
static HANDLE g_hConsoleOutputMutex;
static int g_bfdListen;
static HANDLE g_hListenReleasedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

static void WriteOutputToRichEdit(char *pszStr, COLORREF color, CRichEditCtrl &edit)
{
    CHARFORMAT cf;
    int last;
    int nBefore;

    nBefore = edit.GetLineCount();

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = color;
    cf.dwEffects = 0;
    last = -1;
    edit.SetSel(last, last);
    edit.ReplaceSel(pszStr);
    last = edit.GetTextLength();
    edit.SetSel(last - strlen(pszStr), last);
    edit.SetSelectionCharFormat(cf);
    edit.SetSel(-1, -1);
    edit.LineScroll(edit.GetLineCount() - nBefore);

    if (g_pDlg->m_redirect && g_pDlg->m_fout)
    {
	fprintf(g_pDlg->m_fout, "%s", pszStr);
	fflush(g_pDlg->m_fout);
    }
}

void RedirectRichEdit(int bfd)
{
    HANDLE hEvent[3];
    DWORD dwResult;

    hEvent[0] = g_pDlg->m_hAbortEvent;
    hEvent[1] = g_pDlg->m_hJobFinished;
    hEvent[2] = g_pDlg->m_hRedirectStdinEvent;

    while (true)
    {
	dwResult = WaitForMultipleObjects(3, hEvent, FALSE, INFINITE);
	switch (dwResult)
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0+1:
	    beasy_closesocket(bfd);
	    CloseHandle(g_pDlg->m_hRedirectRicheditThread);
	    g_pDlg->m_hRedirectRicheditThread = NULL;
	    return;
	    break;
	case WAIT_OBJECT_0+2:
	    if (WaitForSingleObject(g_pDlg->m_hRedirectStdinMutex, 10000) == WAIT_OBJECT_0)
	    {
		if (g_pDlg->m_pRedirectStdinList == NULL)
		{
		    beasy_closesocket(bfd);
		    ReleaseMutex(g_pDlg->m_hRedirectStdinMutex);
		    CloseHandle(g_pDlg->m_hRedirectRicheditThread);
		    g_pDlg->m_hRedirectRicheditThread = NULL;
		    return;
		}
		if (beasy_send(bfd, g_pDlg->m_pRedirectStdinList->str.GetBuffer(0), g_pDlg->m_pRedirectStdinList->str.GetLength()) == SOCKET_ERROR)
		{
		    beasy_closesocket(bfd);
		    ReleaseMutex(g_pDlg->m_hRedirectStdinMutex);
		    CloseHandle(g_pDlg->m_hRedirectRicheditThread);
		    g_pDlg->m_hRedirectRicheditThread = NULL;
		    return;
		}
		CGuiMPIRunView::RedirectStdinStruct *pNode = g_pDlg->m_pRedirectStdinList;
		g_pDlg->m_pRedirectStdinList = g_pDlg->m_pRedirectStdinList->pNext;
		delete pNode;
		if (g_pDlg->m_pRedirectStdinList == NULL)
		    ResetEvent(g_pDlg->m_hRedirectStdinEvent);
		ReleaseMutex(g_pDlg->m_hRedirectStdinMutex);
	    }
	    break;
	default:
	    beasy_closesocket(bfd);
	    CloseHandle(g_pDlg->m_hRedirectRicheditThread);
	    g_pDlg->m_hRedirectRicheditThread = NULL;
	    return;
	    break;
	}
    }
}

void RedirectIOThread2(int abort_bfd)
{
    int client_bfd, child_abort_bfd = BFD_INVALID_SOCKET;
    int n, i;
    sockaddr addr;
#ifdef USE_LINGER_SOCKOPT
    struct linger linger;
#endif
    int len;
    BOOL b;
    char pBuffer[1024];
    DWORD num_read;
    bfd_set total_set, readset;
    int bfdActive[FD_SETSIZE];
    int nActive = 0;
    int nRank;
    char cType;
    int nDatalen;
    bool bDeleteOnEmpty = false;
    HANDLE hChildThread = NULL;
    
    BFD_ZERO(&total_set);
    BFD_SET(abort_bfd, &total_set);
    BFD_SET(g_bfdListen, &total_set);

    while (true)
    {
	readset = total_set;
	n = bselect(0, &readset, NULL, NULL, NULL);
	if (n == SOCKET_ERROR)
	{
	    printf("RedirectIOControlThread2: bselect failed, error %d\n", WSAGetLastError());fflush(stdout);
	    beasy_closesocket(abort_bfd);
	    for (i=0; i<nActive; i++)
		beasy_closesocket(bfdActive[i]);
	    return;
	}
	if (n == 0)
	{
	    printf("RedirectIOControlThread2: bselect returned zero sockets available\n");fflush(stdout);
	    beasy_closesocket(abort_bfd);
	    for (i=0; i<nActive; i++)
		beasy_closesocket(bfdActive[i]);
	    return;
	}
	else
	{
	    if (BFD_ISSET(abort_bfd, &readset))
	    {
		char c;
		bool bCloseNow = true;
		num_read = beasy_receive(abort_bfd, &c, 1);
		if (num_read == 1)
		{
		    if (c == 0)
		    {
			if (child_abort_bfd != BFD_INVALID_SOCKET)
			    beasy_send(child_abort_bfd, &c, 1);

			if (nActive == 0)
			    WaitForSingleObject(hChildThread, 10000);
			else
			    bCloseNow = false;
			bDeleteOnEmpty = true;
		    }
		}
		if (bCloseNow)
		{
		    for (i=0; i<nActive; i++)
			beasy_closesocket(bfdActive[i]);
		    nActive = 0;
		    if (child_abort_bfd == BFD_INVALID_SOCKET)
			SetEvent(g_hListenReleasedEvent);
		    else
		    {
			beasy_send(child_abort_bfd, "x", 1);
			beasy_closesocket(child_abort_bfd);
		    }
		    beasy_closesocket(abort_bfd);
		    if (hChildThread != NULL)
			CloseHandle(hChildThread);
		    return;
		}
	    }
	    if (BFD_ISSET(g_bfdListen, &readset))
	    {
		if ((nActive + 3) >= FD_SETSIZE)
		{
		    int temp_bfd;
		    MakeLoop(&temp_bfd, &child_abort_bfd);
		    if (temp_bfd == BFD_INVALID_SOCKET || child_abort_bfd == BFD_INVALID_SOCKET)
		    {
			MessageBox(NULL, "Unable to create a socket", "Critical error", MB_OK);
			break;
		    }
		    DWORD dwThreadId;
		    hChildThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectIOThread2, (LPVOID)temp_bfd, 0, &dwThreadId);
		    if (hChildThread == NULL)
		    {
			MessageBox(NULL, "Unable to create an io thread", "Critical error", MB_OK);
			break;
		    }
		    BFD_CLR(g_bfdListen, &total_set);
		}
		else
		{
		    len = sizeof(addr);
		    client_bfd = baccept(g_bfdListen, &addr, &len);
		    if (client_bfd == BFD_INVALID_SOCKET)
		    {
			char str[256];
			int error = WSAGetLastError();
			sprintf(str, "RedirectIOControlThread: baccept failed: %d\n", error);
			MessageBox(NULL, str, "Error", MB_OK);
			break;
		    }
#ifdef USE_LINGER_SOCKOPT
		    linger.l_onoff = 1;
		    linger.l_linger = 60;
		    if (bsetsockopt(client_bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
		    {
			char str[256];
			int error = WSAGetLastError();
			sprintf(str, "RedirectIOControlThread: bsetsockopt failed: %d\n", error);
			MessageBox(NULL, str, "Error", MB_OK);
			beasy_closesocket(client_bfd);
			break;
		    }
#endif
		    b = TRUE;
		    bsetsockopt(client_bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
		    
		    if (beasy_receive(client_bfd, &cType, sizeof(char)) == SOCKET_ERROR)
			break;
		    
		    if (cType == 0)
		    {
			DWORD dwThreadID;
			if (g_pDlg->m_hRedirectRicheditThread != NULL)
			    TerminateThread(g_pDlg->m_hRedirectRicheditThread, 0);
			g_pDlg->m_hRedirectRicheditThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectRichEdit, (void*)client_bfd, 0, &dwThreadID);
		    }
		    else
		    {
			bfdActive[nActive] = client_bfd;
			BFD_SET(client_bfd, &total_set);
			nActive++;
		    }
		}
		n--;
	    }
	    if (n > 0)
	    {
		for (i=0; n > 0; i++)
		{
		    if (BFD_ISSET(bfdActive[i], &readset))
		    {
			char pTemp[sizeof(int)+sizeof(char)+sizeof(int)];
			num_read = beasy_receive(bfdActive[i], pTemp, sizeof(int)+sizeof(char)+sizeof(int));
			if (num_read == SOCKET_ERROR || num_read == 0)
			{
			    BFD_CLR(bfdActive[i], &total_set);
			    beasy_closesocket(bfdActive[i]);
			    nActive--;
			    bfdActive[i] = bfdActive[nActive];
			    i--;
			    //printf("(-%d)", nActive);fflush(stdout);
			}
			else
			{
			    nDatalen = *(int*)pTemp;
			    cType = pTemp[sizeof(int)];
			    nRank = *(int*)&pTemp[sizeof(int)+sizeof(char)];
			    num_read = beasy_receive(bfdActive[i], pBuffer, nDatalen);
			    if (num_read == SOCKET_ERROR || num_read == 0)
			    {
				BFD_CLR(bfdActive[i], &total_set);
				beasy_closesocket(bfdActive[i]);
				nActive--;
				bfdActive[i] = bfdActive[nActive];
				i--;
				//printf("(-%d)", nActive);fflush(stdout);
			    }
			    else
			    {
				WaitForSingleObject(g_hConsoleOutputMutex, INFINITE);
				pBuffer[num_read] = '\0';
				if (g_pDlg->m_bNoColor)
				    WriteOutputToRichEdit(pBuffer, (COLORREF)0, g_pDlg->m_output);
				else
				    WriteOutputToRichEdit(pBuffer, aGlobalColor[nRank%NUM_GLOBAL_COLORS], g_pDlg->m_output);
				ReleaseMutex(g_hConsoleOutputMutex);
			    }
			}
			n--;
		    }
		}
	    }
	    if (bDeleteOnEmpty && nActive == 0)
	    {
		if (hChildThread != NULL)
		{
		    WaitForSingleObject(hChildThread, 10000);
		    CloseHandle(hChildThread);
		    hChildThread = NULL;
		}
		break;
	    }
	}
    }

    for (i=0; i<nActive; i++)
	beasy_closesocket(bfdActive[i]);
    if (child_abort_bfd == BFD_INVALID_SOCKET)
	SetEvent(g_hListenReleasedEvent);
    else
    {
	beasy_send(child_abort_bfd, "x", 1);
	beasy_closesocket(child_abort_bfd);
    }
    beasy_closesocket(abort_bfd);
    if (hChildThread != NULL)
	CloseHandle(hChildThread);
}

void RedirectIOThread(RedirectIOArg *pArg)
{
    int listen_bfd, client_bfd, signal_bfd, /*abort_bfd,*/ child_abort_bfd = BFD_INVALID_SOCKET;
    int n, i;
    sockaddr addr;
    int len;
    int bfdStopIOSignalSocket;
    char pBuffer[1024];
    DWORD num_read;
    int nRank;
    char cType;
    int nDatalen;
    bool bDeleteOnEmpty = false;
    HANDLE hChildThread = NULL;

    // This is easier than passing these two arguments to all the io threads
    g_pDlg = pArg->pDlg;
    g_hConsoleOutputMutex = pArg->pDlg->m_hConsoleOutputMutex;

    // Create a listener
    if (beasy_create(&listen_bfd, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	char str[256];
	int error = WSAGetLastError();
	sprintf(str, "RedirectIOControlThread: beasy_create listen socket failed: error %d\n", error);
	MessageBox(NULL, str, "Critical Error", MB_OK);
	bsocket_finalize();
	ExitProcess(error);
    }
    blisten(listen_bfd, 5);
    beasy_get_sock_info(listen_bfd, pArg->pDlg->m_pszIOHost, &pArg->pDlg->m_nIOPort);
    g_bfdListen = listen_bfd;

    // Connect a stop socket to myself
    if (beasy_create(&pArg->pDlg->m_bfdStopIOSignalSocket, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	char str[256];
	int error = WSAGetLastError();
	sprintf(str, "beasy_create(m_bfdStopIOSignalSocket) failed, error %d\n", error);fflush(stdout);
	MessageBox(NULL, str, "Critical Error", MB_OK);
	ExitProcess(error);
    }
    if (beasy_connect(pArg->pDlg->m_bfdStopIOSignalSocket, pArg->pDlg->m_pszIOHost, pArg->pDlg->m_nIOPort) == SOCKET_ERROR)
    {
	char str[256];
	int error = WSAGetLastError();
	sprintf(str, "beasy_connect(m_bfdStopIOSignalSocket, %s, %d) failed, error %d\n", pArg->pDlg->m_pszIOHost, pArg->pDlg->m_nIOPort, error);
	MessageBox(NULL, str, "Critical Error", MB_OK);
	ExitProcess(error);
    }
    bfdStopIOSignalSocket = pArg->pDlg->m_bfdStopIOSignalSocket;

    // Accept the connection from myself
    len = sizeof(addr);
    signal_bfd = baccept(listen_bfd, &addr, &len);

    SetEvent(pArg->hReadyEvent); // The waiting thread will delete pArg, so don't use it again after this statement
    pArg = NULL;
    
    bfd_set total_set, readset;
    int bfdActive[FD_SETSIZE];
    int nActive = 0;
    
    BFD_ZERO(&total_set);
    
    BFD_SET(listen_bfd, &total_set);
    BFD_SET(signal_bfd, &total_set);

    while (true)
    {
	readset = total_set;
	n = bselect(0, &readset, NULL, NULL, NULL);
	if (n == SOCKET_ERROR)
	{
	    printf("RedirectIOControlThread: bselect failed, error %d\n", WSAGetLastError());fflush(stdout);
	    break;
	}
	if (n == 0)
	{
	    printf("RedirectIOControlThread: bselect returned zero sockets available\n");fflush(stdout);
	    break;
	}
	else
	{
	    if (BFD_ISSET(signal_bfd, &readset))
	    {
		char c;
		bool bAbortNow = true;
		num_read = beasy_receive(signal_bfd, &c, 1);
		if (num_read == 1)
		{
		    if (c == 0)
		    {
			if (child_abort_bfd != BFD_INVALID_SOCKET)
			    beasy_send(child_abort_bfd, &c, 1);

			if (nActive == 0)
			{
			    if (hChildThread != NULL)
				WaitForSingleObject(hChildThread, 10000);
			    break;
			}
			bDeleteOnEmpty = true;
		    }
		}
		if (bAbortNow)
		{
		    break;
		}
		n--;
	    }
	    if (BFD_ISSET(listen_bfd, &readset))
	    {
		if ((nActive + 3) >= FD_SETSIZE)
		{
		    int temp_bfd;
		    MakeLoop(&temp_bfd, &child_abort_bfd);
		    if (temp_bfd == BFD_INVALID_SOCKET || child_abort_bfd == BFD_INVALID_SOCKET)
		    {
			MessageBox(NULL, "Unable to create a socket", "Critical error", MB_OK);
			break;
		    }
		    DWORD dwThreadId;
		    hChildThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectIOThread2, (LPVOID)temp_bfd, 0, &dwThreadId);
		    if (hChildThread == NULL)
		    {
			MessageBox(NULL, "Unable to create an io thread", "Critical error", MB_OK);
			break;
		    }
		    BFD_CLR(g_bfdListen, &total_set);
		    listen_bfd = BFD_INVALID_SOCKET;
		}
		else
		{
		    client_bfd = beasy_accept(listen_bfd);
		    if (client_bfd == BFD_INVALID_SOCKET)
		    {
			char str[256];
			int error = WSAGetLastError();
			sprintf(str, "RedirectIOControlThread: baccept failed: %d\n", error);
			MessageBox(NULL, str, "Error", MB_OK);
			break;
		    }
		    
		    if (beasy_receive(client_bfd, &cType, sizeof(char)) == SOCKET_ERROR)
			return;
		    
		    if (cType == 0)
		    {
			DWORD dwThreadID;
			if (g_pDlg->m_hRedirectRicheditThread != NULL)
			    TerminateThread(g_pDlg->m_hRedirectRicheditThread, 0);
			g_pDlg->m_hRedirectRicheditThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RedirectRichEdit, (void*)client_bfd, 0, &dwThreadID);
		    }
		    else
		    {
			bfdActive[nActive] = client_bfd;
			BFD_SET(client_bfd, &total_set);
			nActive++;
		    }
		}
		n--;
	    }
	    if (n > 0)
	    {
		for (i=0; n > 0; i++)
		{
		    if (BFD_ISSET(bfdActive[i], &readset))
		    {
			char pTemp[sizeof(int)+sizeof(char)+sizeof(int)];
			num_read = beasy_receive(bfdActive[i], pTemp, sizeof(int)+sizeof(char)+sizeof(int));
			if (num_read == SOCKET_ERROR || num_read == 0)
			{
			    BFD_CLR(bfdActive[i], &total_set);
			    beasy_closesocket(bfdActive[i]);
			    nActive--;
			    bfdActive[i] = bfdActive[nActive];
			    i--;
			    //printf("(-%d)", nActive);fflush(stdout);
			}
			else
			{
			    nDatalen = *(int*)pTemp;
			    cType = pTemp[sizeof(int)];
			    nRank = *(int*)&pTemp[sizeof(int)+sizeof(char)];
			    num_read = beasy_receive(bfdActive[i], pBuffer, nDatalen);
			    if (num_read == SOCKET_ERROR || num_read == 0)
			    {
				BFD_CLR(bfdActive[i], &total_set);
				beasy_closesocket(bfdActive[i]);
				nActive--;
				bfdActive[i] = bfdActive[nActive];
				i--;
				//printf("(-%d)", nActive);fflush(stdout);
			    }
			    else
			    {
				WaitForSingleObject(g_hConsoleOutputMutex, INFINITE);
				pBuffer[num_read] = '\0';
				if (g_pDlg->m_bNoColor)
				    WriteOutputToRichEdit(pBuffer, (COLORREF)0, g_pDlg->m_output);
				else
				    WriteOutputToRichEdit(pBuffer, aGlobalColor[nRank%NUM_GLOBAL_COLORS], g_pDlg->m_output);
				ReleaseMutex(g_hConsoleOutputMutex);
			    }
			}
			n--;
		    }
		}
	    }
	    if (bDeleteOnEmpty && nActive == 0)
	    {
		if (hChildThread != NULL)
		{
		    WaitForSingleObject(hChildThread, 10000);
		    CloseHandle(hChildThread);
		    hChildThread = NULL;
		}
		break;
	    }
	}
    }
    if (child_abort_bfd != BFD_INVALID_SOCKET)
    {
	//printf("signalling child threads to shut down\n");fflush(stdout);
	beasy_send(child_abort_bfd, "x", 1);
	WaitForSingleObject(g_hListenReleasedEvent, 10000);
	beasy_closesocket(g_bfdListen);
    }
    for (i=0; i<nActive; i++)
	beasy_closesocket(bfdActive[i]);
    beasy_closesocket(signal_bfd);
    if (listen_bfd != BFD_INVALID_SOCKET)
	beasy_closesocket(listen_bfd);
    if (hChildThread != NULL)
	CloseHandle(hChildThread);
}
