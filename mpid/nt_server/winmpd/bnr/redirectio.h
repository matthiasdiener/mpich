#ifndef REDIRECT_IO_H
#define REDIRECT_IO_H

#include <winsock2.h>
#include <windows.h>

struct RedirectIOArg
{
    int *m_pbfdStopIOSignalSocket;
    HANDLE hReadyEvent;
};

void RedirectIOThread(RedirectIOArg *pArg);

#endif
