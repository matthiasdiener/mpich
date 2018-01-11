#ifndef MPDUTIL_H
#define MPDUTIL_H

bool ReadStringMax(int bfd, char *str, int max);
bool ReadString(int bfd, char *str);
bool ReadStringTimeout(int bfd, char *str, int timeout);
int WriteString(int bfd, char *str);
int ConnectToMPD(char *host, int port, char *phrase, int *pbfd);
void MakeLoop(int *pbfdRead, int *pbfdWrite);

void err_printf(char *str, ...);

#endif
