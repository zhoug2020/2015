// choujiangDlg.cpp : implementation file
//

#include "stdafx.h"
#include "choujiang.h"
#include "choujiangDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int WM_RESET = WM_USER+100;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoujiangDlg dialog

CChoujiangDlg::CChoujiangDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChoujiangDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChoujiangDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChoujiangDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChoujiangDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	   DDX_Control(pDX, IDC_STATIC_NAME, m_stname);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChoujiangDlg, CDialog)
	//{{AFX_MSG_MAP(CChoujiangDlg)
	ON_MESSAGE(WM_RESET,OnReset)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTCHOUJIANG, OnBtchoujiang)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTSTOP, OnBtstop)
	//}}AFX_MSG_MAP
	ON_WM_KEYUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoujiangDlg message handlers

BOOL CChoujiangDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	//char buff[255];

	//GetModuleFileName(NULL,buff,255);

	//CString strCurPath = buff;

	//int i = strCurPath.ReverseFind('\\');

	//strCurPath = strCurPath.Left(i);

	//CString	strPicPath =  strCurPath + "\\pic";

	//SearchPicFile(strPicPath);

	//if(m_list.GetCount() < 0 )
	//{
	//	AfxMessageBox("图片文件PIC无抽奖图片");
	//}

	srand((unsigned int)time(NULL));
	blCj = FALSE;
	setButtonBmp();
	setInfobmp(false);
	static CFont   font;                                     
	CStatic*pStatic=(CStatic*)GetDlgItem(IDC_STATIC_NAME);

	//font.CreatePointFont(300,_T("Arial"));
	VERIFY(font.CreateFont(
	   25,                        // nHeight
	   0,                         // nWidth
	   0,                         // nEscapement
	   0,                         // nOrientation
	   FW_NORMAL,                 // nWeight
	   FALSE,                     // bItalic
	   FALSE,                     // bUnderline
	   0,                         // cStrikeOut
	   ANSI_CHARSET,              // nCharSet
	   OUT_DEFAULT_PRECIS,        // nOutPrecision
	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   DEFAULT_QUALITY,           // nQuality
	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   L"楷体"));                 // lpszFacename


	pStatic->SetFont(&font);

	//pStatic->ShowWindow(SW_HIDE);

	m_stname.SetWindowText(_T("美女007"));

	readInifile();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChoujiangDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChoujiangDlg::OnPaint() 
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
		SetPic(m_strPicPath,IDC_STATIC_PIC);
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChoujiangDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CChoujiangDlg::SearchPicFile(CString &strPath)
{
	CString strFile = strPath += _T("\\*.*");
	CFileFind finder;
	CString strExt;
	BOOL bWorking = finder.FindFile(strFile);
	if ( !bWorking)
	{
		MessageBox(_T("图片目录PIC不存在!"),_T("春节联欢抽奖"),MB_ICONEXCLAMATION);
		finder.Close();

		return false;
	}

	while(bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots() || finder.IsDirectory())
		{
			continue;
		}

		int nPos = finder.GetFileName().Find('.');
		
		strExt = finder.GetFileName().Right(
				finder.GetFileName().GetLength()- nPos -1);

		//AfxMessageBox(strExt);
	
		if(!strExt.CompareNoCase(_T("jpg")) || !strExt.CompareNoCase(_T("bmp")))
		{
			m_list.AddTail(finder.GetFileName());
		}

	}
	
	finder.Close();

	return TRUE;
	
}

void CChoujiangDlg::OnBtchoujiang() 
{
	if(blCj)
		return;

	wchar_t buff[255];

	GetModuleFileName(NULL,buff,255);

	CString strCurPath = buff;

	int i = strCurPath.ReverseFind('\\');

	strCurPath = strCurPath.Left(i);

	CString	strPicPath =  strCurPath + "\\pic";

	//清空从新计算
	m_list.RemoveAll();

	if(SearchPicFile(strPicPath))
	{
		//TRACE("list count %d\n",m_list.GetCount());

		if(m_list.GetCount() <= 0 )
		{
			MessageBox(_T("图片文件夹PIC里无抽奖图片"),_T("春节联欢抽奖"),MB_ICONEXCLAMATION);
			return;
		}
		else
		{
			//remove winner
			CString strVal;
			POSITION pos = m_dellist.GetHeadPosition();
			while(pos && (m_list.GetCount() > 0))
			{
				strVal = m_dellist.GetNext(pos);
				POSITION pos1 = m_list.Find(strVal);
				if(pos1)
					m_list.RemoveAt(pos1);
			}
			if(m_list.GetCount() <= 0 )
			{
				int nRet = MessageBox(_T("所有人都中奖了，是否重置！！！"),_T("春节联欢抽奖"),MB_YESNO|MB_ICONINFORMATION);
				if(IDYES == nRet)
				{
					this->SendMessage(WM_RESET,0,0);
				}
				return;
			}
			//TRACE("list count %d\n",m_list.GetCount());
			//pos = m_list.GetHeadPosition();
			//while(pos)
			//{
			//	CString strVal = m_list.GetNext(pos);
			//	TRACE(strVal + _T("\n"));
			//}
			SetTimer(1,10,NULL);
			setInfobmp(false);
			blCj = TRUE;
			//GetDlgItem(IDC_STATIC_ZJ)->SetWindowText("");
			GetDlgItem(IDC_BTCHOUJIANG)->EnableWindow(false);
			//GetDlgItem(IDC_STATIC_NAME)->ShowWindow(SW_SHOW);
		}
	}
	// TODO: Add your control notification handler code here

}

void CChoujiangDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	CString strFileName;
	CString strFileTitle;

	int nVal = getRandNo(m_list.GetCount());

/*	CString strMsg;
	strMsg.Format("%d",nVal);
	AfxMessageBox(strMsg); */
	
	int nPos = nVal -1 ;

	POSITION pos = m_list.FindIndex(nPos);

	if(NULL != pos)
	{
		strFileName = m_list.GetAt(pos);
		strFileTitle = strFileName.Left(strFileName.Find('.'));
		GetDlgItem(IDC_STATIC_NAME)->SetWindowText(strFileTitle);
		m_strFileName = strFileName;
	}

	// 绝对路径
	wchar_t buff[255];
	GetModuleFileName(NULL,buff,255);

	CString strFilePath = buff;

	int i = strFilePath.ReverseFind('\\');

	strFilePath = strFilePath.Left(i);

	strFilePath += "\\pic\\" + strFileName;

	SetPic(strFilePath,IDC_STATIC_PIC);

	UpdateData(FALSE);

	CDialog::OnTimer(nIDEvent);
}

void CChoujiangDlg::SetPic(LPCTSTR  lpszName, UINT uCtrlResource)
{
	
	CRect rcShow;

	CStatic* pStatic = (CStatic *)GetDlgItem(uCtrlResource);

	CDC *pShowDC = pStatic->GetDC();    

	pStatic->GetClientRect(&rcShow);

	CImage image;
	//image.Load("D:\\choujiang\\pic\\aaa1.bmp");
	image.Load(lpszName);

	if(image.IsDIBSection())
	{
		//CString  strout;
		//strout.Format(_T("IMAGE width=%d,height=%d\n"),image.GetWidth(),image.GetHeight());
		//TRACE(strout);
		//strout.Format(_T("RECT width=%d,height=%d\n"),rcShow.Width(),rcShow.Height());
		//TRACE(strout);
		if(image.GetWidth() >= rcShow.Width() && image.GetHeight() >=  rcShow.Height())
		{
			image.BitBlt(pShowDC->m_hDC,0,0,rcShow.Width(),rcShow.Height(),0,0,SRCCOPY);
		}
		else
		{
			image.Draw(pShowDC->m_hDC,0,0,rcShow.Width(),rcShow.Height(),

				0,0,image.GetWidth(),image.GetHeight());
		}
		/*image.BitBlt(pShowDC->m_hDC,0,0,rcShow.Width(),rcShow.Height(),0,0,SRCCOPY);*/
		m_strPicPath = lpszName;
	}

	pStatic->ReleaseDC(pShowDC);   
	
}

void CChoujiangDlg::OnBtstop() 
{
	// TODO: Add your control notification handler code here
	if(blCj)
	{
		KillTimer(1);
		GetDlgItem(IDC_BTCHOUJIANG)->EnableWindow(true);
		//GetDlgItem(IDC_STATIC_ZJ)->SetWindowText("恭喜您中将了！！！");
		setInfobmp(true);
		//delete the winner
		POSITION tmpPos = m_dellist.Find(m_strFileName);
		if(NULL == tmpPos)
		{
			m_dellist.AddTail(m_strFileName);
		}
		//trace m_dellist
		//POSITION pos = m_dellist.GetHeadPosition();
		//while(pos)
		//{
		//	CString strVal = m_dellist.GetNext(pos);
		//	TRACE(strVal + _T("\n"));
		//}
		blCj = FALSE;
	}
}

// 取1- nMAX随机数 (X-Y) K = rand()%(y-x+1) + x
int CChoujiangDlg::getRandNo(int nMax)
{
	if(nMax == 0) return 0;
	int k = rand()% nMax + 1;
	return k;
}

