#include "mpdimpl.h"
#include "Service.h"

HANDLE g_hBombDiffuseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hBombThread = NULL;

void BombThread()
{
    if (WaitForSingleObject(g_hBombDiffuseEvent, 25000) == WAIT_TIMEOUT)
    {
	dbg_printf("BombThread timed out, exiting.\n");
	ExitProcess(-1);
    }
}

//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop()
{
    DWORD dwThreadID;
    char str[10] = "no";
    if (ReadMPDRegistry("RevertToMultiUser", str, false))
    {
	if (stricmp(str, "yes") == 0)
	    WriteMPDRegistry("SingleUser", "no");
	DeleteMPDRegistry("RevertToMultiUser");
    }
    g_hBombThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BombThread, NULL, 0, &dwThreadID);
    SetEvent(g_hCommPortEvent);
}
