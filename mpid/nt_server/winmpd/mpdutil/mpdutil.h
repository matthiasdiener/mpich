#ifndef MPDUTIL_H
#define MPDUTIL_H

bool ReadStringMax(int bfd, char *str, int max);
bool ReadString(int bfd, char *str);
bool ReadStringTimeout(int bfd, char *str, int timeout);
int WriteString(int bfd, char *str);

int ConnectToMPD(const char *host, int port, const char *phrase, int *pbfd);
int ConnectToMPDquick(const char *host, int port, const char *inphrase, int *pbfd);
void MakeLoop(int *pbfdRead, int *pbfdWrite);

char * EncodePassword(char *pwd);
void DecodePassword(char *pwd);

#define TRANSFER_BUFFER_SIZE 20*1024
void GetFile(int bfd, char *pszInputStr);
bool PutFile(int bfd, char *pszInputStr);

bool TryCreateDir(char *pszFileName, char *pszError);

bool UpdateMPD(const char *pszHost, const char *pszAccount, const char *pszPassword, int nPort, const char *pszPhrase, const char *pszFileName, char *pszError, int nErrLen);
bool UpdateMPICH(const char *pszHost, const char *pszAccount, const char *pszPassword, int nPort, const char *pszPhrase, const char *pszFileName, const char *pszFileNamed, char *pszError, int nErrLen);

void dbg_printf(char *str, ...);
void warning_printf(char *str, ...);
void err_printf(char *str, ...);

unsigned int mpd_version_string_to_int(char *version_str);
void mpd_version_int_to_string(unsigned int n, char *str);

#if defined(__cplusplus)
#include "qvs.h"
#endif

#endif
