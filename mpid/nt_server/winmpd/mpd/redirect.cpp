//#include "GetStringOpt.h"
#include "mpdimpl.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "Translate_Error.h"

#undef USE_NEW_WAY
//#define USE_NEW_WAY

#ifdef USE_NEW_WAY
void RedirectSocketThread(RedirectSocketArg *arg)
{
    char pBuffer[1024+sizeof(int)+sizeof(char)+sizeof(int)];
    DWORD num_read, num_written;
    HANDLE hReadEvents[2];
    OVERLAPPED pOvl[2];
    char dummy_char;
    DWORD dummy_num_read;
    DWORD dwRetVal;
    BOOL bResult;

    if (arg->bReadisPipe)
    {
	pOvl[0].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pOvl[0].Internal = 0;
	pOvl[0].InternalHigh = 0;
	pOvl[0].Offset = 0;
	pOvl[0].OffsetHigh = 0;
	hReadEvents[0] = pOvl[0].hEvent;
	bResult = ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, &pOvl[0]);
	if (!bResult) 
	{
	    switch (GetLastError()) 
	    {
	    case ERROR_HANDLE_EOF: 
		// At the end of the file.
		goto QUIT_REDIRECTION;
		break;
	    case ERROR_IO_PENDING: 
		// I/O pending.
		break;
	    }
	}
    }
    else
    {
	hReadEvents[0] = WSACreateEvent();
	WSAEventSelect(arg->bfdRead, hReadEvents[0], FD_READ | FD_CLOSE);
    }

    if (arg->bWriteisPipe)
    {
	pOvl[1].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pOvl[1].Internal = 0;
	pOvl[1].InternalHigh = 0;
	pOvl[1].Offset = 0;
	pOvl[1].OffsetHigh = 0;
	hReadEvents[1] = pOvl[1].hEvent;
	bResult = ReadFile(arg->hWrite, &dummy_char, 1, &dummy_num_read, &pOvl[1]);
	if (!bResult) 
	{
	    switch (GetLastError()) 
	    {
	    case ERROR_HANDLE_EOF: 
		// At the end of the file.
		goto QUIT_REDIRECTION;
		break;
	    case ERROR_IO_PENDING: 
		// I/O pending.
		break;
	    }
	}
    }
    else
    {
	hReadEvents[1] = WSACreateEvent();
	WSAEventSelect(arg->bfdWrite, hReadEvents[1], FD_CLOSE);
    }

    // The format is pBuffer[int nDataLength | char cType | int nRank | char[] data]
    pBuffer[sizeof(int)] = arg->cType; // The type never changes
    *(int*)&pBuffer[sizeof(int)+sizeof(char)] = arg->nRank; // The rank never changes

    while (true)
    {
	dwRetVal = WaitForMultipleObjects(2, hReadEvents, FALSE, INFINITE);
	
	if (dwRetVal == WAIT_OBJECT_0)
	{
	    if (arg->bReadisPipe)
	    {
		if (num_read == 0)
		    goto QUIT_REDIRECTION;

		// Reading from a handle
		if (arg->bWriteisPipe)
		{
		    // Writing to a handle
		    if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
			//break;
			goto QUIT_REDIRECTION;
		}
		else
		{
		    // Writing to a socket
		    *(int*)pBuffer = num_read;
		    if (beasy_send(arg->bfdWrite, pBuffer, num_read + sizeof(int) + sizeof(char) + sizeof(int)) == SOCKET_ERROR)
		    {
			// Kill the process if the socket to redirect output is closed
			if (arg->hProcess != NULL)
			{
			    int error = 1;
			    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			    {
				if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				    error = 0;
			    }
			    if (error)
				TerminateProcess(arg->hProcess, 1);
			}
			//break;
			goto QUIT_REDIRECTION;
		    }
		}

		// Post the next read
		ResetEvent(pOvl[0].hEvent);
		bResult = ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, &pOvl[0]);
		if (!bResult) 
		{
		    switch (GetLastError()) 
		    {
		    case ERROR_HANDLE_EOF: 
			// At the end of the file.
			goto QUIT_REDIRECTION;
			break;
		    case ERROR_IO_PENDING: 
			// I/O pending.
			break;
		    }
		}
	    }
	    else
	    {
		// Reading from a socket
		ResetEvent(hReadEvents[0]);
		while (num_read = beasy_receive_some(arg->bfdRead, pBuffer, 1024))
		{
		    if (num_read == SOCKET_ERROR || num_read == 0)
		    {
			// Kill the process if the socket to redirect input is closed
			if (arg->hProcess != NULL)
			{
			    int error = 1;
			    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			    {
				if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				    error = 0;
			    }
			    if (error)
				TerminateProcess(arg->hProcess, 1);
			}
			break;
		    }
		    if (arg->bWriteisPipe)
		    {
			// Writing to a handle
			if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
			    //break;
			    goto QUIT_REDIRECTION;
		    }
		    else
		    {
			// Writing to a socket
			if (beasy_send(arg->bfdWrite, pBuffer, num_read) == SOCKET_ERROR)
			    //break;
			    goto QUIT_REDIRECTION;
		    }
		}
	    }
	}
	else
	{
	    if (dwRetVal != WAIT_OBJECT_0+1)
	    {
		printf("error %d\n", GetLastError());
		goto QUIT_REDIRECTION;
	    }
	}
	
	dwRetVal = WaitForSingleObject(hReadEvents[1], 0);
	if (dwRetVal == WAIT_OBJECT_0)
	{
	    // redirect handle closed, terminate the process
	    if (arg->hProcess != NULL)
	    {
		int error = 1;
		if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
		{
		    if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
			error = 0;
		}
		if (error)
		    TerminateProcess(arg->hProcess, 1);
	    }
	}
	else
	{
	    if (dwRetVal != WAIT_TIMEOUT)
	    {
		printf("error %d\n", GetLastError());
		goto QUIT_REDIRECTION;
	    }
	}
    }

