#include "mpdutil.h"
#include "mpd.h"
#include "bsocket.h"
#include "Translate_Error.h"
#include "Service.h"

bool UpdateMPD(
	       const char *pszHost, 
	       const char *pszAccount, 
	       const char *pszPassword, 
	       int nPort, 
	       const char *pszPhrase, 
	       const char *pszFileName,
	       char *pszError,
	       int nErrLen)
{
    int bfd;
    char pszStr[MAX_CMD_LENGTH];
    char pszTempFileName[MAX_PATH];
    int ret_val;
    char *pszEncoded;

    // Connect to the mpd on pszHost
    ret_val = ConnectToMPD(pszHost, nPort, pszPhrase, &bfd);
    if (ret_val != 0)
    {
	_snprintf(pszError, nErrLen, "Unable to connect to %s\n", pszHost);
	return false;
    }

    // Initialize the file operations
    pszEncoded = EncodePassword((char*)pszPassword);
    _snprintf(pszStr, MAX_CMD_LENGTH, "fileinit account=%s password=%s", pszAccount, pszEncoded);
    if (pszEncoded != NULL) free(pszEncoded);

    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	printf("Writing the fileinit command failed, error %d\n", WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Create a temporary file
    _snprintf(pszStr, MAX_CMD_LENGTH, "createtmpfile host=%s delete=no", pszHost);
    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	_snprintf(pszError, nErrLen, "Writing the createtempfile command failed on %s, error %d\n", pszHost, WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }
    if (!ReadString(bfd, pszTempFileName))
    {
	_snprintf(pszError, nErrLen, "Reading the temporary file name failed\n");
	beasy_closesocket(bfd);
	return false;
    }

    // Copy the new mpd executable into this temporary file
    _snprintf(pszStr, MAX_CMD_LENGTH, "local='%s' remote='%s'", pszFileName, pszTempFileName);
    if (!PutFile(bfd, pszStr))
    {
	_snprintf(pszStr, MAX_CMD_LENGTH, "deletetmpfile host=%s file='%s'", pszHost, pszTempFileName);
	WriteString(bfd, pszStr);
	ReadString(bfd, pszStr);
	WriteString(bfd, "done");
	_snprintf(pszError, nErrLen, "Unable to put the new mpd file on host %s", pszHost);
	beasy_closesocket(bfd);
	return false;
    }

    // Update the mpd
    _snprintf(pszStr, MAX_CMD_LENGTH, "update %s", pszTempFileName);
    ret_val = WriteString(bfd, pszStr);
    if (ret_val == SOCKET_ERROR)
    {
	_snprintf(pszError, nErrLen, "Writing the update command failed, error %d", WSAGetLastError());
	beasy_closesocket(bfd);
	return false;
    }

    // Close the console session
    WriteString(bfd, "done");
    beasy_closesocket(bfd);

    return true;
}
