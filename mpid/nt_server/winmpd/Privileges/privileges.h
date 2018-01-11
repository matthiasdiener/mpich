#ifndef PRIVILEGES_H
#define PRIVILEGES_H

#include <winsock2.h>
#include <windows.h>

DWORD SetAccountRights(LPTSTR User, LPTSTR Privilege);

#endif
