// checkDlg.h : ͷ�ļ�
//

#pragma once
#include <vector>

// CcheckDlg �Ի���
class CcheckDlg : public CDialog
{
// ����
public:
	CcheckDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CHECK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
   void traverseDir(CString& dir, std::vector<std::string>& vec);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
