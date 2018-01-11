#include "nt_global_cpp.h"
#include "bnrfunctions.h"
#include <stdio.h>
#include <stdlib.h>

int g_NumCommPortThreads = 2;
#define EXIT_WORKER_KEY	-1

HANDLE g_hCommPortThread, g_hCommPort;
HANDLE g_hCommPortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE g_hAddSocketMutex = CreateMutex(NULL, FALSE, NULL);
int g_nCommPortCommand;

// Function name	: CommPortWorkerThread
// Description	    : 
// Return type		: void 
void CommPortWorkerThread()
{
	DWORD dwKey, nBytes;
	OVERLAPPED *p_Ovl;
	int error;

	while (true)
	{
		if (GetQueuedCompletionStatus(g_hCommPort, &nBytes, &dwKey, &p_Ovl, INFINITE))
		{
			DPRINTF(("COMMPORT::%d bytes on socket %d\n", nBytes, dwKey));
			if (dwKey == EXIT_WORKER_KEY)
				ExitThread(0);
			if (nBytes)
			{
				//printf("COMMPORT::%d bytes on socket %d\n", nBytes, dwKey);fflush(stdout);
				g_pProcTable[dwKey].msg.nRemaining -= nBytes;
				switch(g_pProcTable[dwKey].msg.state)
				{
				case NT_MSG_READING_TAG:
					if (g_pProcTable[dwKey].msg.nRemaining)
					{
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock, 
							(char*)&g_pProcTable[dwKey].msg.tag + sizeof(int) - g_pProcTable[dwKey].msg.nRemaining,
							g_pProcTable[dwKey].msg.nRemaining, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(tag) from socket %d failed", dwKey);
						}
					}
					else
					{
						g_pProcTable[dwKey].msg.state = NT_MSG_READING_LENGTH;
						g_pProcTable[dwKey].msg.nRemaining = sizeof(int);
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock,
							(char*)&g_pProcTable[dwKey].msg.length,
							sizeof(int), &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(length) from socket %d failed", dwKey);
						}
					}
					break;
				case NT_MSG_READING_LENGTH:
					if (g_pProcTable[dwKey].msg.nRemaining)
					{
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock, 
							(char*)&g_pProcTable[dwKey].msg.length + sizeof(int) - g_pProcTable[dwKey].msg.nRemaining,
							g_pProcTable[dwKey].msg.nRemaining, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(length) from socket %d failed", dwKey);
						}
					}
					else
					{
						g_pProcTable[dwKey].msg.buffer = g_MsgQueue.GetBufferToFill(g_pProcTable[dwKey].msg.tag, g_pProcTable[dwKey].msg.length, dwKey, &g_pProcTable[dwKey].msg.pElement);
						g_pProcTable[dwKey].msg.nRemaining = g_pProcTable[dwKey].msg.length;
						g_pProcTable[dwKey].msg.state = NT_MSG_READING_BUFFER;
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock, g_pProcTable[dwKey].msg.buffer, g_pProcTable[dwKey].msg.length, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error == ERROR_NO_SYSTEM_RESOURCES)
							{
								int n = g_pProcTable[dwKey].msg.length / 2;
								while (error == ERROR_NO_SYSTEM_RESOURCES)
								{
									if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock, g_pProcTable[dwKey].msg.buffer,
										n, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
									{
										error = GetLastError();
									}
									else
										error = ERROR_SUCCESS;
									n = n/2;
									if (n == 0)
										MakeErrMsg(1, "Not enough system resources available to post a read from socket %d\n", dwKey);
								}
								if (error != ERROR_SUCCESS && error != ERROR_IO_PENDING)
									MakeErrMsg(error, "CommPortWorkerThread:Post read(buffer[%d]) from socket %d failed", n*2, dwKey);
							}
							else if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(buffer[%d]) from socket %d failed", g_pProcTable[dwKey].msg.length, dwKey);
						}
					}
					break;
				case NT_MSG_READING_BUFFER:
					if (g_pProcTable[dwKey].msg.nRemaining)
					{
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock, 
							&(((char*)g_pProcTable[dwKey].msg.buffer)[g_pProcTable[dwKey].msg.length - g_pProcTable[dwKey].msg.nRemaining]),
							g_pProcTable[dwKey].msg.nRemaining, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error == ERROR_NO_SYSTEM_RESOURCES)
							{
								int n = g_pProcTable[dwKey].msg.nRemaining / 2;
								while (error == ERROR_NO_SYSTEM_RESOURCES)
								{
									if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock,
										&(((char*)g_pProcTable[dwKey].msg.buffer)[g_pProcTable[dwKey].msg.length - g_pProcTable[dwKey].msg.nRemaining]),
										n, &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
									{
										error = GetLastError();
									}
									else
										error = ERROR_SUCCESS;
									n = n/2;
									if (n == 0)
										MakeErrMsg(1, "Not enough system resources available to post a read from socket %d\n", dwKey);
								}
								if (error != ERROR_SUCCESS && error != ERROR_IO_PENDING)
									MakeErrMsg(error, "CommPortWorkerThread:Post read(buffer[%d]) from socket %d failed", n*2, dwKey);
							}
							else if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(buffer[%d]) from socket %d failed", g_pProcTable[dwKey].msg.length, dwKey);
						}
					}
					else
					{
						g_MsgQueue.SetElementEvent(g_pProcTable[dwKey].msg.pElement);

						g_pProcTable[dwKey].msg.state = NT_MSG_READING_TAG;
						g_pProcTable[dwKey].msg.nRemaining = sizeof(int);
						g_pProcTable[dwKey].msg.ovl.Offset = 0;
						g_pProcTable[dwKey].msg.ovl.OffsetHigh = 0;
						if (!ReadFile((HANDLE)g_pProcTable[dwKey].sock,
							(char*)&g_pProcTable[dwKey].msg.tag,
							sizeof(int), &g_pProcTable[dwKey].msg.nRead, &g_pProcTable[dwKey].msg.ovl))
						{
							error = GetLastError();
							if (error != ERROR_IO_PENDING)
								MakeErrMsg(error, "CommPortWorkerThread:Post read(tag) from socket %d failed", dwKey);
						}
					}
					break;
				default:
					break;
				}
			}
			else
			{
				NT_Tcp_closesocket(g_pProcTable[dwKey].sock, g_pProcTable[dwKey].sock_event);
				g_pProcTable[dwKey].sock = INVALID_SOCKET;
				g_pProcTable[dwKey].sock_event = NULL;
			}
		}
		else
		{
			if (!g_bInNT_ipvishm_End)
				nt_error("GetQueuedCompletionStatus failed", GetLastError());
		}
	}
}

