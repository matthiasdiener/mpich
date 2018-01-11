#ifndef CONNECT_TO_HOST_H
#define CONNECT_TO_HOST_H

#include <winsock2.h>
#include <windows.h>

bool ConnectToHost(const char *host, int port, char *pwd, SOCKET *psock, bool fast = false);

#endif
