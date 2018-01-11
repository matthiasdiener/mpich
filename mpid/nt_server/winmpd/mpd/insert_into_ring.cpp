#include "mpdimpl.h"

bool InsertIntoRing(char *pszHost)
{
    // Note: When this function returns true, it means that the insert operation
    // has successfully started, not that it has completed.  It could fail later.

    PUSH_FUNC("InsertIntoRing");

    if (pszHost == NULL || pszHost[0] == '\0')
    {
	//dbg_printf("InsertIntoRing called with no host name\n");
	POP_FUNC();
	return false;
    }

    dbg_printf("InsertIntoRing: inserting at '%s', please wait\n", pszHost);

    MPD_Context *pContext = new MPD_Context;
    beasy_create(&pContext->bfd, ADDR_ANY, INADDR_ANY);
    if (beasy_connect(pContext->bfd, pszHost, g_nPort) == SOCKET_ERROR)
    {
	err_printf("InsertIntoRing: beasy_connect(%d, %s:%d) failed, error %d\n", bget_fd(pContext->bfd), pszHost, g_nPort, WSAGetLastError());
	delete pContext;
	return false;
    }
    strncpy(pContext->pszHost, pszHost, MAX_HOST_LENGTH);
    pContext->pNext = g_pList;
    g_pList = pContext;

    pContext->nType = MPD_SOCKET;
    pContext->nState = MPD_IDLE;
    pContext->nLLState = MPD_AUTHENTICATE_READING_APPEND;
    pContext->nConnectingState = MPD_INSERTING;
    DoReadSet(pContext->bfd);

    POP_FUNC();
    return true;
}