// Function name	: CommPortThread
// Description	    : 
// Return type		: void 
// Argument         : HANDLE hReadyEvent
void CommPortThread(HANDLE hReadyEvent)
{
	SOCKET listen_socket;
	HANDLE ahEvent[2];				// array of events to wait on
	int error = 0, num_handles=2;
	SOCKET temp_socket;
	WSAEVENT temp_event;
	DWORD ret_val;
	int remote_iproc;
	int i;
	BOOL opt;
	char add_socket_ack;
	DWORD dwThreadID;

	ahEvent[0] = g_hCommPortEvent;

	// create a listening socket
	if (error = NT_Tcp_create_bind_socket(&listen_socket, &ahEvent[1]))
		nt_error("CommPortThread: NT_Tcp_create_bind_socket failed", error);

	// associate listen_socket_event with listen_socket
	if (WSAEventSelect(listen_socket, ahEvent[1], FD_ACCEPT) == SOCKET_ERROR)
		nt_error("CommPortThread: WSAEventSelect failed for listen_socket", 1);

	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
		nt_error("CommPortThread: listen failed", WSAGetLastError());

	// get the port and local hostname for the listening socket
	if (error = NT_Tcp_get_sock_info(listen_socket, g_pProcTable[g_nIproc].host, &g_pProcTable[g_nIproc].listen_port))
		nt_error("CommPortThread: Unable to get host and port of listening socket", error);

	// Create the completion port
	g_hCommPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, g_NumCommPortThreads);
	if (g_hCommPort == NULL)
		nt_error("CommPortThread: CreateIoCompletionPort failed", GetLastError());

	// Start the completion port threads
	for (i=0; i<g_NumCommPortThreads; i++)
	{
	    HANDLE hWorkerThread;
	    hWorkerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CommPortWorkerThread, NULL, NT_THREAD_STACK_SIZE, &dwThreadID);
	    if (hWorkerThread == NULL)
		nt_error("CommPortThread: CreateThread(CommPortWorkerThread) failed", GetLastError());
	    CloseHandle(hWorkerThread);
	}

	// Signal that the port number is valid
	if (!SetEvent(hReadyEvent))
		nt_error("CommPortThread: SetEvent(hReadyEvent) failed", GetLastError());

	while (true)
	{
		ret_val = WaitForMultipleObjects(num_handles, ahEvent, FALSE, INFINITE);
		if (ret_val != WAIT_OBJECT_0 && ret_val != WAIT_OBJECT_0+1)
		{
			nt_error("CommPortThread: Wait failed", GetLastError());
			return;
		}

		// Event[0] is the event used by other threads in this process to communicate with this thread
		if (WaitForSingleObject(ahEvent[0], 0) == WAIT_OBJECT_0)
		{
			switch (g_nCommPortCommand)
			{
			case NT_COMM_CMD_EXIT:
				DPRINTF(("process %d: Exit command.\n", g_nIproc));
				for (i=0; i<g_NumCommPortThreads; i++)
					PostQueuedCompletionStatus(g_hCommPort, 0, EXIT_WORKER_KEY, NULL);
				CloseHandle(g_hAddSocketMutex);
				CloseHandle(g_hCommPortEvent); 
				CloseHandle(g_hCommPort);
				closesocket(listen_socket);
				WSACloseEvent(ahEvent[1]);
				ExitThread(0);
				break;
			default:
				nt_error("Invalid command sent to CommPortThread", g_nCommPortCommand);
				break;
			}
		}

		// Event[1] is the listen socket event, which is signalled when other processes whish to establish a socket connection with this process
		if (WaitForSingleObject(ahEvent[1], 0) == WAIT_OBJECT_0)
		{
			///DPRINTF(("process %d: listen_socket signalled.\n", g_nIproc));
			i=0;
			opt = TRUE;
			// Something in my code is causing the listen_socket event to fail to be reset by the accept call.
			// For now I manually reset it here.
			WSAResetEvent(ahEvent[1]);
			temp_socket = accept(listen_socket, NULL, NULL);
			if (temp_socket != INVALID_SOCKET)
			{
				// Create an event and associate it with the newly accepted socket
				//if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
					//nt_error("setsockopt failed", WSAGetLastError());
				if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
				{
				    error = WSAGetLastError();
				    if (error == WSAENOBUFS)
				    {
					Sleep(250);
					if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
					{
					    error = WSAGetLastError();
					    if (error == WSAENOBUFS)
					    {
						Sleep(250);
						if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
						{
						    error = WSAGetLastError();
						    if (error != WSAENOBUFS)
							nt_error("setsockopt failed in CommPortThread", error);
						}
					    }
					    else
						nt_error("setsockopt failed in CommPortThread", error);
					}
				    }
				    else
					nt_error("setsockopt failed in CommPortThread", error);
				}

				if ((temp_event = WSACreateEvent()) == WSA_INVALID_EVENT)
					nt_error("WSACreateEvent failed after accepting socket", WSAGetLastError());
				if (WSAEventSelect(temp_socket, temp_event, FD_READ | FD_CLOSE) == SOCKET_ERROR)
					nt_error("WSAEventSelect failed after accepting socket", WSAGetLastError());
				
				// Receive the rank of the remote process
				if (ret_val = ReceiveBlocking(temp_socket, temp_event, (char*)&remote_iproc, sizeof(int), 0))
					nt_error("ReceiveBlocking remote_iproc failed after accepting socket", ret_val);
				
				if (remote_iproc >= 0 && remote_iproc < g_nNproc)
				{
					if (WaitForSingleObject(g_hAddSocketMutex, 0) == WAIT_OBJECT_0)
					{
						if (g_pProcTable[remote_iproc].sock == INVALID_SOCKET)
						{
							add_socket_ack = 1;
							if (SendBlocking(temp_socket, &add_socket_ack, 1, 0) == SOCKET_ERROR)
								MakeErrMsg(WSAGetLastError(), "send add_socket_ack(1) failed for socket %d", remote_iproc);

							// Insert the information in g_pProcTable
							g_pProcTable[remote_iproc].sock_event = temp_event;
							g_pProcTable[remote_iproc].sock = temp_socket;

							// Associate the socket with the completion port
							if (CreateIoCompletionPort((HANDLE)temp_socket, g_hCommPort, remote_iproc, g_NumCommPortThreads) == NULL)
								nt_error("Unable to associate completion port with socket", GetLastError());

							// Post the first read from the socket
							g_pProcTable[remote_iproc].msg.ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
							if (g_pProcTable[remote_iproc].msg.ovl.hEvent == NULL)
								MakeErrMsg(GetLastError(), "CommPortThread:CreateEvent failed for %d event", remote_iproc);
							g_pProcTable[remote_iproc].msg.state = NT_MSG_READING_TAG;
							g_pProcTable[remote_iproc].msg.nRemaining = sizeof(int);
							g_pProcTable[remote_iproc].msg.ovl.Offset = 0;
							g_pProcTable[remote_iproc].msg.ovl.OffsetHigh = 0;
							g_pProcTable[remote_iproc].msg.ovl.Internal = 0;
							g_pProcTable[remote_iproc].msg.ovl.InternalHigh = 0;
							if (!ReadFile((HANDLE)temp_socket, &(g_pProcTable[remote_iproc].msg.tag), sizeof(int), &(g_pProcTable[remote_iproc].msg.nRead), &(g_pProcTable[remote_iproc].msg.ovl)))
							{
								int error = GetLastError();
								if (error != ERROR_IO_PENDING)
									MakeErrMsg(error, "CommPortThread:First posted read from socket %d failed", remote_iproc);
							}

							DPRINTF(("process %d: socket accepted and inserted in location %d, no race condition\n", g_nIproc, remote_iproc));
						}
						else
						{
							add_socket_ack = 0;
							if (SendBlocking(temp_socket, &add_socket_ack, 1, 0) == SOCKET_ERROR)
								MakeErrMsg(WSAGetLastError(), "send add_socket_ack(0) failed for socket %d", remote_iproc);
							NT_Tcp_closesocket(temp_socket, temp_event);

							DPRINTF(("process %d: socket closed, valid socket already in location %d", g_nIproc, remote_iproc));
						}
						ReleaseMutex(g_hAddSocketMutex);
					}
					else
					{
						if (g_nIproc > remote_iproc)
						{
							add_socket_ack = 1;
							if (SendBlocking(temp_socket, &add_socket_ack, 1, 0) == SOCKET_ERROR)
								MakeErrMsg(WSAGetLastError(), "send add_socket_ack(1) failed for socket %d", remote_iproc);
						
							// Insert the information in g_pProcTable
							g_pProcTable[remote_iproc].sock_event = temp_event;
							g_pProcTable[remote_iproc].sock = temp_socket;
							
							// Associate the socket with the completion port
							if (CreateIoCompletionPort((HANDLE)temp_socket, g_hCommPort, remote_iproc, g_NumCommPortThreads) == NULL)
								nt_error("Unable to associate completion port with socket", GetLastError());

							// Post the first read from the socket
							g_pProcTable[remote_iproc].msg.ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
							if (g_pProcTable[remote_iproc].msg.ovl.hEvent == NULL)
								MakeErrMsg(GetLastError(), "CommPortThread:CreateEvent failed for %d event", remote_iproc);
							g_pProcTable[remote_iproc].msg.state = NT_MSG_READING_TAG;
							g_pProcTable[remote_iproc].msg.nRemaining = sizeof(int);
							g_pProcTable[remote_iproc].msg.ovl.Offset = 0;
							g_pProcTable[remote_iproc].msg.ovl.OffsetHigh = 0;
							g_pProcTable[remote_iproc].msg.ovl.Internal = 0;
							g_pProcTable[remote_iproc].msg.ovl.InternalHigh = 0;
							if (!ReadFile((HANDLE)temp_socket, &(g_pProcTable[remote_iproc].msg.tag), sizeof(int), &(g_pProcTable[remote_iproc].msg.nRead), &(g_pProcTable[remote_iproc].msg.ovl)))
							{
								int error = GetLastError();
								if (error != ERROR_IO_PENDING)
									MakeErrMsg(error, "CommPortThread:First posted read from socket %d failed", remote_iproc);
							}

							DPRINTF(("process %d: %d > %d, socket accepted and inserted in location %d\n", g_nIproc, g_nIproc, remote_iproc, remote_iproc));
						}
						else
						{
							add_socket_ack = 0;
							if (SendBlocking(temp_socket, &add_socket_ack, 1, 0) == SOCKET_ERROR)
								MakeErrMsg(1, "send add_socket_ack(0) failed for socket %d", remote_iproc);
							NT_Tcp_closesocket(temp_socket, temp_event);

							DPRINTF(("process %d: socket closed, %d > %d", g_nIproc, g_nIproc, remote_iproc));
						}
					}
				}
				else
				{
					MakeErrMsg(1, "CommPortThread: Process out of range, remote_iproc: %d\n", remote_iproc);
					return;
				}
			}
			else
			{
				error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					nt_error("CommPortThread: accept failed", error);
					return;
				}
			}
		}

	}
}

