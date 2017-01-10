// choujiangDlg.h : header file
//

#if !defined(AFX_CHOUJIANGDLG_H__C4E94DF8_A84E_44DB_87C9_504C380EB83C__INCLUDED_)
#define AFX_CHOUJIANGDLG_H__C4E94DF8_A84E_44DB_87C9_504C380EB83C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlimage.h>
#include "TransparentStatic.h"

/////////////////////////////////////////////////////////////////////////////
// CChoujiangDlg dialog

class CChoujiangDlg : public CDialog
{
// Construction
public:
	CChoujiangDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CChoujiangDlg)
	enum { IDD = IDD_CHOUJIANG_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoujiangDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
private:
	CList<CString,CString&> m_list;
	CList<CString,CString&> m_dellist;
	BOOL blCj;
	HBITMAP   m_hBmp; 
	CString m_strPicPath;
	CString m_strFileName;
	CBitmapButton m_btchoujiang,m_btstop;
	CTransparentStatic m_stname;
	//FUNCTION
	int getRandNo(int nMax);
	void SetPic(LPCTSTR  lpszName,   UINT   uCtrlResource);
	BOOL SearchPicFile(CString& strPath);
	void setButtonBmp();
	void setInfobmp(BOOL blflag);
	//void testFont(CString strVal = "");
	void setStaticFont(UINT uId);
	void readInifile();
	// Generated message map functions
	//{{AFX_MSG(CChoujiangDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtchoujiang();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtstop();
	afx_msg LRESULT OnReset(WPARAM wParam,LPARAM lParam);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnClose();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOUJIANGDLG_H__C4E94DF8_A84E_44DB_87C9_504C380EB83C__INCLUDED_)
