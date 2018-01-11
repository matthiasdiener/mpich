// MPIConfigDlg.h : header file
//

#if !defined(AFX_MPICONFIGDLG_H__0095789A_A062_11D3_95FB_009027106653__INCLUDED_)
#define AFX_MPICONFIGDLG_H__0095789A_A062_11D3_95FB_009027106653__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resizer.h"

/////////////////////////////////////////////////////////////////////////////
// CMPIConfigDlg dialog

class CMPIConfigDlg : public CDialog
{
    // Construction
public:
    CMPIConfigDlg(CWnd* pParent = NULL);	// standard constructor
    
    HANDLE m_hFindThread;
    DWORD m_num_threads;
    int m_nPort;
    char m_pszHost[100];
    char m_pszPhrase[100];
    bool m_bNeedPassword;
    void ParseRegistry();
    
    Resizer rRefresh, rFind, rOk, rCancel, rList, rBox, rDomain, rStatic;
    int m_nMinWidth, m_nMinHeight;

    // Dialog Data
    //{{AFX_DATA(CMPIConfigDlg)
	enum { IDD = IDD_MPICONFIG_DIALOG };
	CEdit	m_domain_edit;
	CEdit	m_static_edit;
    CButton	m_ok_btn;
    CButton	m_cancel_btn;
    CButton	m_edit_add_btn;
    CListBox	m_host_list;
    CButton	m_set_btn;
    CButton	m_refresh_btn;
    CButton	m_find_btn;
    CListBox	m_list;
    CString	m_hostname;
	CString	m_domain;
	CString	m_static;
	//}}AFX_DATA
    
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMPIConfigDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL
    
    // Implementation
protected:
    HICON m_hIcon;
    
    // Generated message map functions
    //{{AFX_MSG(CMPIConfigDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnFindBtn();
    afx_msg void OnRefreshBtn();
    afx_msg void OnSetBtn();
    afx_msg void OnEditAddBtn();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MPICONFIGDLG_H__0095789A_A062_11D3_95FB_009027106653__INCLUDED_)