// Function name	: ConnectTo
// Description	    : 
// Return type		: int 
// Argument         : int remote_iproc
int ConnectTo(int remote_iproc)
{
	SOCKET temp_socket;
	WSAEVENT temp_event;
	char ack = 0;
	int ret_val;
	BOOL opt = TRUE;
	int i=0;
	HOSTENT *hostEnt;
	unsigned long nic_addr = INADDR_ANY;
	int error;

	if (remote_iproc < 0 || remote_iproc >= g_nNproc)
	{
		MakeErrMsg(1, "ConnectTo failed, invalid remote process rank: %d\n", remote_iproc);
		return 0;
	}

	if (WaitForSingleObject(g_hAddSocketMutex, 5000) == WAIT_TIMEOUT)
		MakeErrMsg(1, "ConnectTo %d failed, wait for AddSocketMutex timed out", remote_iproc);

	if (g_pProcTable[remote_iproc].sock != INVALID_SOCKET)
	{
		ReleaseMutex(g_hAddSocketMutex);
		return 1;
	}

	if (g_bUseBNR)
	{
		char pszKey[100], pszValue[100];
		sprintf(pszKey, "ListenHost%d", remote_iproc);
		BNR_Get(g_myBNRgroup, pszKey, g_pProcTable[remote_iproc].host);
		sprintf(pszKey, "ListenPort%d", remote_iproc);
		BNR_Get(g_myBNRgroup, pszKey, pszValue);
		g_pProcTable[remote_iproc].listen_port = atoi(pszValue);
	}
	else if (g_bUseDatabase)
	{
		char pszKey[100], pszValue[100];
		int length = NT_HOSTNAME_LEN;
		sprintf(pszKey, "ListenHost%d", remote_iproc);
		g_Database.Get(pszKey, g_pProcTable[remote_iproc].host, &length);
		sprintf(pszKey, "ListenPort%d", remote_iproc);
		length = 100;
		g_Database.Get(pszKey, pszValue, &length);
		g_pProcTable[remote_iproc].listen_port = atoi(pszValue);
	}
	else
		GetProcessConnectInfo(remote_iproc);

	hostEnt = gethostbyname(g_pProcTable[remote_iproc].host);
	if (hostEnt != NULL)
		nic_addr = *((unsigned long*)hostEnt->h_addr_list[0]);
	// Create a socket and connect to process 'remote_iproc'
	// create the event
	temp_event = WSACreateEvent();
	if (temp_event == WSA_INVALID_EVENT)
		nt_error("WSACreateEvent failed in ConnectTo", WSAGetLastError());
	// create the socket
	temp_socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (temp_socket == INVALID_SOCKET)
		nt_error("socket failed in ConnectTo", WSAGetLastError());

	DPRINTF(("connecting to %s on %d\n", g_pProcTable[remote_iproc].host, g_pProcTable[remote_iproc].listen_port));
	if (ret_val = NT_Tcp_connect(temp_socket, g_pProcTable[remote_iproc].host, g_pProcTable[remote_iproc].listen_port))
		MakeErrMsg(ret_val, "NT_Tcp_connect failed in ConnectTo(%s:%d)", g_pProcTable[remote_iproc].host, g_pProcTable[remote_iproc].listen_port);
	
	if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
	{
		error = WSAGetLastError();
		if (error == WSAENOBUFS)
		{
		    Sleep(250);
		    if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
		    {
			error = WSAGetLastError();
			if (error == WSAENOBUFS)
			{
			    Sleep(250);
			    if (setsockopt(temp_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(BOOL)) == SOCKET_ERROR)
			    {
				error = WSAGetLastError();
				if (error != WSAENOBUFS)
				    nt_error("setsockopt failed in ConnectTo", error);
			    }
			}
			else
				nt_error("setsockopt failed in ConnectTo", error);
		    }
		}
		else
			nt_error("setsockopt failed in ConnectTo", error);
	}

	if (WSAEventSelect(temp_socket, temp_event, FD_READ | FD_CLOSE) == SOCKET_ERROR)
		nt_error("WSAEventSelect failed in ConnectTo", WSAGetLastError());
	
	// Send my rank so the remote side knows who is connecting
	if (SendBlocking(temp_socket, (char*)&g_nIproc, sizeof(int), 0) == SOCKET_ERROR)
		nt_error("send g_nIproc failed in ConnectTo", WSAGetLastError());

	// Receive an ack determining whether the connection was added to the list or not
	if (ret_val = ReceiveBlocking(temp_socket, temp_event, &ack, 1, 0))
		MakeErrMsg(ret_val, "ConnectTo failed to receive ack for socket %d", remote_iproc);

	if (ack == 1)
	{
		// Insert the socket in the proc table
		g_pProcTable[remote_iproc].sock = temp_socket;
		g_pProcTable[remote_iproc].sock_event = temp_event;

		// Associate the socket with the completion port
		if (CreateIoCompletionPort((HANDLE)temp_socket, g_hCommPort, remote_iproc, g_NumCommPortThreads) == NULL)
			nt_error("Unable to associate completion port with socket", GetLastError());

		// Post the first read from the socket
		g_pProcTable[remote_iproc].msg.ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (g_pProcTable[remote_iproc].msg.ovl.hEvent == NULL)
			MakeErrMsg(GetLastError(), "ConnectTo:CreateEvent failed for event[%d]", remote_iproc);
		g_pProcTable[remote_iproc].msg.state = NT_MSG_READING_TAG;
		g_pProcTable[remote_iproc].msg.nRemaining = sizeof(int);
		g_pProcTable[remote_iproc].msg.ovl.Offset = 0;
		g_pProcTable[remote_iproc].msg.ovl.OffsetHigh = 0;
		if (!ReadFile((HANDLE)temp_socket, &(g_pProcTable[remote_iproc].msg.tag), sizeof(int), &(g_pProcTable[remote_iproc].msg.nRead), &(g_pProcTable[remote_iproc].msg.ovl)))
		{
			int error = GetLastError();
			if (error != ERROR_IO_PENDING)
				MakeErrMsg(error, "ConnectTo:First posted read from socket %d failed", remote_iproc);
		}

		DPRINTF(("process %d: established connection to %d\n", g_nIproc, remote_iproc));
	}
	else
	{
		// The listener determined this side to be the loser in a race condition
		// So close the socket and wait for the socket created in another thread
		// to be inserted in the proc table.
		DPRINTF(("process %d: connection rejected for rank %d, waiting for connection to be established\n", g_nIproc, remote_iproc));
		NT_Tcp_closesocket(temp_socket, temp_event);
		while (g_pProcTable[remote_iproc].sock == INVALID_SOCKET)
			Sleep(100);
	}

	ReleaseMutex(g_hAddSocketMutex);
	return 1;
}
