#include "mpdimpl.h"
#include "GetOpt.h"
#include "Translate_Error.h"

void ConnectAndRestart(int *argc, char ***argv, char *host)
{
    int bfd;
    char str[CONSOLE_STR_LENGTH+1];
    char *result;
    int error;
    char phrase[MPD_PASSPHRASE_MAX_LENGTH+1];
    int port = -1;
    bool bAskPwd;

    PUSH_FUNC("ConnectAndRestart");

    bsocket_init();
    GetOpt(*argc, *argv, "-port", &port);
    bAskPwd = GetOpt(*argc, *argv, "-getphrase");
    GetOpt(*argc, *argv, "-phrase", phrase);

    ParseRegistry(false);
    if (host == NULL || host[0] == '\0')
	host = g_pszHost;
    if (port == -1)
	port = g_nPort;
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("beasy_create failed: %d\n%s\n", error, str);
	fflush(stdout);
	POP_FUNC();
	return;
    }
    if (bAskPwd || !ReadMPDRegistry("phrase", phrase, false))
    {
	printf("please input the passphrase: ");fflush(stdout);
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwMode;
	if (!GetConsoleMode(hStdin, &dwMode))
		dwMode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin, dwMode & ~ENABLE_ECHO_INPUT);
	gets(phrase);
	SetConsoleMode(hStdin, dwMode);
	printf("\n");fflush(stdout);
    }
    printf("connecting to %s:%d\n", host, port);fflush(stdout);
    if (beasy_connect(bfd, host, port) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("beasy_connect failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading challenge string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (strlen(phrase) + strlen(str) > MPD_PASSPHRASE_MAX_LENGTH)
    {
	printf("unable to process passphrase.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    strcat(phrase, str);
    result = crypt(phrase, MPD_SALT_VALUE);
    memset(phrase, 0, strlen(phrase)); // zero out the passphrase
    strcpy(str, result);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString of the encrypted response string failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("host authentication failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString('console') failed: %d\n%s\n", error, str);fflush(stdout);
	beasy_closesocket(bfd);
	POP_FUNC();
	return;
    }
    printf("connected\n");fflush(stdout);

    // send restart request
    if (WriteString(bfd, "restart") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	printf("writing '%s' failed, %d\n", str, error);
	Translate_Error(error, str);
	printf("%s\n", str);
	fflush(stdout);
	POP_FUNC();
	return;
    }
    //printf("waiting for result\n");fflush(stdout);
    if (ReadStringTimeout(bfd, str, 10))
    {
	printf("%s\n", str);fflush(stdout);
    }

    if (WriteString(bfd, "done") == SOCKET_ERROR)
    {
	error = WSAGetLastError();
	Translate_Error(error, str);
	printf("WriteString failed: %d\n%s\n", error, str);
	fflush(stdout);
    }
    beasy_closesocket(bfd);

    bsocket_finalize();
    POP_FUNC();
}
