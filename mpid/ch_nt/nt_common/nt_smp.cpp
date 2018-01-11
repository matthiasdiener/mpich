#include "nt_global_cpp.h"
#include "ShmemLockedQueue.h"
#include "parsecliques.h"
#include <stdlib.h>

// Shared memory stuff
int g_ShmemQSize = 1024*1024;
//ShmemQueue **g_pShmemQueue = NULL;
ShmemLockedQueue **g_pShmemQueue = NULL;
int g_nMaxShmSendSize = 1024*15;
HANDLE g_hShmRecvThread = NULL;
int g_nSMPLow = 0;
int g_nSMPHigh = 0;

// Shared process stuff
HANDLE *g_hShpMutex = NULL, *g_hShpSendCompleteEvent = NULL;
HANDLE *g_hProcesses = NULL;

// Function name	: ShmRecvThread
// Description	    : 
// Return type		: void 
// Argument         : ShmemQueue *pShmemQueue
void ShmRecvThread(ShmemLockedQueue *pShmemLockedQueue)
{
	while (true)
	{
		if (!pShmemLockedQueue->RemoveNextInsert(&g_MsgQueue))
			ExitThread(0);
	}	
}

// Function name	: PollShmemQueue
// Description	    : 
// Return type		: void 
void PollShmemQueue()
{
	//*
	if (!g_pShmemQueue[g_nIproc]->RemoveNextInsert(&g_MsgQueue, false))
		Sleep(0);
	/*/
	for (int i=0; i<10; i++)
	{
		if (g_pShmemQueue[g_nIproc]->RemoveNextInsert(&g_MsgQueue, false))
			return;
	}
	Sleep(0);
	//*/
}

// Function name	: GetShmemClique
// Description	    : Determine which processes this process can reach through shared memory
// Return type		: void 
void GetShmemClique()
{
	char pszTemp[100];

	if (GetEnvironmentVariable("MPICH_SHM_CLICKS", pszTemp, 100) || GetEnvironmentVariable("MPICH_SHM_CLIQUES", pszTemp, 100))
	{
		int nCount=0, *pMembers = NULL;
		if (ParseCliques(pszTemp, g_nIproc, g_nNproc, &nCount, &pMembers))
		{
			nt_error("Unable to parse the SHM cliques", 1);
			return;
		}

		// For now, the block of shared memory processes must be contiguous
		if (nCount > 0)
		{
			g_nSMPLow = pMembers[0];
			g_nSMPHigh = pMembers[nCount-1];
		}
		else
			g_nSMPLow = g_nSMPHigh = g_nIproc;
		/*
		// In the future the mapping will be arbitrary
		for (int i=0; i<nCount; i++)
		{
			if ( (pMembers[i] >= 0) && (pMembers[i] < g_nNproc) )
				g_pProcTable[pMembers[i]].shm = 1;
		}
		//*/
		if (pMembers != NULL)
			delete pMembers;
	}
	else
	{
		char pszSMPLow[10]="", pszSMPHigh[10]="";

		if (GetEnvironmentVariable("MPICH_SHM_LOW", pszSMPLow, 10))
			g_nSMPLow = atoi(pszSMPLow);
		else
			g_nSMPLow = g_nIproc;
		if (GetEnvironmentVariable("MPICH_SHM_HIGH", pszSMPHigh, 10))
			g_nSMPHigh = atoi(pszSMPHigh);
		else
			g_nSMPHigh = g_nIproc;
	}
}

