// MPIConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MPIConfig.h"
#include "MPIConfigDlg.h"
#include "mpd.h"
#include "bsocket.h"
#include "crypt.h"
#include <tchar.h>
#include "RegistrySettingsDialog.h"
#include "PwdDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMPIConfigDlg dialog

CMPIConfigDlg::CMPIConfigDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMPIConfigDlg::IDD, pParent)
{
    m_bNeedPassword = false;
    //{{AFX_DATA_INIT(CMPIConfigDlg)
    m_hostname = _T("");
	m_domain = _T("");
	m_static = _T("domain\r\nblank = default");
	//}}AFX_DATA_INIT
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_hFindThread = NULL;
    m_nMinWidth = -1;
    m_nMinHeight = -1;
}

void CMPIConfigDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMPIConfigDlg)
	DDX_Control(pDX, IDC_DOMAIN, m_domain_edit);
	DDX_Control(pDX, IDC_STATIC_EDIT, m_static_edit);
	DDX_Control(pDX, IDOK, m_ok_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDC_EDIT_ADD_BTN, m_edit_add_btn);
    DDX_Control(pDX, IDC_HOST_LIST, m_host_list);
    DDX_Control(pDX, IDC_SET_BTN, m_set_btn);
    DDX_Control(pDX, IDC_REFRESH_BTN, m_refresh_btn);
    DDX_Control(pDX, IDC_FIND_BTN, m_find_btn);
    DDX_Control(pDX, IDC_LIST, m_list);
    DDX_Text(pDX, IDC_HOSTNAME, m_hostname);
	DDX_Text(pDX, IDC_DOMAIN, m_domain);
	DDX_Text(pDX, IDC_STATIC_EDIT, m_static);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMPIConfigDlg, CDialog)
//{{AFX_MSG_MAP(CMPIConfigDlg)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_FIND_BTN, OnFindBtn)
ON_BN_CLICKED(IDC_REFRESH_BTN, OnRefreshBtn)
ON_BN_CLICKED(IDC_SET_BTN, OnSetBtn)
ON_BN_CLICKED(IDC_EDIT_ADD_BTN, OnEditAddBtn)
	ON_WM_VKEYTOITEM()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPIConfigDlg message handlers

void CMPIConfigDlg::ParseRegistry()
{
    HKEY tkey;
    DWORD result, len;
    
    // Set the defaults.
    m_nPort = MPD_DEFAULT_PORT;
    gethostname(m_pszHost, 100);
    
    m_bNeedPassword = true;

    // Open the root key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MPD_REGISTRY_KEY,
	0, KEY_ALL_ACCESS, &tkey) != ERROR_SUCCESS)
    {
	printf("Unable to open SOFTWARE\\MPICH\\MPD registry key, error %d\n", GetLastError());
	return;
    }
    
    // Read the port
    len = sizeof(int);
    result = RegQueryValueEx(tkey, "port", 0, NULL, (unsigned char *)&m_nPort, &len);
    
    // Check to see if a passphrase has been set and set it to the default if necessary.
    len = 100;
    result = RegQueryValueEx(tkey, "phrase", 0, NULL, (unsigned char *)m_pszPhrase, &len);
    if (result == ERROR_SUCCESS)
	m_bNeedPassword = false;
    
    RegCloseKey(tkey);
}

