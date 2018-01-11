#include "mpdimpl.h"

bool Extract(bool bReConnect)
{
    // Note: Calling Extract with bReConnect = false will cause the mpd to exit
    // after the extract operation finishes.

    PUSH_FUNC("Extract");
    if (g_pLeftContext == NULL || g_pRightContext == NULL)
    {
	POP_FUNC();
	return false;
    }
    if (strcmp(g_pLeftContext->pszHost, g_pszHost) == 0)
    {
	if (strcmp(g_pRightContext->pszHost, g_pszHost))
	{
	    err_printf("invalid state: g_pszHost = %s, pszLeftHost = %s, pszRightHost = %s\n", g_pszHost, 
		g_pLeftContext->pszHost, g_pRightContext->pszHost);
	    POP_FUNC();
	    return false;
	}
	if (!bReConnect)
	{
	    RemoveContext(g_pLeftContext);
	    RemoveContext(g_pRightContext);
	    SignalExit();
	    SignalExit();
	}
	POP_FUNC();
	return true;
    }
    if (strcmp(g_pRightContext->pszHost, g_pszHost) == 0)
    {
	err_printf("invalid state: g_pszHost = %s, pszLeftHost = %s, pszRightHost = %s\n", g_pszHost, 
	    g_pLeftContext->pszHost, g_pRightContext->pszHost);
	POP_FUNC();
	return false;
    }

    EnqueueWrite(g_pRightContext, "connect left", MPD_WRITING_CONNECT_LEFT);
    if (bReConnect)
    {
	EnqueueWrite(g_pRightContext, g_pLeftContext->pszHost, MPD_WRITING_NEW_LEFT_HOST);
	EnqueueWrite(g_pLeftContext, "done", MPD_WRITING_DONE);
    }
    else
    {
	EnqueueWrite(g_pRightContext, g_pLeftContext->pszHost, MPD_WRITING_NEW_LEFT_HOST_EXIT);
	EnqueueWrite(g_pLeftContext, "done", MPD_WRITING_DONE_EXIT);
    }

    if (bReConnect)
    {
	if (!ConnectToSelf())
	{
	    err_printf("ConnectToSelf failed\n");
	}
    }

    POP_FUNC();
    return true;
}