QUIT_REDIRECTION:
    CloseHandle(hReadEvents[0]);
    CloseHandle(hReadEvents[1]);
    if (arg->bReadisPipe)
	CloseHandle(arg->hRead);
    if (arg->bWriteisPipe)
	CloseHandle(arg->hWrite);
    if (arg->bfdRead != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdRead);
    if (arg->bfdWrite != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdWrite);
    delete arg;
}

void RedirectLockedSocketThread(RedirectSocketArg *arg)
{
    char pBuffer[1024+sizeof(int)+sizeof(char)+sizeof(int)];
    DWORD num_read, num_written;
    HANDLE hReadEvents[2];
    OVERLAPPED pOvl[2];
    char dummy_char;
    DWORD dummy_num_read;
    DWORD dwRetVal;
    BOOL bResult;

    if (arg->bReadisPipe)
    {
	pOvl[0].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pOvl[0].Internal = 0;
	pOvl[0].InternalHigh = 0;
	pOvl[0].Offset = 0;
	pOvl[0].OffsetHigh = 0;
	hReadEvents[0] = pOvl[0].hEvent;
	bResult = ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, &pOvl[0]);
	if (!bResult) 
	{
	    switch (GetLastError()) 
	    {
	    case ERROR_HANDLE_EOF: 
		// At the end of the file.
		goto QUIT_REDIRECTION;
		break;
	    case ERROR_IO_PENDING: 
		// I/O pending.
		break;
	    }
	}
    }
    else
    {
	hReadEvents[0] = WSACreateEvent();
	WSAEventSelect(arg->bfdRead, hReadEvents[0], FD_READ | FD_CLOSE);
    }

    if (arg->bWriteisPipe)
    {
	pOvl[1].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pOvl[1].Internal = 0;
	pOvl[1].InternalHigh = 0;
	pOvl[1].Offset = 0;
	pOvl[1].OffsetHigh = 0;
	hReadEvents[1] = pOvl[1].hEvent;
	bResult = ReadFile(arg->hWrite, &dummy_char, 1, &dummy_num_read, &pOvl[1]);
	if (!bResult) 
	{
	    switch (GetLastError()) 
	    {
	    case ERROR_HANDLE_EOF: 
		// At the end of the file.
		goto QUIT_REDIRECTION;
		break;
	    case ERROR_IO_PENDING: 
		// I/O pending.
		break;
	    }
	}
    }
    else
    {
	hReadEvents[1] = WSACreateEvent();
	WSAEventSelect(arg->bfdWrite, hReadEvents[1], FD_CLOSE);
    }

    // The format is pBuffer[int nDataLength | char cType | int nRank | char[] data]
    pBuffer[sizeof(int)] = arg->cType; // The type never changes
    *(int*)&pBuffer[sizeof(int)+sizeof(char)] = arg->nRank; // The rank never changes

    while (true)
    {
	dwRetVal = WaitForMultipleObjects(2, hReadEvents, FALSE, INFINITE);
	
	if (dwRetVal == WAIT_OBJECT_0)
	{
	    if (arg->bReadisPipe)
	    {
		if (num_read == 0)
		    goto QUIT_REDIRECTION;

		// Reading from a handle
		if (arg->bWriteisPipe)
		{
		    // Writing to a handle
		    if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
			//break;
			goto QUIT_REDIRECTION;
		}
		else
		{
		    // Writing to a socket
		    WaitForSingleObject(arg->hMutex, INFINITE);
		    *(int*)pBuffer = num_read;
		    if (beasy_send(arg->bfdWrite, pBuffer, num_read + sizeof(int) + sizeof(char) + sizeof(int)) == SOCKET_ERROR)
		    {
			// Kill the process if the socket to redirect output is closed
			if (arg->hProcess != NULL)
			{
			    int error = 1;
			    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			    {
				if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				    error = 0;
			    }
			    if (error)
				TerminateProcess(arg->hProcess, 1);
			}
			ReleaseMutex(arg->hMutex);
			//break;
			goto QUIT_REDIRECTION;
		    }
		    ReleaseMutex(arg->hMutex);
		}

		// Post the next read
		ResetEvent(pOvl[0].hEvent);
		bResult = ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, &pOvl[0]);
		if (!bResult) 
		{
		    switch (GetLastError()) 
		    {
		    case ERROR_HANDLE_EOF: 
			// At the end of the file.
			goto QUIT_REDIRECTION;
			break;
		    case ERROR_IO_PENDING: 
			// I/O pending.
			break;
		    }
		}
	    }
	    else
	    {
		// Reading from a socket
		ResetEvent(hReadEvents[0]);
		while (num_read = beasy_receive_some(arg->bfdRead, pBuffer, 1024))
		{
		    if (num_read == SOCKET_ERROR || num_read == 0)
		    {
			// Kill the process if the socket to redirect input is closed
			if (arg->hProcess != NULL)
			{
			    int error = 1;
			    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			    {
				if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				    error = 0;
			    }
			    if (error)
				TerminateProcess(arg->hProcess, 1);
			}
			break;
		    }
		    if (arg->bWriteisPipe)
		    {
			// Writing to a handle
			if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
			    //break;
			    goto QUIT_REDIRECTION;
		    }
		    else
		    {
			// Writing to a socket
			if (beasy_send(arg->bfdWrite, pBuffer, num_read) == SOCKET_ERROR)
			    //break;
			    goto QUIT_REDIRECTION;
		    }
		}
	    }
	}
	else
	{
	    if (dwRetVal != WAIT_OBJECT_0+1)
	    {
		printf("error %d\n", GetLastError());
		goto QUIT_REDIRECTION;
	    }
	}
	
	dwRetVal = WaitForSingleObject(hReadEvents[1], 0);
	if (dwRetVal == WAIT_OBJECT_0)
	{
	    // redirect handle closed, terminate the process
	    if (arg->hProcess != NULL)
	    {
		int error = 1;
		if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
		{
		    if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
			error = 0;
		}
		if (error)
		    TerminateProcess(arg->hProcess, 1);
	    }
	}
	else
	{
	    if (dwRetVal != WAIT_TIMEOUT)
	    {
		printf("error %d\n", GetLastError());
		goto QUIT_REDIRECTION;
	    }
	}
    }

