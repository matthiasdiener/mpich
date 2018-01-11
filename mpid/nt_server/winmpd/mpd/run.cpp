#include "mpdimpl.h"

bool GenAuthenticationStrings(char *append, char *crypted)
{
    int stamp;
    char *crypted_internal;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
    char phrase_internal[MPD_PASSPHRASE_MAX_LENGTH+1];

    PUSH_FUNC("GenAuthenticationStrings");

    srand(GetTickCount());
    stamp = rand();

    if (!ReadMPDRegistry("phrase", phrase))
    {
	POP_FUNC();
	return false;
    }

    _snprintf(phrase_internal, MPD_PASSPHRASE_MAX_LENGTH, "%s%d", phrase, stamp);
    sprintf(append, "%d", stamp);

    //dbg_printf("GenAuthenticationStrings: calling crypt on '%s'\n", phrase_internal);
    crypted_internal = crypt(phrase_internal, MPD_SALT_VALUE);
    if (strlen(crypted_internal) > MPD_PASSPHRASE_MAX_LENGTH)
    {
	POP_FUNC();
	return false;
    }
    strcpy(crypted, crypted_internal);

    memset(phrase, 0, MPD_PASSPHRASE_MAX_LENGTH);
    memset(phrase_internal, 0, MPD_PASSPHRASE_MAX_LENGTH);

    POP_FUNC();
    return true;
}