// Function name	: InitSMP
// Description	    : 
// Return type		: void 
void InitSMP()
{
	int i;
	char nameBuffer[256], pszTemp[100], pszSMPLow[10]="", pszSMPHigh[10]="";

	GetShmemClique();

	if (g_nSMPLow == g_nSMPHigh)
		return;

	// Initialize shared memory stuff
	if (GetEnvironmentVariable("MPICH_MAXSHMMSG", pszTemp, 100))
	{
		g_nMaxShmSendSize = atoi(pszTemp);
		if (g_nMaxShmSendSize < 0)
			g_nMaxShmSendSize = 0;
	}
	if (GetEnvironmentVariable("MPICH_SHMQSIZE", pszTemp, 100))
	{
		g_ShmemQSize = atoi(pszTemp);
		if (g_ShmemQSize < g_nMaxShmSendSize)
			g_ShmemQSize = g_nMaxShmSendSize;
	}

	g_pShmemQueue = new ShmemLockedQueue*[g_nNproc];

	for (i=0; i<g_nNproc; i++)
		g_pShmemQueue[i] = NULL;

	for (i=g_nSMPLow; i<=g_nSMPHigh; i++)
	{
		g_pShmemQueue[i] = new ShmemLockedQueue;
		sprintf(nameBuffer, "%s.shm%d", g_pszJobID, i);
		if (!g_pShmemQueue[i]->Init(nameBuffer, g_ShmemQSize))
			nt_error("unable to initialize ShmemQueue", i);
		g_pProcTable[i].shm = 1;
	}

	// Initialize shared process stuff
	int nSMP = g_nSMPHigh - g_nSMPLow + 1;
	g_hShpMutex = new HANDLE[nSMP];
	g_hShpSendCompleteEvent = new HANDLE[nSMP];
	g_hProcesses = new HANDLE[nSMP];

	// Create all the named events and mutexes
	for (i=0; i<nSMP; i++)
	{
		char pBuffer[100];
		sprintf(pBuffer, "%s.shp%dMutex", g_pszJobID, i);
		g_hShpMutex[i] = CreateMutex(NULL, FALSE, pBuffer);
		if (g_hShpMutex[i] == NULL)
			MakeErrMsg(GetLastError(), "InitSMP: CreateMutex failed for g_hShmMutex[%d]", i);
		sprintf(pBuffer, "%s.shp%dSendComplete", g_pszJobID, i);
		g_hShpSendCompleteEvent[i] = CreateEvent(NULL, TRUE, FALSE, pBuffer);
		if (g_hShpSendCompleteEvent[i] == NULL)
			MakeErrMsg(GetLastError(), "InitSMP: CreateEvent failed for g_hShpSendCompleteEvent[%d]", i);
	}

	unsigned long pid = GetCurrentProcessId();
	// Send my information to the other processes
	for (i=g_nSMPLow; i<=g_nSMPHigh; i++)
	{
		if (i != g_nIproc)
		{
			if (!g_pShmemQueue[i]->Insert((unsigned char *)&pid, sizeof(unsigned long), 0, g_nIproc))
				nt_error("InitSMP: Unable to send pid info to remote process", i);
		}
	}
	// Get the information about the other processes
	for (i=1; i<nSMP; i++)
	{
		int tag, from;
		unsigned int length = sizeof(unsigned long);
		if (!g_pShmemQueue[g_nIproc]->RemoveNext((unsigned char *)&pid, &length, &tag, &from))
			nt_error("InitSMP: Unable to receive pid information from remote processes", 0);
		g_hProcesses[from - g_nSMPLow] = OpenProcess(STANDARD_RIGHTS_REQUIRED | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
	}

	pszTemp[0] = '\0';
	GetEnvironmentVariable("MPICH_SHM_SINGLETHREAD", pszTemp, 100);

	if (pszTemp[0] == '1')
	{
		// Set the poll function so the shmem device will run single threaded.
		g_MsgQueue.SetProgressFunction(PollShmemQueue);
		//g_pShmemQueue[g_nIproc]->SetProgressFunction(PollShmemQueue);
	}
	else
	{
		// Start the shared memory receive thread
		DWORD dwThreadID;
		g_hShmRecvThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShmRecvThread, g_pShmemQueue[g_nIproc], NT_THREAD_STACK_SIZE, &dwThreadID);
		if (g_hShmRecvThread == NULL)
			nt_error("InitSMP: Unable to create ShmRecvThread", 0);
	}
}

// Function name	: EndSMP
// Description	    : 
// Return type		: void 
void EndSMP()
{
	int i;

	if (g_nSMPLow == g_nSMPHigh)
		return;

	if (g_hShmRecvThread != NULL)
	{
		// Signal the shm thread to exit
		g_pShmemQueue[g_nIproc]->Insert(NULL, 0, 0, -1);
		WaitForSingleObject(g_hShmRecvThread, 5000);
		CloseHandle(g_hShmRecvThread);
		g_hShmRecvThread = NULL;
	}

	// Delete the shared memory queues
	for (i=g_nSMPLow; i<=g_nSMPHigh; i++)
	{
		delete g_pShmemQueue[i];
	}
	delete g_pShmemQueue;

	// Delete all the shared process stuff
	int nSMP = g_nSMPHigh - g_nSMPLow + 1;

	// Delete all the named events and mutexes
	for (i=0; i<nSMP; i++)
	{
		CloseHandle(g_hShpMutex[i]);
		CloseHandle(g_hShpSendCompleteEvent[i]);
		CloseHandle(g_hProcesses[i]);
	}
	delete g_hShpMutex;
	delete g_hShpSendCompleteEvent;
	delete g_hProcesses;
	g_hShpMutex = NULL;
	g_hShpSendCompleteEvent = NULL;
	g_hProcesses = NULL;
}

// Function name	: NT_ShmSend
// Description	    : 
// Return type		: void 
// Argument         : int type
// Argument         : void *buffer
// Argument         : int length
// Argument         : int to
void NT_ShmSend(int type, void *buffer, int length, int to)
{
	if (length > g_nMaxShmSendSize)
	{
		// Shared process send
		if (!g_pShmemQueue[to]->InsertSHP((unsigned char *)buffer, length, type, g_nIproc, g_hShpMutex[to-g_nSMPLow], g_hShpSendCompleteEvent[to-g_nSMPLow], g_pShmemQueue[g_nIproc]))
		{
			nt_error("shared process send failed", to);
		}
	}
	else
	{
		// Shared memory send
		if (!g_pShmemQueue[to]->Insert((unsigned char *)buffer, length, type, g_nIproc))
		{
			nt_error("shared memory send failed", to);
		}
	}
}