QUIT_REDIRECTION:
    CloseHandle(hReadEvents[0]);
    CloseHandle(hReadEvents[1]);
    if (arg->bReadisPipe)
	CloseHandle(arg->hRead);
    if (arg->bWriteisPipe)
	CloseHandle(arg->hWrite);
    if (arg->bfdRead != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdRead);
    if (arg->bfdWrite != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdWrite);
    if (arg->bFreeMutex)
    {
	WaitForSingleObject(arg->hOtherThread, INFINITE);
	if (arg->bfdWrite != BFD_INVALID_SOCKET)
	{
	    dbg_printf("closing output redirection socket %d, rank %d\n", bget_fd(arg->bfdWrite), arg->nRank);
	    if (beasy_closesocket(arg->bfdWrite) == SOCKET_ERROR)
	    {
		err_printf("ERROR: beasy_closesocket(%d) failed, error %d\n", bget_fd(arg->bfdWrite), WSAGetLastError());
	    }
	}
	if (arg->hMutex != NULL)
	    CloseHandle(arg->hMutex);
    }
    delete arg;
}

#else

void RedirectSocketThread(RedirectSocketArg *arg)
{
    char pBuffer[1024+sizeof(int)+sizeof(char)+sizeof(int)];
    DWORD num_read, num_written;
    
    if (arg->bReadisPipe)
    {
	// The format is pBuffer[int nDataLength | char cType | int nRank | char[] data]
	pBuffer[sizeof(int)] = arg->cType; // The type never changes
	*(int*)&pBuffer[sizeof(int)+sizeof(char)] = arg->nRank; // The rank never changes
	
	while (ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, NULL))
	{
	    if (arg->bWriteisPipe)
	    {
		if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
		    break;
	    }
	    else
	    {
		*(int*)pBuffer = num_read;
		if (beasy_send(arg->bfdWrite, pBuffer, num_read + sizeof(int) + sizeof(char) + sizeof(int)) == SOCKET_ERROR)
		{
		    // Kill the process if the socket to redirect output is closed
		    if (arg->hProcess != NULL)
		    {
			int error = 1;
			if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			{
			    if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				error = 0;
			}
			if (error)
			    TerminateProcess(arg->hProcess, 1);
		    }
		    break;
		}
	    }
	}
    }
    else
    {
	while (num_read = beasy_receive_some(arg->bfdRead, pBuffer, 1024))
	{
	    if (num_read == SOCKET_ERROR || num_read == 0)
	    {
		// Kill the process if the socket to redirect input is closed
		if (arg->hProcess != NULL)
		{
		    int error = 1;
		    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
		    {
			if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
			    error = 0;
		    }
		    if (error)
			TerminateProcess(arg->hProcess, 1);
		}
		break;
	    }
	    if (arg->bWriteisPipe)
	    {
		if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
		    break;
	    }
	    else
	    {
		if (beasy_send(arg->bfdWrite, pBuffer, num_read) == SOCKET_ERROR)
		    break;
	    }
	}
    }
    if (arg->bReadisPipe)
	CloseHandle(arg->hRead);
    if (arg->bWriteisPipe)
	CloseHandle(arg->hWrite);
    if (arg->bfdRead != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdRead);
    if (arg->bfdWrite != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdWrite);
    delete arg;
}