BOOL CChoujiangDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(pMsg->wParam == VK_SPACE)
			{
				OnBtchoujiang();
				return TRUE;
			}
			if(pMsg->wParam == VK_RETURN)
			{
				OnBtstop();
				return TRUE;
			}

			if(pMsg->wParam == VK_ESCAPE)
			{
				return FALSE;
			}

			if(pMsg->wParam == VK_F9)
			{
				if(m_dellist.GetCount() >0)
				{
					int nRet = MessageBox(_T("是否重置,重新开始游戏？"),_T("春节联欢抽奖"),MB_YESNO|MB_ICONQUESTION);
					if(IDYES == nRet)
					{
						this->SendMessage(WM_RESET,0,0);
						return FALSE;
					}
				}
				return CDialog::PreTranslateMessage(pMsg);
			}
	}


	return CDialog::PreTranslateMessage(pMsg);
}

void CChoujiangDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CChoujiangDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//
	CDC memDc;
	CRect rc;
	GetWindowRect(&rc);
	CBitmap bm;
	BITMAP btmap;
	CBitmap *pOldbm;
	if(bm.LoadBitmap(IDB_BKGROUND))
	{
		bm.GetBitmap(&btmap);
		memDc.CreateCompatibleDC(pDC);
		pOldbm = memDc.SelectObject(&bm);
		pDC->BitBlt(0,0,rc.Width(),rc.Height(),&memDc,0,0,SRCCOPY);
		//pDC->StretchBlt(0,0,rc.Width(),rc.Height(),&memDc,0,0,
		//	btmap.bmWidth,btmap.bmHeight,SRCCOPY);
		//memDc.SelectObject(pOldbm);
		return true;
	}

	return CDialog::OnEraseBkgnd(pDC);
}

HBRUSH CChoujiangDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_NAME)
	{
		pDC->SetTextColor(RGB(0,255,0));

		pDC->SetBkColor(RGB(255,255,255));//设置文本背景色

		pDC->SetBkMode(TRANSPARENT);//设置背景透明
		//static HBRUSH _hbr = ::CreateSolidBrush(RGB(128,0,0));
		//return _hbr;

	}
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_PIC)
	{
		//pDC->SetBkMode(TRANSPARENT);//设置背景透明
		//static HBRUSH _hbr = ::CreateSolidBrush(RGB(128,0,0));
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CChoujiangDlg::setButtonBmp()
{
	//HBITMAP hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BTCHOUJIANG));
	//((CBitmapButton*)GetDlgItem(IDC_BTCHOUJIANG))->LoadBitmaps(IDB_BTCHOUJIANG);
	//((CButton*)GetDlgItem(IDC_BTCHOUJIANG))->SetBitmap(hbm);
	//hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BTTINGZHI));
	//((CButton*)GetDlgItem(IDC_BTSTOP))->SetBitmap(hbm);

	m_btchoujiang.LoadBitmaps(IDB_BTCHOUJIANG, IDB_BTCHOUJIANGUNABLE, IDB_BTCHOUJIANG, IDB_BTCHOUJIANGUNABLE); 
	m_btstop.LoadBitmaps(IDB_BTTINGZHI,IDB_BTSTOPDOWN,IDB_BTTINGZHI);

	m_btchoujiang.SubclassDlgItem(IDC_BTCHOUJIANG,this);
	m_btstop.SubclassDlgItem(IDC_BTSTOP,this);
	// Load the bitmaps for this button.
	m_btchoujiang.SizeToContent();
	m_btstop.SizeToContent();

	return;	
}

void CChoujiangDlg::setInfobmp(BOOL blflag)
{
	if(blflag)
	{
		HBITMAP hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_INFO));
		((CStatic*)GetDlgItem(IDC_STATIC_INFO))->SetBitmap(hbm);
	}
	else
	{
		HBITMAP hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_INFO2));
		((CStatic*)GetDlgItem(IDC_STATIC_INFO))->SetBitmap(hbm);
	}
	return;
}

