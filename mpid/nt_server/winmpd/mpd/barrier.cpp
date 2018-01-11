#include "mpdimpl.h"

struct bfdNode
{
    int bfd;
    bfdNode *pNext;
};

struct BarrierStruct
{
    char pszName[100];
    int nCount, nCurIn;
    bfdNode *pBfdList;
    BarrierStruct *pNext;
};

BarrierStruct *g_pBarrierList;

static void DeleteBfdList(bfdNode *p)
{
    if (p == NULL)
	return;
    DeleteBfdList(p->pNext);
    delete p;
}

static void RemoveBarrierStruct(char *pszName)
{
    BarrierStruct *p, *pTrailer;
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdBarrierStructMutex");
    WaitForSingleObject(hMutex, INFINITE);

    pTrailer = p = g_pBarrierList;

    while (p)
    {
	if (strcmp(p->pszName, pszName) == 0)
	{
	    if (p == g_pBarrierList)
		g_pBarrierList = NULL;
	    else
		pTrailer->pNext = p->pNext;
	    DeleteBfdList(p->pBfdList);
	    dbg_printf("barrier structure '%s' removed\n", pszName);
	    delete p;
	    ReleaseMutex(hMutex);
	    CloseHandle(hMutex);
	    return;
	}
	if (pTrailer != p)
	    pTrailer = pTrailer->pNext;
	p = p->pNext;
    }

    err_printf("Error: RemoveBarrierStruct: barrier structure '%s' not found\n", pszName);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

void SetBarrier(char *pszName, int nCount, int bfd)
{
    BarrierStruct *p;
    HANDLE hMutex = CreateMutex(NULL, FALSE, "mpdBarrierStructMutex");
    WaitForSingleObject(hMutex, INFINITE);

    p = g_pBarrierList;

    while (p)
    {
	if (strcmp(p->pszName, pszName) == 0)
	{
	    p->nCurIn++;
	    if (p->nCount != nCount)
		err_printf("Error: count's don't match, %d != %d", p->nCount, nCount);
	    if (bfd != BFD_INVALID_SOCKET)
	    {
		bfdNode *pNode = new bfdNode;
		pNode->bfd = bfd;
		pNode->pNext = p->pBfdList;
		p->pBfdList = pNode;
	    }
	    dbg_printf("SetBarrier: name=%s count=%d curcount=%d\n", p->pszName, p->nCount, p->nCurIn);
	    break;
	}
	p = p->pNext;
    }
    if (p == NULL)
    {
	p = new BarrierStruct;
	strcpy(p->pszName, pszName);
	p->nCount = nCount;
	p->nCurIn = 1;
	if (bfd != BFD_INVALID_SOCKET)
	{
	    p->pBfdList = new bfdNode;
	    p->pBfdList->bfd = bfd;
	    p->pBfdList->pNext = NULL;
	}
	else
	    p->pBfdList = NULL;
	p->pNext = g_pBarrierList;
	g_pBarrierList = p;
	dbg_printf("SetBarrier: name=%s count=%d curcount=%d\n", p->pszName, p->nCount, p->nCurIn);
    }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    if (p->nCurIn >= p->nCount)
    {
	dbg_printf("SetBarrier: count reached for name=%s, %d:%d\n", p->pszName, p->nCount, p->nCurIn);
	bfdNode *pNode = p->pBfdList;
	while (pNode)
	{
	    dbg_printf("SetBarrier: writing success for name=%s\n", p->pszName);
	    WriteString(pNode->bfd, "SUCCESS");
	    pNode = pNode->pNext;
	}
	RemoveBarrierStruct(p->pszName);
    }
}
