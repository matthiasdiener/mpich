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

static void BarrierToString(BarrierStruct *p, char *pszStr, int length)
{
    struct bfdNode *pBfd;
    if (!snprintf_update(pszStr, length, "BARRIER:\n"))
	return;
    if (!snprintf_update(pszStr, length, " name: %s\n count: %d\n in: %d\n", p->pszName, p->nCount, p->nCurIn))
	return;
    pBfd = p->pBfdList;
    if (pBfd)
    {
	if (!snprintf_update(pszStr, length, " bfds: "))
	    return;
	while (pBfd)
	{
	    if (!snprintf_update(pszStr, length, "%d, ", pBfd->bfd))
		return;
	    pBfd = pBfd->pNext;
	}
	if (!snprintf_update(pszStr, length, "\n"))
	    return;
    }
}

void statBarrier(char *pszOutput, int length)
{
    BarrierStruct *p;

    *pszOutput = '\0';
    length--; // leave room for the null character

    if (g_pBarrierList == NULL)
	return;

    p = g_pBarrierList;
    while (p)
    {
	BarrierToString(p, pszOutput, length);
	length = length - strlen(pszOutput);
	pszOutput = &pszOutput[strlen(pszOutput)];
	p = p->pNext;
    }
}

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
	strncpy(p->pszName, pszName, 100);
	p->pszName[99] = '\0';
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