//void CChoujiangDlg::testFont(CString strVal)
//{
//	CFont font;
//	VERIFY(font.CreateFont(
//	   30,                        // nHeight
//	   0,                         // nWidth
//	   0,                         // nEscapement
//	   0,                         // nOrientation
//	   FW_NORMAL,                 // nWeight
//	   FALSE,                     // bItalic
//	   FALSE,                     // bUnderline
//	   0,                         // cStrikeOut
//	   ANSI_CHARSET,              // nCharSet
//	   OUT_DEFAULT_PRECIS,        // nOutPrecision
//	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
//	   DEFAULT_QUALITY,           // nQuality
//	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
//	   "Arial"));                 // lpszFacename
//
//	// Do something with the font just created...
//	//	CClientDC dc(this);  
//	CDC *pDC;
//	pDC =  GetDlgItem(IDC_STATIC_NAME)->GetDC();
//	CFont* def_font =	pDC->SelectObject(&font);
//	//pDC->TextOut(5, 5, "Hello", 5);
//	//pDC->TextOut(5, 5, "Hello_____________", 5);
//	pDC->TextOut(5, 5, strVal);
//	//GetDlgItem(IDC_STATIC_TEST)->SetWindowText(_T("美女AAFB"));
//	pDC->SelectObject(def_font);
//	
//	// Done with the font.  Delete the font object.
//	font.DeleteObject(); 
//
//}

void CChoujiangDlg::setStaticFont(UINT uId)
{

	CFont   font;                                     
	CStatic*pStatic=(CStatic*)GetDlgItem(uId);

	if(NULL != pStatic)
	{

		font.CreatePointFont(266,_T("Arial"));
		//VERIFY(font.CreateFont(
		//   560,                        // nHeight
		//   0,                         // nWidth
		//   0,                         // nEscapement
		//   0,                         // nOrientation
		//   FW_NORMAL,                 // nWeight
		//   FALSE,                     // bItalic
		//   FALSE,                     // bUnderline
		//   0,                         // cStrikeOut
		//   ANSI_CHARSET,              // nCharSet
		//   OUT_DEFAULT_PRECIS,        // nOutPrecision
		//   CLIP_DEFAULT_PRECIS,       // nClipPrecision
		//   DEFAULT_QUALITY,           // nQuality
		//   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		//   "Arial"));                 // lpszFacename


		pStatic->SetFont(&font);
	}
	
}

//void CChoujiangDlg::setPicbk(UINT uId)
//{
//	HBITMAP hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(uId));
//	((CStatic*)GetDlgItem(IDC_STATIC_PIC))->SetBitmap(hbm);
//	return;
//}

LRESULT CChoujiangDlg::OnReset(WPARAM wParam,LPARAM lParam)
{
	m_dellist.RemoveAll();
	return 0;
}
void CChoujiangDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TCHAR curDir[255];
	TCHAR filename[255];
	::GetModuleFileName(NULL,curDir,255);
	TCHAR* pos = _tcsrchr(curDir,'\\');
	if(NULL != pos)
	{
		*pos = 0;
	}
	wcscat(curDir, _T("\\cj.ini"));
	wcscpy(filename,curDir);
	
	CFile file;

    try
    {
	       // try to open the file
		if(file.Open(filename,CFile::modeCreate | CFile::modeWrite))
		{

			POSITION pos = m_dellist.GetHeadPosition();
			CString strVal;
			int size = 0;
			while(pos)
			{
				strVal = m_dellist.GetNext(pos);
				strVal += _T("*");
				file.Write(strVal,strVal.GetLength());
				size += strVal.GetLength();
				if(size >= 2000)
				{
					file.Write("\n",1);
					size = 0;
				}
			}
		}
       
    }
    catch (CFileException* pEx)
    {
		file.Close();
		pEx->ReportError();
		pEx->Delete();

    }

	file.Close();

	CDialog::OnClose();
}


void CChoujiangDlg::readInifile()
{
	TCHAR curDir[255];
	TCHAR filename[255];
	::GetModuleFileName(NULL,curDir,255);
	TCHAR* pos = _tcsrchr(curDir,'\\');
	if(NULL != pos)
	{
		*pos = 0;
	}
	wcscat(curDir,_T("\\cj.ini"));
	wcscpy(filename,curDir);

	CFile file;
	TCHAR buff[2001];
	int nCount;
	CString strVal;
	try
	{
		if(file.Open(filename,CFile::modeRead))
		{
			while(TRUE)
			{
				nCount = file.Read(buff,2000);
				if(nCount <= 0)
				{
					break;
				}
				if(nCount <= 2000)
				{
					buff[nCount] = '\0';
					TCHAR* p = buff;
					while(*p != _T('\0'))
					{
						if(*p == _T('*'))
						{
							p++;
							m_dellist.AddTail(strVal);
							strVal = "";
							continue;
						}
						strVal += *p;
						p++;
					}
				}

			}

			file.Close();
		}

    }
    catch (CFileException* pEx)
    {
		file.Close();
		pEx->ReportError();
		pEx->Delete();

    }

	//POSITION ps = m_dellist.GetHeadPosition();
	//while(ps)
	//{
	//	strVal = m_dellist.GetNext(ps);
	//	TRACE(strVal + "\n");
	//}

}