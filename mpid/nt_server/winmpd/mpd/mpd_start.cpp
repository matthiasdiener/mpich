#include "mpdimpl.h"
#include "Service.h"
#include "database.h"

void StdinThread()
{
    char str[MAX_CMD_LENGTH];
    while (gets(str))
    {
	if (strcmp(str, "quit") == 0)
	{
	    if (ReadMPDRegistry("RevertToMultiUser", str, false))
	    {
		if (stricmp(str, "yes") == 0)
		    WriteMPDRegistry("SingleUser", "no");
		DeleteMPDRegistry("RevertToMultiUser");
	    }
	    ExitProcess(0);
	}
	if (strcmp(str, "stop") == 0)
	{
	    ServiceStop();
	}
	if (strcmp(str, "print") == 0)
	{
	    PrintState(stdout);
	}
    }
}

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)
{
    int run_retval = RUN_EXIT;
    HANDLE stdin_thread = NULL;
    PUSH_FUNC("ServiceStart");
    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
	POP_FUNC();
	return;
    }
    
    // Load the path to the service executable
    char szExe[1024];
    HMODULE hModule = GetModuleHandle(NULL);
    if (!GetModuleFileName(hModule, szExe, 1024)) 
	strcpy(szExe, "mpd.exe");
    WriteMPDRegistry("path", szExe);
    
    // Initialize
    dbs_init();

    if (!bDebug)
    {
#ifndef _DEBUG
	// If we are not running in debug mode and this is the release
	// build then set the error mode to prevent popup message boxes.
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
//#pragma message("Release build suppresses mpd error message popup boxes")
#endif
	bsocket_init();
	ParseRegistry(false);
    }

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
	bsocket_finalize();
	dbs_finalize();
	POP_FUNC();
	return;
    }
    
    do
    {
	// Setup the listener
	if (g_nPort == 0)
	{
	    err_printf("the listening port cannot be zero.\n");
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(-1);
	}
	MPD_Context *pListenContext = new MPD_Context;
	if (beasy_create(&pListenContext->bfd, g_nPort, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create listen socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	blisten(pListenContext->bfd, 20);
	pListenContext->nCurPos = 0;
	pListenContext->nState = MPD_IDLE;
	pListenContext->nType = MPD_LISTENER;
	strncpy(pListenContext->pszHost, g_pszHost, MAX_HOST_LENGTH);
	pListenContext->pszHost[MAX_HOST_LENGTH-1] = '\0';
	pListenContext->pNext = g_pList;
	g_pList = pListenContext;
	beasy_get_ip(&g_nIP);
	beasy_get_ip_string(g_pszIP);
	
	// report the status to the service control manager.
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    return;
	}
	
	// Setup the signal socket
	if (beasy_create(&g_bfdSignal, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create signal socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	if (beasy_connect(g_bfdSignal, g_pszHost, g_nPort) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_connect signal socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	sockaddr addr;
	int addrlen = sizeof(addr);
	MPD_Context *pSignalContext = new MPD_Context;
	pSignalContext->bfd = baccept(pListenContext->bfd, &addr, &addrlen);
	if(pSignalContext->bfd == INVALID_SOCKET)
	{
	    int error = WSAGetLastError();
	    err_printf("ServiceStart: baccept failed: %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	pSignalContext->nCurPos = 0;
	pSignalContext->nState = MPD_IDLE;
	pSignalContext->nType = MPD_SIGNALLER;
	strncpy(pSignalContext->pszHost, g_pszHost, MAX_HOST_LENGTH);
	pSignalContext->pszHost[MAX_HOST_LENGTH-1] = '\0';
	pSignalContext->pNext = g_pList;
	g_pList = pSignalContext;
	
	// Setup the ReSelect socket
	if (beasy_create(&g_bfdReSelect, ADDR_ANY, INADDR_ANY) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_create reselect socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	if (beasy_connect(g_bfdReSelect, g_pszHost, g_nPort) == SOCKET_ERROR)
	{
	    int error = WSAGetLastError();
	    err_printf("beasy_connect reselect socket failed: error %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	
	addrlen = sizeof(addr);
	MPD_Context *pReSelectContext = new MPD_Context;
	pReSelectContext->bfd = baccept(pListenContext->bfd, &addr, &addrlen);
	if (pReSelectContext->bfd == INVALID_SOCKET)
	{
	    int error = WSAGetLastError();
	    err_printf("baccept failed: %d\n", error);
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    ExitProcess(error);
	}
	pReSelectContext->nCurPos = 0;
	pReSelectContext->nState = MPD_IDLE;
	pReSelectContext->nType = MPD_RESELECTOR;
	strncpy(pReSelectContext->pszHost, g_pszHost, MAX_HOST_LENGTH);
	pReSelectContext->pszHost[MAX_HOST_LENGTH-1] = '\0';
	pReSelectContext->pNext = g_pList;
	g_pList = pReSelectContext;
	
	BFD_ZERO(&g_ReadSet);
	BFD_ZERO(&g_WriteSet);
	g_nActiveW = 0;
	g_nActiveR = 0;
	DoReadSet(pSignalContext->bfd);
	DoReadSet(pReSelectContext->bfd);
	DoReadSet(pListenContext->bfd);
	g_maxfds = BFD_MAX(pListenContext->bfd, pSignalContext->bfd);
	
	// report the status to the service control manager.
	if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0))
	{
	    bsocket_finalize();
	    dbs_finalize();
	    POP_FUNC();
	    return;
	}
	
	
	////////////////////////////////////////////////////////
	//
	// Service is now running, perform work until shutdown
	//
	
	AddInfoToMessageLog("MPICH_MPD Daemon service started.");
	
	if (stdin_thread == NULL)
	{
	    stdin_thread = CreateThread(
		NULL, 0,
		(LPTHREAD_START_ROUTINE)StdinThread,
		NULL, 0, NULL);
	}
	
	if (ConnectToSelf() == false)
	{
	    err_printf("ServiceStart: ConnectToSelf failed\n");
	    ExitProcess(0);
	}
	if (!g_bStartAlone)
	{
	    if (stricmp(g_pszHost, g_pszInsertHost))
	    {
		if (InsertIntoRing(g_pszInsertHost) == false)
		{
		    if (stricmp(g_pszHost, g_pszInsertHost2))
			InsertIntoRing(g_pszInsertHost2);
		}
	    }
	}
	
	run_retval = Run();
	if (run_retval == RUN_RESTART)
	{
	    warning_printf("Socket connections lost, restarting mpd.");
	}
	
	while (g_pList)
	    RemoveContext(g_pList);
	
    } while (run_retval == RUN_RESTART);
    
    TerminateThread(stdin_thread, 0);
    CloseHandle(stdin_thread);
    
    bsocket_finalize();
    dbs_finalize();
    AddInfoToMessageLog("MPICH_MPD Daemon service stopped.");

    SetEvent(g_hBombDiffuseEvent);
    if (g_hBombThread != NULL)
    {
	if (WaitForSingleObject(g_hBombThread, 5000) == WAIT_TIMEOUT)
	{
	    TerminateThread(g_hBombThread, 0);
	}
	CloseHandle(g_hBombThread);
    }
    CloseHandle(g_hBombDiffuseEvent);

    POP_FUNC();
}
