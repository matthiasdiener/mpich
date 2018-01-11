/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef REDIRECT_IO_H
#define REDIRECT_IO_H

#include <winsock2.h>
#include <windows.h>

struct RedirectIOArg
{
    int *m_pbfdStopIOSignalSocket;
    HANDLE hReadyEvent;
};

void RedirectIOThread(RedirectIOArg *pArg);

#endif
