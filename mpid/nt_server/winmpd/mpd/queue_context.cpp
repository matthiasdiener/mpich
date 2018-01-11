#include "mpdimpl.h"

HANDLE g_hEnqueueMutex = CreateMutex(NULL, FALSE, NULL);

void EnqueueWrite(MPD_Context *p, char *pszStr, MPD_LowLevelState nState)
{
    // Note: You cannot have a concurrent reading and writing sessions in a
    // single context.  You can have multiple writes enqueued, but not multiple
    // reads.  There is a potential bug here if a command is being read when
    // EnqueueWrite is called.  This function will switch the state to WRITING
    // and the remaining data will not be read.  Then when the write is finished
    // the rest of the data will be read and it will be misunderstood.

    PUSH_FUNC("EnqueueWrite");

    if (p == NULL)
    {
	err_printf("attempting to enqueue '%s' into a NULL context\n", pszStr);
	POP_FUNC();
	return;
    }

    WaitForSingleObject(g_hEnqueueMutex, INFINITE);
    //dbg_printf("EnqueueWrite[%d]: '%s'\n", p->bfd, pszStr);
    if (p->nState == MPD_READING)
    {
	dbg_printf(":::DANGER WILL ROGERS::: switching from MPD_READING to MPD_WRITING on bfd[%d]\n", p->bfd);
    }
    if (p->nState != MPD_WRITING)
    {
	p->nCurPos = 0;
	p->nLLState = nState;
	strncpy(p->pszOut, pszStr, MAX_CMD_LENGTH);
	p->pszOut[MAX_CMD_LENGTH-1] = '\0';
	DoWriteSet(p->bfd);
	//dbg_printf("write enqueued directly into context\n");
    }
    else
    {
	if (p->pWriteList == NULL)
	{
	    p->pWriteList = new WriteNode(pszStr, nState);
	}
	else
	{
	    WriteNode *e = p->pWriteList;
	    while (e->pNext)
		e = e->pNext;
	    e->pNext = new WriteNode(pszStr, nState);
	}
	//dbg_printf("write enqueued into pWriteList\n");
    }
    p->nState = MPD_WRITING;
    ReleaseMutex(g_hEnqueueMutex);
    POP_FUNC();
}

void DequeueWrite(MPD_Context *p)
{
    PUSH_FUNC("DequeueWrite");
    //dbg_printf("DequeueWrite[%d]: %s\n", p->bfd, p->pszOut);
    if (p->pWriteList == NULL)
    {
	//dbg_printf("bfd[%d] state: MPD_IDLE, llstate: MPD_READING_CMD\n", p->bfd);
	p->nLLState = MPD_READING_CMD;
	p->nState = MPD_IDLE;
	BFD_CLR(p->bfd, &g_WriteSet);
	g_nActiveW--;
	//dbg_printf("bfd %d removed from write set\n", p->bfd);
	POP_FUNC();
	return;
    }

    WriteNode *e = p->pWriteList;
    p->pWriteList = p->pWriteList->pNext;

    p->nCurPos = 0;
    p->nState = MPD_WRITING;
    p->nLLState = e->nState;
    strncpy(p->pszOut, e->pString, MAX_CMD_LENGTH);
    p->pszOut[MAX_CMD_LENGTH-1] = '\0';
    //dbg_printf("bfd[%d] currently set to write '%s'\n", p->bfd, p->pszOut);

    delete e;
    POP_FUNC();
}
