#include "mpdimpl.h"

void GetDirectoryContents(int bfd, char *pszInputStr)
{
    int nFolders, nFiles;
    char pszStr[MAX_PATH], pszLength[50];
    int i;

    if (WriteString(bfd, pszInputStr) == SOCKET_ERROR)
    {
	printf("writing '%s' command failed\n", pszInputStr);
	return;
    }

    if (!ReadString(bfd, pszStr))
    {
	printf("Error: reading nFolders failed\n");
	return;
    }
    if (strnicmp(pszStr, "ERROR", 5) == 0)
    {
	printf("%s\n", pszStr);
	return;
    }
    nFolders = atoi(pszStr);

    //printf("Folders:\n");
    for (i=0; i<nFolders; i++)
    {
	if (!ReadString(bfd, pszStr))
	{
	    printf("Error: reading folder name failed\n");
	    return;
	}
	printf("            %s\n", pszStr);
    }

    if (!ReadString(bfd, pszStr))
    {
	printf("Error: reading nFiles failed\n");
	return;
    }
    nFiles = atoi(pszStr);

    //printf("Files:\n");
    for (i=0; i<nFiles; i++)
    {
	if (!ReadString(bfd, pszStr))
	{
	    printf("Error: reading file name failed\n");
	    return;
	}
	if (!ReadString(bfd, pszLength))
	{
	    printf("Error: reading file length failed\n");
	    return;
	}
	//printf("%s %s\n", pszStr, pszLength);
	printf("%11s %s\n", pszLength, pszStr);
    }
}
