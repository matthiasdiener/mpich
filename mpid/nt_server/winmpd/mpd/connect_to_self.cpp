#include "mpdimpl.h"

bool ConnectToSelf()
{
    int bfdListen, nPort;
    char host[MAX_HOST_LENGTH];
    sockaddr addr;
    int len = sizeof(addr);

    PUSH_FUNC("ConnectToSelf");

    // Create a bogus listener
    if (beasy_create(&bfdListen, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_create failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    if (blisten(bfdListen, 5) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: blisten failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    beasy_get_sock_info(bfdListen, host, &nPort);

    // Create a new right context
    g_pRightContext = new MPD_Context;
    if (beasy_create(&g_pRightContext->bfd, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_create failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }
    if (beasy_connect(g_pRightContext->bfd, host, nPort) == SOCKET_ERROR)
    {
	err_printf("ConnectToSelf: beasy_connect failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }

    // Create a new left context
    g_pLeftContext = new MPD_Context;
    g_pLeftContext->bfd = baccept(bfdListen, &addr, &len);
    if (g_pLeftContext->bfd == BFD_INVALID_SOCKET)
    {
	err_printf("ConnectToSelf: baccept failed, error %d\n", WSAGetLastError());
	POP_FUNC();
	return false;
    }

    // Close the listener
    beasy_closesocket(bfdListen);

    // Initialize the new contexts
    strncpy(g_pRightContext->pszHost, host, MAX_HOST_LENGTH);
    strncpy(g_pszRightHost, host, MAX_HOST_LENGTH);
    g_pRightContext->nCurPos = 0;
    g_pRightContext->nState = MPD_IDLE;
    g_pRightContext->nLLState = MPD_READING_CMD;
    g_pRightContext->nType = MPD_RIGHT_SOCKET;
    g_pRightContext->pNext = g_pList;
    g_pList = g_pRightContext;
    DoReadSet(g_pRightContext->bfd);
    g_maxfds = BFD_MAX(g_pRightContext->bfd, g_maxfds);

    strncpy(g_pLeftContext->pszHost, host, MAX_HOST_LENGTH);
    strncpy(g_pszLeftHost, host, MAX_HOST_LENGTH);
    g_pLeftContext->nCurPos = 0;
    g_pLeftContext->nState = MPD_IDLE;
    g_pLeftContext->nLLState = MPD_READING_CMD;
    g_pLeftContext->nType = MPD_LEFT_SOCKET;
    g_pLeftContext->pNext = g_pList;
    g_pList = g_pLeftContext;
    DoReadSet(g_pLeftContext->bfd);
    g_maxfds = BFD_MAX(g_pLeftContext->bfd, g_maxfds);

    //dbg_printf("ConnectToSelf succeeded\n");
    POP_FUNC();
    return true;
}
