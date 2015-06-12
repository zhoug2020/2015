// checkDlg.h : 头文件
//

#pragma once
#include <vector>

// CcheckDlg 对话框
class CcheckDlg : public CDialog
{
// 构造
public:
	CcheckDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CHECK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
   void traverseDir(CString& dir, std::vector<std::string>& vec);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedButton1();
public:
   afx_msg void OnBnClickedBtfolder();
public:
   afx_msg void OnBnClickedBtfile();
};
