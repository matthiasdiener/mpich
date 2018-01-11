#ifndef REDIRECT_IO_H
#define REDIRECT_IO_H

/*
#include "resource.h"
#include "guiMPIRunView.h"
#include <winsock2.h>
#include <windows.h>
*/

struct RedirectIOArg
{
    CGuiMPIRunView *pDlg;
    HANDLE hReadyEvent;
};

void RedirectIOThread(RedirectIOArg *pArg);

#endif
