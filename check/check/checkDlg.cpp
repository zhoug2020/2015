// checkDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "check.h"
#include "checkDlg.h"
#include "FileCheck.h"
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CcheckDlg 对话框




CcheckDlg::CcheckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CcheckDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CcheckDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_BUTTON1, &CcheckDlg::OnBnClickedButton1)
   ON_BN_CLICKED(IDC_BTFOLDER, &CcheckDlg::OnBnClickedBtfolder)
   ON_BN_CLICKED(IDC_BTFILE, &CcheckDlg::OnBnClickedBtfile)
END_MESSAGE_MAP()


// CcheckDlg 消息处理程序

BOOL CcheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

   ((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);//选上

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CcheckDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CcheckDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CcheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcheckDlg::OnBnClickedButton1()
{
   FileCheck checkCls;
   std::vector<std::string> files;
   if (((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck())
   {
      CString strFolder;
      GetDlgItem(IDC_EDITFOLDER)->GetWindowText(strFolder);
      traverseDir(strFolder,files);
      checkCls.check(files);
   }
   else 
   {
      CString strfileName;
      GetDlgItem(IDC_EDITFILE)->GetWindowText(strfileName);
      USES_CONVERSION;
      std::string fileName(W2A(strfileName));
      if (!fileName.empty())
      {
         files.push_back(fileName);
         checkCls.check(files);
      }
   }
}

void CcheckDlg::OnBnClickedBtfolder()
{
   wchar_t szPath[MAX_PATH]; 
   CString str;

   ZeroMemory(szPath, sizeof(szPath));   

   BROWSEINFO bi;   
   bi.hwndOwner = m_hWnd;   
   bi.pidlRoot = NULL;   
   bi.pszDisplayName = szPath;   
   bi.lpszTitle = L"please select the folder：";   
   bi.ulFlags = 0;   
   bi.lpfn = NULL;   
   bi.lParam = 0;   
   bi.iImage = 0;   

   LPITEMIDLIST lp = SHBrowseForFolder(&bi);   

   if(lp && SHGetPathFromIDList(lp, szPath))   
   {
      GetDlgItem(IDC_EDITFOLDER)->SetWindowText(szPath);
   }
}

void CcheckDlg::OnBnClickedBtfile()
{
   CString strFile = _T("");

   CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, 
      _T("Describe Files All Files (*.*)|*.*"
      _T("|(*.hpp)|*.hpp")
      _T("|(*.cpp)|*.cpp")
      _T("|(*.h)|*.h")
      _T("||")), NULL);

   if (dlgFile.DoModal())
   {
      strFile = dlgFile.GetPathName();
      GetDlgItem(IDC_EDITFILE)->SetWindowText(strFile);
   }
}

void CcheckDlg::traverseDir(CString& dir, std::vector<std::string>& vec)
{
   CFileFind ff;
   if (dir.Right(1) != "\\")
      dir += "\\";
   dir += "*.*";

   BOOL ret = ff.FindFile(dir);
   while (ret)
   {
      ret = ff.FindNextFile();
      if (ret != 0)
      {
         if (ff.IsDirectory() && !ff.IsDots())
         {
            CString path = ff.GetFilePath();
            traverseDir(path, vec);
         }
         else if (!ff.IsDirectory() && !ff.IsDots())
         {
            CString path = ff.GetFilePath();
            USES_CONVERSION;
            std::string fileName(W2A(path));
            vec.push_back(fileName);
         }
      }
   }

   ff.Close();
}