BOOL CMPIConfigDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon
    
    bsocket_init();
    ParseRegistry();

    RECT r;
    GetClientRect(&r);
    m_nMinWidth = r.right;
    m_nMinHeight = r.bottom;

    rList.SetInitialPosition(m_host_list.m_hWnd, RSR_STRETCH_BOTTOM);
    rOk.SetInitialPosition(m_ok_btn.m_hWnd, RSR_ANCHOR_RIGHT);
    rCancel.SetInitialPosition(m_cancel_btn.m_hWnd, RSR_ANCHOR_RIGHT);
    rBox.SetInitialPosition(m_list.m_hWnd, RSR_STRETCH);
    rFind.SetInitialPosition(m_find_btn.m_hWnd, RSR_MOVE);
    rRefresh.SetInitialPosition(m_refresh_btn.m_hWnd, RSR_MOVE);
    rDomain.SetInitialPosition(m_domain_edit.m_hWnd, RSR_ANCHOR_BOTTOM);
    rStatic.SetInitialPosition(m_static_edit.m_hWnd, RSR_ANCHOR_BOTTOM);

    char host[100] = "";
    gethostname(host, 100);
    m_hostname = host;
    UpdateData(FALSE);

    OnRefreshBtn();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMPIConfigDlg::OnPaint() 
{
    if (IsIconic())
    {
	CPaintDC dc(this); // device context for painting
	
	SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
	
	// Center icon in client rectangle
	int cxIcon = GetSystemMetrics(SM_CXICON);
	int cyIcon = GetSystemMetrics(SM_CYICON);
	CRect rect;
	GetClientRect(&rect);
	int x = (rect.Width() - cxIcon + 1) / 2;
	int y = (rect.Height() - cyIcon + 1) / 2;
	
	// Draw the icon
	dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
	CDialog::OnPaint();
    }
}

HCURSOR CMPIConfigDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

struct FindThreadSingleArg
{
    CListBox *list;
    int i;
    HWND hWnd;
    int port;
    char phrase[100];
};

HANDLE g_hMutex = CreateMutex(NULL, FALSE, NULL);

int WriteString(int bfd, char *str)
{
    return beasy_send(bfd, str, strlen(str)+1);
}

bool ReadString(int bfd, char *str)
{
    int n;
    do {
	n = 0;
	while (!n)
	{
	    n = bread(bfd, str, 1);
	    if (n == SOCKET_ERROR)
	    {
		printf("ReadString[%d] failed, error %d\n", bget_fd(bfd), WSAGetLastError());
		return false;
	    }
	}
    } while (*str++ != '\0');
    return true;
}

