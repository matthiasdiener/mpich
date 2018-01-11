#ifndef CONNECT_TO_HOST_H
#define CONNECT_TO_HOST_H

bool ConnectToHost(const char *host, int port, char *pwd, int *pbfd, bool fast = false);

#endif
