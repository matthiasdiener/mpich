// HtmlControl.cpp: implementation of the CHtmlControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "guimpirun.h"
#include "HtmlCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CHtmlCtrl, CWnd)

BEGIN_MESSAGE_MAP(CHtmlCtrl, CWnd)
    //{{AFX_MSG_MAP(CHtmlCtrl)
    //}}AFX_MSG_MAP
    // Standard printing commands
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHtmlCtrl, CWnd)
/*
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 102, OnStatusTextChange, VTS_BSTR)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 108, OnProgressChange, VTS_I4 VTS_I4)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 105, OnCommandStateChange, VTS_I4 VTS_BOOL)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 106, OnDownloadBegin, VTS_NONE)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 104, OnDownloadComplete, VTS_NONE)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 113, OnTitleChange, VTS_BSTR)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 252, NavigateComplete2, VTS_DISPATCH VTS_PVARIANT)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 250, BeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 112, OnPropertyChange, VTS_BSTR)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 251, OnNewWindow2, VTS_PDISPATCH VTS_PBOOL)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 259, DocumentComplete, VTS_DISPATCH VTS_PVARIANT)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 253, OnQuit, VTS_NONE)
    ON_EVENT(CHtmlCtrl, AFX_IDW_PANE_FIRST, 254, OnVisible, VTS_BOOL)
    */
END_EVENTSINK_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHtmlCtrl::CHtmlCtrl()
{
    m_pBrowser = NULL;
}

CHtmlCtrl::~CHtmlCtrl()
{
    if (m_pBrowser != NULL)
        m_pBrowser->Release();
    m_pBrowser = NULL;
}

bool CHtmlCtrl::Create(CWnd* pParent, const RECT& rc, LPCTSTR pszHomeURL /*= NULL*/, bool fBtnText /*= true*/)
{
    AfxEnableControlContainer();
    CRect rcClient(rc);

    if (!CreateControl(CLSID_WebBrowser, NULL, 
            WS_VISIBLE | WS_CHILD, rcClient, pParent, AFX_IDW_PANE_FIRST)) {
        TRACE0("Unable to create control");
        return false;
    }

    LPUNKNOWN lpUnk = GetControlUnknown();
    HRESULT hr = lpUnk->QueryInterface(IID_IWebBrowser2, (void**) &m_pBrowser);
    if (FAILED(hr)) {
        m_pBrowser = NULL;
        TRACE0("Unable to create web browser");
        return false;
    }

    return true;
}

bool CHtmlCtrl::ReplaceControl(CWnd* pDlg, UINT idCtrl, LPCTSTR pszHomeURL /*= NULL*/, bool fBtnText /*= true*/)
{
    CRect rc;
    pDlg->GetDlgItem(idCtrl)->GetWindowRect(&rc);
    pDlg->ScreenToClient(&rc);
    pDlg->GetDlgItem(idCtrl)->DestroyWindow();

    return Create(pDlg, rc, pszHomeURL, fBtnText);
}

BOOL CHtmlCtrl::LoadFromResource(UINT nRes)
{
    HINSTANCE hInstance = AfxGetResourceHandle();
    ASSERT(hInstance != NULL);
    
    CString strResourceURL;
    BOOL bRetVal = TRUE;
    LPTSTR lpszModule = new TCHAR[_MAX_PATH];
    
    if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
    {
	strResourceURL.Format(_T("res://%s/%d"), lpszModule, nRes);
	Navigate(strResourceURL, 0, 0, 0);
    }
    else
	bRetVal = FALSE;
    
    delete [] lpszModule;
    return bRetVal;
}

HRESULT CHtmlCtrl::Navigate(LPCTSTR lpszURL, 
			    DWORD dwFlags /*= 0*/,
			    LPCTSTR lpszTargetFrameName /*= NULL*/,
			    LPCTSTR lpszHeaders /*= NULL*/, 
			    LPVOID lpvPostData /*= NULL*/,
			    DWORD dwPostDataLen /*= 0*/)
{
    CString strURL(lpszURL);
    BSTR bstrURL = strURL.AllocSysString();

    COleSafeArray vPostData;
    if (lpvPostData != NULL)
    {
        if (dwPostDataLen == 0)
            dwPostDataLen = lstrlen((LPCTSTR) lpvPostData);

        vPostData.CreateOneDim(VT_UI1, dwPostDataLen, lpvPostData);
    }

    return m_pBrowser->Navigate(bstrURL,
        COleVariant((long) dwFlags, VT_I4),
        COleVariant(lpszTargetFrameName, VT_BSTR),
        vPostData,
        COleVariant(lpszHeaders, VT_BSTR));
}

#ifdef _DEBUG
void CHtmlCtrl::AssertValid() const
{
    CWnd::AssertValid();
}

void CHtmlCtrl::Dump(CDumpContext& dc) const
{
    CWnd::Dump(dc);
}
#endif