// Run ///////////////////////////////////////////////////////////////////////
//
//
int Run()
{
    sockaddr addr;
#ifdef USE_LINGER_SOCKOPT
    struct linger linger;
#endif
    int len;
    int bfd;
    BOOL b;
    int n;
    MPD_Context *p, *e, *pNext;
    bfd_set read_set, write_set;
    int ret_val;
    int nRunCount = 0;
    int error;
    
    PUSH_FUNC("Run");

    //dbg_printf("Run started\n");
    while(true)
    {
	read_set = g_ReadSet;
	write_set = g_WriteSet;

	//printSet("ReadSet:\n", &read_set);
	//printSet("WriteSet:\n", &write_set);
	nRunCount++;
	n = bselect(g_maxfds, &read_set, &write_set, NULL, NULL);
	if (n == SOCKET_ERROR)
	{
	    error = WSAGetLastError();
	    err_printf("Run: bselect failed, error %d\n", error);
	    POP_FUNC();
	    return RUN_RESTART;
	}
	if (n == 0)
	{
	    err_printf("Run: bselect returned zero sockets available\n");
	    POP_FUNC();
	    return RUN_RESTART;
	}

	p = g_pList;
	while (n)
	{
	    if (p)
	    {
		if (p->nState != MPD_INVALID)
		{
		    if (BFD_ISSET(p->bfd, &read_set))
		    {
			//dbg_printf("Run[%d]: bfd[%d] readable\n", nRunCount, p->bfd);
			switch(p->nType)
			{
			case MPD_SOCKET:
			    //dbg_printf("read on generic mpd socket %d\n", p->bfd);
			case MPD_LEFT_SOCKET:
			case MPD_RIGHT_SOCKET:
			case MPD_CONSOLE_SOCKET:
			    p->nState = MPD_READING;
			    ret_val = bread(p->bfd, &p->pszIn[p->nCurPos], 1);
			    if (ret_val == 1)
			    {
				while (ret_val == 1)
				{
				    if (p->pszIn[p->nCurPos] == '\0')
				    {
					//dbg_printf("read string '%s'\n", p->pszIn);
					p->nState = MPD_IDLE;
					p->nCurPos = 0;
					StringRead(p);
					/*
					// If writes were to be enqueued after reads, then I would need to check for
					// enqueued writes here.
					if (p->pWriteList)
					{
					    strncpy(p->pszOut, p->pWriteList->pString, MAX_CMD_LENGTH);
					    p->pszOut[MAX_CMD_LENGTH-1] = '\0';
					    p->nLLState = p->pWriteList->nState;
					    p->nState = MPD_WRITING;
					    p->nCurPos = 0;
					    WriteNode *e = p->pWriteList;
					    p->pWriteList = p->pWriteList->pNext;
					    delete e;
					    DoWriteSet(p->bfd);
					}
					*/
					break;
				    }
				    else
				    {
					p->nCurPos++;
				    }
				    ret_val = bread(p->bfd, &p->pszIn[p->nCurPos], 1);
				}
			    }
			    else
			    {
				error = WSAGetLastError();
				char *pszSocket;
				switch (p->nType)
				{
				case MPD_LEFT_SOCKET:
				    pszSocket = "MPD_LEFT_SOCKET";
				    break;
				case MPD_RIGHT_SOCKET:
				    pszSocket = "MPD_RIGHT_SOCKET";
				    break;
				case MPD_CONSOLE_SOCKET:
				    pszSocket = "MPD_CONSOLE_SOCKET";
				    break;
				default:
				    pszSocket = "MPD_SOCKET";
				    break;
				}
				if (ret_val == SOCKET_ERROR)
				{
				    err_printf("Run: bread failed for %s, error %d\n", pszSocket, error);
				}
				else if (ret_val == 0)
				{
				    err_printf("Run: %s %d unexpectedly closed\n", pszSocket, p->bfd);
				}
				else
				{
				    err_printf("Run: bread on %s returned unknown value, %d\n", pszSocket, ret_val);
				}
				p->bDeleteMe = true;
				p->nState = MPD_INVALID;
			    }
			    break;
			case MPD_LISTENER:
			    //dbg_printf("listener[%d] accepting new connection\n", p->bfd);
			    len = sizeof(addr);
			    bfd = baccept(p->bfd, &addr, &len);
			    if (bfd == BFD_INVALID_SOCKET)
			    {
				int error = WSAGetLastError();
				err_printf("Run: baccept failed: %d\n", error);
				break;
			    }
			    dbg_printf("listener[%d] accepted new connection bfd[%d]\n", p->bfd, bfd);
#ifdef USE_LINGER_SOCKOPT
			    linger.l_onoff = 1;
			    linger.l_linger = 60;
			    if (bsetsockopt(bfd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOCKET_ERROR)
			    {
				int error = WSAGetLastError();
				err_printf("Run: bsetsockopt failed: %d\n", error);
				beasy_closesocket(bfd);
				break;
			    }
#endif
			    b = TRUE;
			    bsetsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(BOOL));
			    if ((g_nActiveR >= FD_SETSIZE - 3) || (g_nActiveW >= FD_SETSIZE - 3))
			    {
				// No more sockets available
				err_printf("Run: No sockets available, incoming connect rejected.\n");
				beasy_closesocket(bfd);
			    }
			    else
			    {
				e = new MPD_Context;
				e->bfd = bfd;
				e->nCurPos = 0;
				e->nState = MPD_IDLE;
				e->nLLState = MPD_AUTHENTICATE_WRITING_APPEND;
				e->nType = MPD_SOCKET;
				e->pNext = g_pList;
				g_pList = e;
				g_maxfds = BFD_MAX(bfd, g_maxfds);
				DoReadSet(bfd);
				DoWriteSet(bfd);
				if (!GenAuthenticationStrings(e->pszOut, e->pszCrypt))
				{
				    err_printf("Run: failed to generate the authentication strings\n");
				    RemoveContext(e);
				}
			    }
			    break;
			case MPD_SIGNALLER:
			    dbg_printf("read available on signal socket %d ... ", p->bfd);
			    ret_val = beasy_receive(p->bfd, p->pszIn, 1);

			    if (ret_val == SOCKET_ERROR)
			    {
				// If we lose the signaller, we cannot stop the service.
				// So, shut down here and abort.
				err_printf("Run: signaller socket failed, error %d\n", WSAGetLastError());
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_RESTART;
			    }

			    if (g_nSignalCount)
			    {
				dbg_printf("extracting\n");
				if (!Extract(false))
				{
				    // If we cannot start the extraction process, just exit
				    ShutdownAllProcesses();
				    AbortAllForwarders();
				    RemoveAllTmpFiles();
				    FinalizeDriveMaps();
				    warning_printf("mpd exiting without extracting itself from the ring\n");
				    POP_FUNC();
				    return RUN_EXIT;
				}
			    }
			    else
			    {
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_EXIT;
			    }
			    break;
			case MPD_RESELECTOR:
			    //dbg_printf("read available on re-selector socket %d\n", p->bfd);
			    if (beasy_receive(p->bfd, p->pszIn, 1) == SOCKET_ERROR)
			    {
				err_printf("Run: reselector socket failed, error %d\n", WSAGetLastError());
				ShutdownAllProcesses();
				AbortAllForwarders();
				RemoveAllTmpFiles();
				FinalizeDriveMaps();
				dbg_printf("exiting\n");
				POP_FUNC();
				return RUN_RESTART;
			    }
			    break;
			default:
			    err_printf("Run: Error, read available on socket %d of unknown type %d\n", p->bfd, p->nType);
			    break;
			}
			n--;
		    }
		    if (p->nState != MPD_INVALID && BFD_ISSET(p->bfd, &write_set))
		    {
			//dbg_printf("Run[%d]: bfd[%d] writeable\n", nRunCount, p->bfd);
			/*
			MPD_Context *pTemp = p->pNext;
			while (pTemp)
			{
			    if (pTemp->nState != MPD_INVALID && BFD_ISSET(pTemp->bfd, &write_set))
			    {
				dbg_printf("Run[%d]: bfd[%d] pending write '%s'\n", nRunCount, pTemp->bfd, pTemp->pszOut);
			    }
			    pTemp = pTemp->pNext;
			}
			*/
			switch(p->nType)
			{
			case MPD_SOCKET:
			    //dbg_printf("write on generic mpd socket %d\n", p->bfd);
			case MPD_LEFT_SOCKET:
			case MPD_RIGHT_SOCKET:
			case MPD_CONSOLE_SOCKET:
			    p->nState = MPD_WRITING;
			    ret_val = bwrite(p->bfd, &p->pszOut[p->nCurPos], strlen(&p->pszOut[p->nCurPos])+1);
			    if (ret_val > 0)
			    {
				p->nCurPos += ret_val;
				if (p->pszOut[p->nCurPos - 1] == '\0')
				{
				    //dbg_printf("wrote string '%s'\n", p->pszOut);
				    if (p->pWriteList == NULL)
					p->nState = MPD_IDLE;
				    p->nCurPos = 0;
				    StringWritten(p);
				}
			    }
			    else
			    {
				if (ret_val == SOCKET_ERROR)
				{
				    err_printf("Run: bwrite failed in MPD_SOCKET case: %d\n", WSAGetLastError());
				}
				else if (ret_val == 0)
				{
				    err_printf("Run: bwrite returned 0 bytes after being set for writing\n");
				}
			    }
			    break;
			case MPD_LISTENER:
			    err_printf("Run: Error, write available on listener socket %d\n", p->bfd);
			    break;
			case MPD_SIGNALLER:
			    err_printf("Run: Error, write available on signal socket %d\n", p->bfd);
			    break;
			default:
			    err_printf("Run: Error, write available on socket %d of unknown type %d\n", p->bfd, p->nType);
			    break;
			}
			n--;
		    }
		    else
		    {
			if (p->nState == MPD_INVALID && BFD_ISSET(p->bfd, &write_set))
			{
			    err_printf("Run: write available on invalid bfd[%d]\n", p->bfd);
			    n--;
			}
		    }
		}
		else
		{
		    if (BFD_ISSET(p->bfd, &read_set))
		    {
			err_printf("Run: read available on invalid bfd[%d]\n", p->bfd);
			n--;
		    }
		    if (BFD_ISSET(p->bfd, &write_set))
		    {
			err_printf("Run: write available on invalid bfd[%d]\n", p->bfd);
			n--;
		    }
		}
		pNext = p->pNext;
		if (p->bDeleteMe)
		    RemoveContext(p);
		p = pNext; // p may be deleted so we use pNext to access p->pNext
	    }
	    else
	    {
		err_printf("Run: n(%d) arbitrarily set to zero because p == NULL\n", n);
		n = 0;
		//Sleep(250);
	    }
	}
    }
    POP_FUNC();
    return RUN_EXIT;
}
