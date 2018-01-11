#ifndef WAIT_THREAD_H
#define WAIT_THREAD_H

#include <winsock2.h>
#include <windows.h>

void WaitForLotsOfObjects(int nHandles, HANDLE *pHandle);

#endif