bool ConnectToHost(char *host, int port, char *pwd, int *pbfd)
{
    int bfd;
    char str[100];
    char phrase[100];
    char *result;
    
    strcpy(phrase, pwd);
    
    if (beasy_create(&bfd, 0, INADDR_ANY) == SOCKET_ERROR)
    {
	printf("beasy_create failed: %d\n", WSAGetLastError());fflush(stdout);
	return false;
    }
    //printf("connecting to %s:%d\n", host, arg->port);fflush(stdout);
    if (beasy_connect(bfd, host, port) == SOCKET_ERROR)
    {
	printf("beasy_connect failed: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading prepend string failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    strcat(phrase, str);
    WaitForSingleObject(g_hMutex, INFINITE);
    result = crypt(phrase, MPD_SALT_VALUE);
    strcpy(str, result);
    ReleaseMutex(g_hMutex);
    if (WriteString(bfd, str) == SOCKET_ERROR)
    {
	printf("WriteString of the crypt string failed: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (!ReadString(bfd, str))
    {
	printf("reading authentication result failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (strcmp(str, "SUCCESS"))
    {
	printf("authentication request failed.\n");fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    if (WriteString(bfd, "console") == SOCKET_ERROR)
    {
	printf("WriteString failed after attempting passphrase authentication: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	return false;
    }
    //printf("connected\n");fflush(stdout);
    *pbfd = bfd;
    return true;
}

void FindThreadSingle(FindThreadSingleArg *arg)
{
    TCHAR host[100];
    char str[100];
    int bfd;
    
    if (arg->list->GetText(arg->i, host) == LB_ERR)
    {
	::PostMessage(arg->hWnd, WM_USER+1, arg->i, FALSE);
	delete arg;
	return;
    }
    
    ::PostMessage(arg->hWnd, WM_USER+1, arg->i, TRUE);
    
    if (!ConnectToHost(host, arg->port, arg->phrase, &bfd))
    {
	::PostMessage(arg->hWnd, WM_USER+1, arg->i, FALSE);
	delete arg;
	return;
    }
    
    if (WriteString(bfd, "version") == SOCKET_ERROR)
    {
	printf("WriteString failed after attempting passphrase authentication: %d\n", WSAGetLastError());fflush(stdout);
	beasy_closesocket(bfd);
	::PostMessage(arg->hWnd, WM_USER+1, arg->i, FALSE);
	delete arg;
	return;
    }
    if (!ReadString(bfd, str))
    {
	::PostMessage(arg->hWnd, WM_USER+1, arg->i, FALSE);
	delete arg;
	return;
    }
    WriteString(bfd, "done");
    beasy_closesocket(bfd);
    
    ::PostMessage(arg->hWnd, WM_USER+1, -1, FALSE);
    delete arg;
}

void CMPIConfigDlg::OnFindBtn() 
{
    HCURSOR hOldCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
    
    m_find_btn.EnableWindow(FALSE);
    m_refresh_btn.EnableWindow(FALSE);
    m_set_btn.EnableWindow(FALSE);
    m_edit_add_btn.EnableWindow(FALSE);
    
    DWORD count = m_list.GetCount();
    m_num_threads = count;

    if (count < 1)
    {
	SetCursor(hOldCursor);
	return;
    }

    if (m_bNeedPassword)
    {
	CPwdDialog dlg;
	dlg.DoModal();
	if (dlg.m_bUseDefault)
	    strcpy(m_pszPhrase, MPD_DEFAULT_PASSPHRASE);
	else
	    strcpy(m_pszPhrase, dlg.m_password);
    }

    for (DWORD i=0; i<count; i++)
    {
	DWORD dwThreadID;
	FindThreadSingleArg *arg = new FindThreadSingleArg;
	arg->hWnd = m_hWnd;
	arg->list = &m_list;
	arg->i = i;
	arg->port = m_nPort;
	if (strlen(m_pszPhrase) < 100)
	    strcpy(arg->phrase, m_pszPhrase);
	else
	    arg->phrase[0] = '\0';
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindThreadSingle, arg, 0, &dwThreadID));
    }
    
    SetCursor(hOldCursor);
}

#include <lmerr.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmserver.h>

#ifndef LMCSTR
#define LMCSTR LPCWSTR
#endif

void CMPIConfigDlg::OnRefreshBtn() 
{
    DWORD num_read=0, total=0, size;
    int index;
    SERVER_INFO_100 *pBuf = NULL;
    WCHAR wBuffer[1024] = L"";
    char tBuf[100], tLocalHost[100];
    DWORD ret_val;

    UpdateData();

    HCURSOR hOldCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
    
    if (m_domain == "")
    {
	ret_val = NetServerEnum(
	    NULL, 
	    100,
	    (LPBYTE*)&pBuf,
	    MAX_PREFERRED_LENGTH,
	    &num_read,
	    &total,
	    SV_TYPE_NT, 
	    NULL,
	    0);
    }
    else
    {
	WCHAR wDomain[100];
	mbstowcs(wDomain, m_domain, 100);
	ret_val = NetServerEnum(
	    NULL, 
	    100,
	    (LPBYTE*)&pBuf,
	    MAX_PREFERRED_LENGTH,
	    &num_read,
	    &total,
	    SV_TYPE_NT, 
	    (LMCSTR)wDomain,
	    0);
    }
    
    if (ret_val == NERR_Success)
    {
	size = 100;
	GetComputerName(tLocalHost, &size);
	m_list.ResetContent();
	if (num_read == 0)
	{
	    m_list.InsertString(-1, tLocalHost);
	    m_list.SetSel(0);
	}
	else
	{
	    index = -1;
	    for (unsigned int i=0; i<num_read; i++)
	    {
		wcstombs(tBuf, (WCHAR*)pBuf[i].sv100_name, wcslen((WCHAR*)pBuf[i].sv100_name)+1);
		ret_val = m_list.InsertString(-1, tBuf);
		if (stricmp(tBuf, tLocalHost) == 0)
		    index = ret_val;
	    }
	    if (index != -1)
		m_list.SetSel(index);
	}
	NetApiBufferFree(pBuf);
    }
    else
    {
	sprintf(tBuf, "error: %d", ret_val);
	MessageBox(tBuf, "Unable to retrieve network host names");
    }
    
    SetCursor(hOldCursor);
}

void CMPIConfigDlg::OnSetBtn()
{
    int i;
    int num_hosts = m_host_list.GetCount();
    char pszStr[4096];
    char hoststring[4096] = "";
    char host[100];
    int bfd;
    
    if (num_hosts == 0)
	return;
    
    CRegistrySettingsDialog dlg;
    if (dlg.DoModal() == IDCANCEL)
	return;
    
    if (dlg.m_bHostsChk == FALSE && dlg.m_bTempChk == FALSE && dlg.m_bLaunchTimeoutChk == FALSE)
	return;
    
    if (m_bNeedPassword)
    {
	CPwdDialog dlg;
	dlg.DoModal();
	if (dlg.m_bUseDefault)
	    strcpy(m_pszPhrase, MPD_DEFAULT_PASSPHRASE);
	else
	    strcpy(m_pszPhrase, dlg.m_password);
    }

    HCURSOR hOldCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
    
    // Create the host list
    for (i=0; i<num_hosts; i++)
    {
	if (m_host_list.GetText(i, host) == LB_ERR)
	{
	    SetCursor(hOldCursor);
	    MessageBox("GetText failed", "Error", MB_OK);
	    return;
	}
	strcat(hoststring, host);
	if (i<num_hosts-1)
	    strcat(hoststring, "|");
    }
    
    for (i=0; i<num_hosts; i++)
    {
	if (m_host_list.GetText(i, host) == LB_ERR)
	    continue;
	
	if (!ConnectToHost(host, m_nPort, m_pszPhrase, &bfd))
	    continue;
	
	if (dlg.m_bHostsChk)
	{
	    sprintf(pszStr, "lset hosts=%s", hoststring);
	    WriteString(bfd, pszStr);
	}
	if (dlg.m_bTempChk)
	{
	    sprintf(pszStr, "lset temp=%s", dlg.m_pszTempDir);
	    WriteString(bfd, pszStr);
	}
	if (dlg.m_bLaunchTimeoutChk)
	{
	    sprintf(pszStr, "lset timeout=%d", dlg.m_nLaunchTimeout);
	    WriteString(bfd, pszStr);
	}
	WriteString(bfd, "done");
	beasy_closesocket(bfd);
    }
    SetCursor(hOldCursor);
}

LRESULT CMPIConfigDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if (message == WM_USER+1)
    {
	if (lParam)
	    m_list.SetSel((int)wParam, TRUE);
	else
	{
	    if ((int)wParam != -1)
		m_list.SetSel((int)wParam, FALSE);
	    m_num_threads--;
	    if (m_num_threads == 0)
	    {
		m_find_btn.EnableWindow();
		m_refresh_btn.EnableWindow();
		m_set_btn.EnableWindow();
		m_edit_add_btn.EnableWindow();
	    }
	}
    }
    if (message == WM_CHAR)
    {
	if (wParam == VK_RETURN)
	{
	    MessageBox("enter key pressed");
	}
    }
    return CDialog::WindowProc(message, wParam, lParam);
}

/*
void CMPIConfigDlg::OnAddBtn() 
{
    // Parse list window and add to host list
    int i;
    int *iHosts;
    int num_hosts = m_list.GetSelCount();
    char host[100];
    
    UpdateData();

    if (num_hosts == 0)
	return;
    
    // Create the host list
    iHosts = new int[num_hosts];
    if (m_list.GetSelItems(num_hosts, iHosts) == LB_ERR)
    {
	MessageBox("GetSelItems failed", "Error", MB_OK);
	return;
    }
    for (i=0; i<num_hosts; i++)
    {
	if (m_list.GetText(iHosts[i], host) == LB_ERR)
	{
	    MessageBox("GetText failed", "Error", MB_OK);
	    return;
	}
	CString str;
	int n = m_host_list.GetCount();
	if (n != LB_ERR)
	{
	    bool bFound = false;
	    for (int i=0; i<n; i++)
	    {
		m_host_list.GetText(i, str);
		if (str.CompareNoCase(host) == 0)
		{
		    bFound = true;
		    //break;
		}
	    }
	    if (!bFound)
	    {
		m_host_list.InsertString(-1, host);
	    }
	}
    }
}
*/

void CMPIConfigDlg::OnEditAddBtn() 
{
    int i;
    int *iHosts;
    int num_hosts = m_list.GetSelCount();
    char host[100];
    
    // Add hostname to host list
    UpdateData();
    
    if (m_hostname.GetLength() != 0)
    {
	
	CString str;
	int n = m_host_list.GetCount();
	if (n != LB_ERR)
	{
	    bool bFound = false;
	    for (int i=0; i<n; i++)
	    {
		m_host_list.GetText(i, str);
		if (str.CompareNoCase(m_hostname) == 0)
		{
		    bFound = true;
		    //break;
		}
	    }
	    if (!bFound)
	    {
		m_host_list.InsertString(-1, m_hostname);
	    }
	}
    }

    // Parse list window and add to host list
    if (num_hosts == 0)
	return;
    
    // Create the host list
    iHosts = new int[num_hosts];
    if (m_list.GetSelItems(num_hosts, iHosts) == LB_ERR)
    {
	MessageBox("GetSelItems failed", "Error", MB_OK);
	return;
    }
    for (i=0; i<num_hosts; i++)
    {
	if (m_list.GetText(iHosts[i], host) == LB_ERR)
	{
	    MessageBox("GetText failed", "Error", MB_OK);
	    return;
	}
	CString str;
	int n = m_host_list.GetCount();
	if (n != LB_ERR)
	{
	    bool bFound = false;
	    for (int i=0; i<n; i++)
	    {
		m_host_list.GetText(i, str);
		if (str.CompareNoCase(host) == 0)
		{
		    bFound = true;
		}
	    }
	    if (!bFound)
	    {
		m_host_list.InsertString(-1, host);
	    }
	}
    }
}

int CMPIConfigDlg::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex) 
{
    if (*pListBox == m_host_list)
    {
	if (nKey == VK_DELETE)
	{
	    int index = m_host_list.GetCurSel();
	    if (index != LB_ERR)
	    {
		m_host_list.DeleteString(index);
		if (m_host_list.SetCurSel(index) == LB_ERR)
		    m_host_list.SetCurSel(index-1);
	    }
	}
    }
	return CDialog::OnVKeyToItem(nKey, pListBox, nIndex);
}

void CMPIConfigDlg::OnClose() 
{
    bsocket_finalize();
	CDialog::OnClose();
}

void CMPIConfigDlg::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    
    if (nType != SIZE_MINIMIZED)
    {
	/*
	if (m_nMinWidth != -1)
	{
	    if (cx < m_nMinWidth || cy < m_nMinHeight)
	    {
		RECT r, r2;
		r.left = 0;
		r.top = 0;
		r.right = m_nMinWidth;
		r.bottom = m_nMinHeight;
		AdjustWindowRect(&r, WS_CHILD, FALSE);
		GetWindowRect(&r2);
		MoveWindow(r2.left, r2.top, r.right-r.left, r.bottom-r.top, TRUE);
	    }
	}
	*/

	rList.Resize(cx, cy);
	rOk.Resize(cx, cy);
	rCancel.Resize(cx, cy);
	rBox.Resize(cx, cy);
	rFind.Resize(cx, cy);
	rRefresh.Resize(cx, cy);
	rDomain.Resize(cx, cy);
	rStatic.Resize(cx, cy);
    }
}
