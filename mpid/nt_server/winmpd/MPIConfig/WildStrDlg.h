#if !defined(AFX_WILDSTRDLG_H__9E9BF915_A442_4990_8D53_AE65390A3AA2__INCLUDED_)
#define AFX_WILDSTRDLG_H__9E9BF915_A442_4990_8D53_AE65390A3AA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WildStrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWildStrDlg dialog

class CWildStrDlg : public CDialog
{
// Construction
public:
	CWildStrDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWildStrDlg)
	enum { IDD = IDD_WILDCARD_DLG };
	CString	m_wildstr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWildStrDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWildStrDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WILDSTRDLG_H__9E9BF915_A442_4990_8D53_AE65390A3AA2__INCLUDED_)
