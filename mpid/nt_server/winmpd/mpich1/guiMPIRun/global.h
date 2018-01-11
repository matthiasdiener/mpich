#ifndef GLOBAL_H
#define GLOBAL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>

#define FORWARD_NPROC_THRESHOLD 3

struct HostNode
{
    TCHAR host[100];
    TCHAR exe[MAX_PATH];
    long nSMPProcs;
    HostNode *next;
};

struct ForwardHostStruct
{
    char pszHost[40];
    int nPort;
};

struct MapDriveNode
{
    char cDrive;
    char pszShare[MAX_PATH];
    //char pszAccount[40];
    //char pszPassword[40];
    MapDriveNode *pNext;
};

bool UnmapDrives(int bfd, MapDriveNode *pList);

// Global variables

#define NUM_GLOBAL_COLORS 8
extern COLORREF aGlobalColor[NUM_GLOBAL_COLORS];

#endif
