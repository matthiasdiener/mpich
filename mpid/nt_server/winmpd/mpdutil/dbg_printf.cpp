#include "mpd.h"
#include "GetStringOpt.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>
#include <service.h>

//__declspec(thread) char g_call_stack[100][FUNC_STR_LENGTH+1];
//__declspec(thread) int g_call_index = 0;

void dbg_printf(char *str, ...)
{
    char pszStr[4096];
    char pszCheck[100];
    va_list list;
    int n, i;
    char *token;

    // Write to a temporary string
    va_start(list, str);
    vsprintf(pszStr, str, list);
    va_end(list);

    // modify the output
    if (GetStringOpt(pszStr, "p", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "pwd", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    token = strstr(pszStr, "PMI_PWD=");
    if (token != NULL)
    {
	strncpy(pszCheck, &token[8], 100);
	pszCheck[99] = '\0';
	token = strtok(pszCheck, " '|\n");
	n = strlen(pszCheck);
	token = strstr(pszStr, "PMI_PWD=");
	token = &token[8];
	if (n > 0)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "password", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    // print the modified string
    printf("%s", pszStr);
    fflush(stdout);
}

void log_error(char *lpszMsg)
{
    char    szMsg[256];
    HANDLE  hEventSource;
    char   *lpszStrings[2];
    
    // Use event logging to log the error.
    //
    hEventSource = RegisterEventSource(NULL, SZSERVICENAME);
    
    sprintf(szMsg, "%s error", SZSERVICENAME);
    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;
    
    if (hEventSource != NULL) {
	ReportEvent(hEventSource, // handle of event source
	    EVENTLOG_ERROR_TYPE,  // event type
	    0,                    // event category
	    0,                    // event ID
	    NULL,                 // current user's SID
	    2,                    // strings in lpszStrings
	    0,                    // no bytes of raw data
	    (LPCTSTR*)lpszStrings,// array of error strings
	    NULL);                // no raw data
	
	DeregisterEventSource(hEventSource);
    }
}

void err_printf(char *str, ...)
{
    char pszStr[4096];
    char pszCheck[100];
    va_list list;
    int n, i;
    char *token;

    // Write to a temporary string
    va_start(list, str);
    vsprintf(pszStr, str, list);
    va_end(list);

    // modify the output
    if (GetStringOpt(pszStr, "p", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "pwd", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    token = strstr(pszStr, "PMI_PWD=");
    if (token != NULL)
    {
	strncpy(pszCheck, &token[8], 100);
	pszCheck[99] = '\0';
	token = strtok(pszCheck, " '|\n");
	n = strlen(pszCheck);
	token = strstr(pszStr, "PMI_PWD=");
	token = &token[8];
	if (n > 0)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "password", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    // print error
    //fprintf(stderr, "----- ERROR --------\n");
    fprintf(stderr, "%s", pszStr);
    log_error(pszStr);
    /*
    for (int i=0; i<g_call_index; i++) 
    { 
	printf("%s\n", g_call_stack[i]); 
    }
    */
    //fprintf(stderr, "--------------------\n");
    fflush(stderr); 
}

void log_warning(char *lpszMsg)
{
    char    szMsg[256];
    HANDLE  hEventSource;
    char   *lpszStrings[2];
    
    // Use event logging to log the error.
    //
    hEventSource = RegisterEventSource(NULL, SZSERVICENAME);
    
    sprintf(szMsg, "%s error", SZSERVICENAME);
    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;
    
    if (hEventSource != NULL) {
	ReportEvent(hEventSource, // handle of event source
	    EVENTLOG_WARNING_TYPE,  // event type
	    0,                    // event category
	    0,                    // event ID
	    NULL,                 // current user's SID
	    2,                    // strings in lpszStrings
	    0,                    // no bytes of raw data
	    (LPCTSTR*)lpszStrings,// array of error strings
	    NULL);                // no raw data
	
	DeregisterEventSource(hEventSource);
    }
}

void warning_printf(char *str, ...)
{
    char pszStr[4096];
    char pszCheck[100];
    va_list list;
    int n, i;
    char *token;

    // Write to a temporary string
    va_start(list, str);
    vsprintf(pszStr, str, list);
    va_end(list);

    // modify the output
    if (GetStringOpt(pszStr, "p", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "pwd", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    token = strstr(pszStr, "PMI_PWD=");
    if (token != NULL)
    {
	strncpy(pszCheck, &token[8], 100);
	pszCheck[99] = '\0';
	token = strtok(pszCheck, " '|\n");
	n = strlen(pszCheck);
	token = strstr(pszStr, "PMI_PWD=");
	token = &token[8];
	if (n > 0)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    if (GetStringOpt(pszStr, "password", pszCheck))
    {
	token = strstr(pszStr, pszCheck);
	n = strlen(pszCheck);
	if (token != NULL)
	{
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    if (pszCheck[n-1] == '\r' || pszCheck[n-1] == '\n')
		n--;
	    for (i=0; i<n; i++)
		token[i] = '*';
	}
    }

    // print error
    //fprintf(stderr, "----- ERROR --------\n");
    fprintf(stderr, "%s", pszStr);
    log_warning(pszStr);
    /*
    for (int i=0; i<g_call_index; i++) 
    { 
	printf("%s\n", g_call_stack[i]); 
    }
    */
    //fprintf(stderr, "--------------------\n");
    fflush(stderr); 
}