void RedirectLockedSocketThread(RedirectSocketArg *arg)
{
    char pBuffer[1024+sizeof(int)+sizeof(char)+sizeof(int)];
    DWORD num_read, num_written;

    if (arg->bReadisPipe)
    {
	// The format is pBuffer[int nDataLength | char cType | int nRank | char[] data]
	pBuffer[sizeof(int)] = arg->cType; // The type never changes
	*(int*)&pBuffer[sizeof(int)+sizeof(char)] = arg->nRank; // The rank never changes
	
	while (ReadFile(arg->hRead, &pBuffer[sizeof(int)+sizeof(char)+sizeof(int)], 1024, &num_read, NULL))
	{
	    if (arg->bWriteisPipe)
	    {
		if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
		    break;
	    }
	    else
	    {
		if (num_read == 0)
		    break;
		WaitForSingleObject(arg->hMutex, INFINITE);
		*(int*)pBuffer = num_read;
		if (beasy_send(arg->bfdWrite, pBuffer, num_read + sizeof(int) + sizeof(char) + sizeof(int)) == SOCKET_ERROR)
		{
		    // Kill the process if the socket to redirect output is closed
		    if (arg->hProcess != NULL)
		    {
			int error = 1;
			if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
			{
			    if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
				error = 0;
			}
			if (error)
			    TerminateProcess(arg->hProcess, 1);
		    }
		    ReleaseMutex(arg->hMutex);
		    break;
		}
		ReleaseMutex(arg->hMutex);
	    }
	}
    }
    else
    {
	while (num_read = beasy_receive_some(arg->bfdRead, pBuffer, 1024))
	{
	    if (num_read == SOCKET_ERROR || num_read == 0)
	    {
		// Kill the process if the socket to redirect input is closed
		if (arg->hProcess != NULL)
		{
		    int error = 1;
		    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, arg->dwPid))
		    {
			if (WaitForSingleObject(arg->hProcess, 500) == WAIT_OBJECT_0)
			    error = 0;
		    }
		    if (error)
			TerminateProcess(arg->hProcess, 1);
		}
		break;
	    }
	    if (arg->bWriteisPipe)
	    {
		if (!WriteFile(arg->hWrite, pBuffer, num_read, &num_written, NULL))
		    break;
	    }
	    else
	    {
		if (beasy_send(arg->bfdWrite, pBuffer, num_read) == SOCKET_ERROR)
		    break;
	    }
	}
    }
    if (arg->bReadisPipe)
	CloseHandle(arg->hRead);
    if (arg->bWriteisPipe)
	CloseHandle(arg->hWrite);
    if (arg->bfdRead != BFD_INVALID_SOCKET)
	beasy_closesocket(arg->bfdRead);
    if (arg->bFreeMutex)
    {
	WaitForSingleObject(arg->hOtherThread, INFINITE);
	if (arg->bfdWrite != BFD_INVALID_SOCKET)
	{
	    dbg_printf("closing output redirection socket %d, rank %d\n", bget_fd(arg->bfdWrite), arg->nRank);
	    if (beasy_closesocket(arg->bfdWrite) == SOCKET_ERROR)
	    {
		err_printf("ERROR: beasy_closesocket(%d) failed, error %d\n", bget_fd(arg->bfdWrite), WSAGetLastError());
	    }
	}
	if (arg->hMutex != NULL)
	    CloseHandle(arg->hMutex);
    }
    delete arg;
}
#endif

