
// GetFlightsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GetFlights.h"
#include "GetFlightsDlg.h"
#include "afxdialogex.h"
#include "common/CStringToolEx.h"
#include "C9CHtmlParse.h"
#include "DbCenter.h"
#include "AQHtmlParse.h"
#include "KnHtmlParse.h"

#include "../../SharePlatform/common/ThreadPool.h"
#include "util.h"
#include "strconv.h"
#include "CCaHtmlParse.h"
#include "./tinyxml/tinystr.h"
#include "./tinyxml/tinyxml.h"

#include "CeairMobileE.h"

//联航6秒事件
HANDLE  CGetFlightsDlg::m_h6Second = NULL;
HANDLE  CGetFlightsDlg::m_h6Second2 = NULL;
//退出软件事件
HANDLE	CGetFlightsDlg::m_hExitEvt = NULL;
//继续执行事件
HANDLE	CGetFlightsDlg::m_hContinueDoEvt = NULL;
CGetFlightsDlg*	CGetFlightsDlg::m_pThis = NULL;
HWND CGetFlightsDlg::m_hDlgConfigHwnd = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ThreadPool g_threadPoolProcess(ThreadCount);

BOOL CGetFlightsDlg::m_bNoChangeFlightPoll = FALSE;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGetFlightsDlg 对话框




CGetFlightsDlg::CGetFlightsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGetFlightsDlg::IDD, pParent)
{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pDlgConfig = NULL;
	m_pDlgShowFlights = NULL;

	m_bCloseClicked = false;
	m_bStart = false;
	m_nThreadEndCount = 0;
	__SetCurThreadCount(0);

#ifdef GET_CA_FLIGHT
	m_hExitEvt = CreateEvent(NULL, TRUE, FALSE, _T("EXIT_ZL_GET_CA_TUAN_FLIGHT"));
	m_hContinueDoEvt = CreateEvent(NULL, TRUE, FALSE, _T("GET_CA_TUAN_FLIGHT"));
#else
	#ifdef GET_KN_FLIGHT
		m_h6Second = CreateEvent(NULL, TRUE, TRUE, _T("KN_6SECOND"));
		m_h6Second2 = CreateEvent(NULL, TRUE, TRUE, _T("KN_6SECOND2"));
		m_hExitEvt = CreateEvent(NULL, TRUE, FALSE, _T("EXIT_ZL_GET_KN_FLIGHT"));
		m_hContinueDoEvt = CreateEvent(NULL, TRUE, FALSE, _T("GET_KN_FLIGHT"));
	#else
	#ifdef GET_AQ_FLIGHT
		m_hExitEvt = CreateEvent(NULL, TRUE, FALSE, _T("EXIT_ZL_GET_AQ_FLIGHT"));
		m_hContinueDoEvt = CreateEvent(NULL, TRUE, FALSE, _T("GET_AQ_FLIGHT"));
	#else
	#ifdef CEAIR_MOBILE_E
		m_hExitEvt = CreateEvent(NULL, TRUE, FALSE, _T("EXIT_ZL_GET_MOBILEE_FLIGHT"));
		m_hContinueDoEvt = CreateEvent(NULL, TRUE, FALSE, _T("GET_MOBILEE_FLIGHT"));
	#else
		m_hExitEvt = CreateEvent(NULL, TRUE, FALSE, _T("EXIT_ZL_GET_FLIGHT"));
		m_hContinueDoEvt = CreateEvent(NULL, TRUE, FALSE, _T("GET_9C_FLIGHT"));
	#endif
	#endif
	#endif
#endif
	__LoadCaLowPriceAirLineList();
	
	m_caMapProcess.clear();

}

void CGetFlightsDlg::__SetCurThreadCount(UINT u)
{
	m_uThreadCount.Set(u);
}
UINT CGetFlightsDlg::__GetCurThreadCount(void)
{
	return m_uThreadCount.Get();
}
void CGetFlightsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
}

BEGIN_MESSAGE_MAP(CGetFlightsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CGetFlightsDlg::OnTcnSelchangeTab1)
	ON_WM_TIMER()
	ON_MESSAGE(WM_PROCESS_THREAD_END,OnProcessThreadEnd)
	ON_MESSAGE(WM_UPDATE_FLIGHT,OnUpdateFlight)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_QUERY_LESS_CHANGE_FLIGHT_OK, OnQueryLessChangeFlightOk)
END_MESSAGE_MAP()


// CGetFlightsDlg 消息处理程序

BOOL CGetFlightsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	m_tab.InsertItem(0,_T("设置"));
	m_tab.InsertItem(1,_T("网页"));
	m_pDlgConfig = new CDlgConfig();
	m_pDlgConfig->Create(CDlgConfig::IDD,&m_tab);

//	m_pDlgGetFlights = new CDlgBrowsers();
//	m_pDlgGetFlights->Create(CDlgBrowsers::IDD,&m_tab);

	//m_pDlgShowFlights = new CDlgShowFlights();
	//m_pDlgShowFlights->Create(CDlgShowFlights::IDD,&m_tab);

	LayoutCtrls();

	m_pThis = this;
	m_hDlgConfigHwnd = m_pDlgConfig->m_hWnd;

	//test
	//PT9CFlightInfo pInfo = new T9CFlightInfo;
	//pInfo->straCompany = "9C";	
	//pInfo->straFromCityCode = "SHA";
	//pInfo->straToCityCode = "CAN";	
	//pInfo->straFlightNo = "8835";	
	//pInfo->straFromDate = "2014-12-10";	
	//pInfo->straToDate = "2014-12-10";			
	//pInfo->straFromTime = "";		
	//pInfo->straToTime = "";	
	//pInfo->nMinHangPrice = 1280;
	//pInfo->nMinPrice = 540;
	//pInfo->nPrice1 = 710;
	//pInfo->nPrice2 = 540;
	//pInfo->nPrice3 = 0;
	//pInfo->bHavePolicy = false;
	//pInfo->straPrice1Json = "";
	//pInfo->straPrice2Json = "";
	//pInfo->straPrice3Json = "";
	////CDbCenter::GetInstance()->ClearAddTicketFlag(pInfo);
	//CDbCenter::GetInstance()->InsertFligthInfo(pInfo);

	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	CTime endTime(pDataProcess->m_uCaTuanGetFlightEndDate.sDate.wYear, pDataProcess->m_uCaTuanGetFlightEndDate.sDate.u8Month, pDataProcess->m_uCaTuanGetFlightEndDate.sDate.u8Day, 0, 0, 0);
	CCaHtmlParse::SetCaTuanGetFlightEndDate(endTime);
	CCaHtmlParse::SetAllCaTuanFlightFlag(pDataProcess->m_bGetAllCaTuanFlight);
	CCaHtmlParse::SetCaTuanMinTicketNumWarnNum(1);

	C9CHtmlParse::SetExtraAddPrice(pDataProcess->m_iExtraAddPrice);

	CCeairMobileE::SetOutLogFlag(pDataProcess->m_bOutCeairLog);
#ifdef CEAIR_MOBILE_E
	if (!CCeairMobileE::GetServerKey())
	{
		AfxMessageBox(_T("获取移动E SeverkKey 失败,软件将无法获取航班数据"));
	}
#endif

#ifndef GET_KN_FLIGHT
	#ifndef GET_CA_FLIGHT
	#ifndef GET_AQ_FLIGHT
		#ifdef CEAIR_MOBILE_E
			AfxBeginThread(__ThreadInsertMobileEJobToThreadPool, this);
		#else
			#if (SSS_CLIENT_NUM == SSS_1_CLIENT)
				AfxBeginThread(__ThreadInsertJobToThreadPoolClient1, this);
			#else
				AfxBeginThread(__ThreadInsertJobToThreadPool, this);
			#endif
		#endif
	#else
		AfxBeginThread(__ThreadInsertAqJobToThreadPool,this);
	#endif
	#else //ifdef GET_CA_FLIGHT
		AfxBeginThread(__ThreadInsertCaJobToThreadPool, this);
	#endif
#else
	AfxBeginThread(__ThreadInsertKnJobToThreadPool, this);
#endif

	CString strCap = _T("GetFlights");
#ifndef GET_KN_FLIGHT 
	#ifndef GET_CA_FLIGHT
		#ifndef GET_AQ_FLIGHT
			#ifdef CEAIR_MOBILE_E
				CString strEx = _T(" for Mobile E Client");
				strCap = strCap + strEx;
			#else
				#if (SSS_CLIENT_NUM == SSS_1_CLIENT)
					CString strEx = _T(" for 1 Client");
					strCap = strCap + strEx;
				#endif
			#endif
		#else
			CString strEx = _T(" for Aq Client ");
			strCap = strCap + strEx;
		#endif
	#else //ifdef GET_CA_FLIGHT
		CString strEx = _T(" for CA");
		strCap = strCap + strEx;
	#endif
#else //ifdef GET_CA_FLIGHT
	#if (GET_FLIGHT_TYPE == 0)
		CString strEx = _T(" for KN 5days");
	#elif (GET_FLIGHT_TYPE == 1)
		CString strEx = _T(" for KN 10days");
	#else
		CString strEx = _T(" for KN 15days");
	#endif
		strCap = strCap + strEx;
#endif
	SetWindowText(strCap);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGetFlightsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGetFlightsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGetFlightsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CGetFlightsDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->DestroyWindow();
		delete m_pDlgConfig;
	}

	if (m_pDlgShowFlights && m_pDlgShowFlights->GetSafeHwnd())
	{
		m_pDlgShowFlights->DestroyWindow();
		delete m_pDlgShowFlights;
	}

#if (SSS_CLIENT_NUM == SSS_1_CLIENT)
	ClearAllLineList();
	m_client1MapProcess.erase(m_client1MapProcess.begin(), m_client1MapProcess.end());
#else
	ClearProcessMap(m_mapProcess);
#endif
	
	__FreeFlightList(m_caLowPriceAirLineList);
	//__ReleaseCaLowPriceFlightList();

	return CDialogEx::DestroyWindow();
}


void CGetFlightsDlg::LayoutCtrls(void)
{
	CRect rc;
	GetClientRect(&rc);
	int cx = rc.Width();
	int cy = rc.Height();

	int nCulSel = 0;
	if (m_tab.GetSafeHwnd())
	{
		nCulSel = m_tab.GetCurSel();
		m_tab.SetWindowPos(&wndTop,0,0,cx,cy,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->SetWindowPos(&wndTop,2,22,cx - 4,cy - 24,(nCulSel == 0 ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE);
	}
	if (m_pDlgShowFlights && m_pDlgShowFlights->GetSafeHwnd())
	{
		m_pDlgShowFlights->SetWindowPos(&wndTop,2,22,cx - 4,cy - 24,(nCulSel == 1 ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE);
	}
}

void CGetFlightsDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	LayoutCtrls();

	*pResult = 0;
}


int CGetFlightsDlg::Start(bool bLoop)
{
	OutputDebugString(_T("开始处理//停止"));
	if (!bLoop && m_bStart)
	{
		Stop();
		return 0;
	}

	//整理要处理的航线日期信息，将同一个日期的数据放在一个list中，多个日期的数据形成一个map，以日期为key

	PTAirLineDateInfo pInfo = NULL;
	list<PTAirLineDateInfo>::iterator it;  
	CString strKey;
	map<CString,PTAirLineDateProcess>::iterator itMap;
	list<PTAirLineDateInfo> listAirDate;

	if (!(m_pDlgConfig && m_pDlgConfig->GetSafeHwnd() && m_pDlgConfig->m_listAirDateData.size() > 0))
	{
		AddLog(_T("没有要处理的航线信息"));
		return 0;
	}

	listAirDate = m_pDlgConfig->m_listAirDateData;

	AddLog(_T("\r\n开始处理...\r\n"));
	//先清空map
	ClearProcessMap(m_mapProcess);
	OutputDebugString(_T("\r\n开始取得航线日期信息\r\n"));
	for (it = listAirDate.begin(); it != listAirDate.end();it++)  
	{  
		pInfo = *it;
		CTime t = pInfo->tStart;
		CTimeSpan ts = pInfo->tEnd - t;
		while (ts.GetDays() >= 0)
		{
			PTAirLineInfo pAirline = new TAirLineInfo;
			*pAirline = pInfo->_airInfo;

			strKey.Format(_T("%d-%02d-%02d"),t.GetYear(),t.GetMonth(),t.GetDay());
			itMap = m_mapProcess.find(strKey);
			if (itMap != m_mapProcess.end())
			{
				itMap->second->listAirLine.push_back(pAirline);
			}
			else
			{
				PTAirLineDateProcess pProcess = new TAirLineDateProcess;
				pProcess->tDate = t;
				pProcess->listAirLine.push_back(pAirline);

				pProcess->pDlg = m_pDlgShowFlights;
				m_mapProcess.insert(pair<CString,PTAirLineDateProcess>(strKey,pProcess));
			}

			CTimeSpan tsTmp(1,0,0,0);
			t += tsTmp;
			ts = pInfo->tEnd - t;
		}
	}

	//开始处理数据
	if (m_mapProcess.size() <= 0)
	{
		return 0;
	}
	m_bStart = true;
	CDataProcess::GetInstance()->m_bStarting = true;
	m_nThreadEndCount = 0;

	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}

	//	SetTimer(2014,20000,NULL);
	SetEvent(m_hContinueDoEvt);

	//map<CString,PTAirLineDateProcess>::iterator itMapprocess;
	//PTAirLineDateProcess pProcess = NULL;
	//m_nThreadCount = 0;
	//CDataProcess::GetInstance()->m_bStarting = true;
	//for (itMapprocess = m_mapProcess.begin(); itMapprocess != m_mapProcess.end(); itMapprocess++)
	//{
	//	OutputDebugString(_T("\r\n启动线程函数\r\n"));
	//	pProcess = itMapprocess->second;
	//	AfxBeginThread(ThreadProcess,pProcess);
	//	m_nThreadCount++;
	//}

	return 0;
}

int CGetFlightsDlg::Client1Start(bool bLoop)
{
	OutputDebugString(_T("开始处理//停止"));
	if (!bLoop && m_bStart)
	{
		Stop();
		return 0;
	}

	//单台电脑跑近两三天的数据时，要求速度要快，所以将所有航线分配到map中，map的key不再是日期，仅是一个序号，map的value是航线列表，里面的日期是不同的

	PTAirLineDateInfo pInfo = NULL;
	list<PTAirLineDateInfo>::iterator it;  
	CString strKey;
	map<CString,PTAirLineDateProcess>::iterator itMap;
	list<PTAirLineDateInfo> listAirDate;

	if (!(m_pDlgConfig && m_pDlgConfig->GetSafeHwnd() && m_pDlgConfig->m_listAirDateData.size() > 0))
	{
		AddLog(_T("没有要处理的航线信息"));
		return 0;
	}

	listAirDate = m_pDlgConfig->m_listAirDateData;

	AddLog(_T("\r\n开始处理...\r\n"));

	//先清空 allairlinelist. m_mapProcess 中的list 包含的元素是从alllinelist 中复制过去的，allairlinelist 释放内存后，m_mapProcess中的list不需要再释放
	ClearAllLineList();
	m_client1MapProcess.erase(m_client1MapProcess.begin(), m_client1MapProcess.end());

	it = listAirDate.begin(); 
	pInfo = *it;
	CTimeSpan ts = pInfo->tEnd - pInfo->tStart;
	int iFlightDays = ts.GetDays()+1;
	int iAirlineTotalCount = listAirDate.size() * iFlightDays;
	if (iAirlineTotalCount <= 0)
		return FALSE;
	for (it = listAirDate.begin(); it != listAirDate.end();it++)  
	{  
		pInfo = *it;
		for (int i = 0; i < iFlightDays; i++)
		{
			PAirLineDetailInfo pAirline = new TAirLineDetailInfo;
			pAirline->_airInfo = pInfo->_airInfo;
			CTimeSpan t1DaySpan(i*1, 0, 0, 0);
			pAirline->tStart = pInfo->tStart + t1DaySpan;
			m_allLineList.push_back(pAirline);
		}
	}
	TRACE(_T("\n total airline=%d"), m_allLineList.size());

	std::list<PAirLineDetailInfo>::iterator allLineListIt = m_allLineList.begin();
	int iLinesPerThread = iAirlineTotalCount / ThreadCount;
	for (UINT n = 0; n < ThreadCount-1; n++)
	{
		PTAirLineDetailProcess pProcess = new TAirLineDetailProcess;
		for (int i = 0; i < iLinesPerThread; i++)
		{
			PAirLineDetailInfo pDetailInfo = *allLineListIt;
			pProcess->pDlg = m_pDlgShowFlights;
			pProcess->listAirLine.push_back(pDetailInfo);
			allLineListIt++;
		}
		TRACE(_T("\nthread%d airline count=%d"), n+1, pProcess->listAirLine.size());
		m_client1MapProcess.insert(pair<UINT, PTAirLineDetailProcess>(n, pProcess));
	}
	//剩余的航线全加入最后一个列表
	PTAirLineDetailProcess pProcess = new TAirLineDetailProcess;
	for (; allLineListIt  != m_allLineList.end(); allLineListIt++)
	{
		PAirLineDetailInfo pDetailInfo = *allLineListIt;
		pProcess->pDlg = m_pDlgShowFlights;
		pProcess->listAirLine.push_back(pDetailInfo);
	}
	TRACE(_T("\nthread%d airline count=%d"), ThreadCount-1, pProcess->listAirLine.size());
	m_client1MapProcess.insert(pair<UINT, PTAirLineDetailProcess>(ThreadCount-1, pProcess));
	//

	//开始处理数据
	if (m_client1MapProcess.size() <= 0)
	{
		return 0;
	}
	m_bStart = true;
	CDataProcess::GetInstance()->m_bStarting = true;
	m_nThreadEndCount = 0;

	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}

	SetEvent(m_hContinueDoEvt);

	return 0;
}

int CGetFlightsDlg::KnClientStart(bool bLoop)
{
	OutputDebugString(_T("开始处理//停止"));
	if (!bLoop && m_bStart)
	{
		Stop();
		return 0;
	}

	//单台电脑跑近两三天的数据时，要求速度要快，所以将所有航线分配到map中，map的key不再是日期，仅是一个序号，map的value是航线列表，里面的日期是不同的

	PTAirLineDateInfo pInfo = NULL;
	list<PTAirLineDateInfo>::iterator it;  
	CString strKey;
	map<CString,PTAirLineDateProcess>::iterator itMap;
	list<PTAirLineDateInfo> listAirDate;

	if (!(m_pDlgConfig && m_pDlgConfig->GetSafeHwnd() && m_pDlgConfig->m_listAirDateData.size() > 0))
	{
		AddLog(_T("没有要处理的航线信息"));
		return 0;
	}

	listAirDate = m_pDlgConfig->m_listAirDateData;

	AddLog(_T("\r\n开始处理...\r\n"));

	//先清空 allairlinelist. m_mapProcess 中的list 包含的元素是从alllinelist 中复制过去的，allairlinelist 释放内存后，m_mapProcess中的list不需要再释放
	ClearAllLineList();
	m_client1MapProcess.erase(m_client1MapProcess.begin(), m_client1MapProcess.end());

	it = listAirDate.begin(); 
	pInfo = *it;
	CTimeSpan ts = pInfo->tEnd - pInfo->tStart;
	int iFlightDays = ts.GetDays()+1;
	int iAirlineTotalCount = listAirDate.size() * iFlightDays;
	if (iAirlineTotalCount <= 0)
		return FALSE;
	for (it = listAirDate.begin(); it != listAirDate.end();it++)  
	{  
		pInfo = *it;
		for (int i = 0; i < iFlightDays; i++)
		{
			PAirLineDetailInfo pAirline = new TAirLineDetailInfo;
			pAirline->_airInfo = pInfo->_airInfo;
			CTimeSpan t1DaySpan(i*1, 0, 0, 0);
			pAirline->tStart = pInfo->tStart + t1DaySpan;
			m_allLineList.push_back(pAirline);
		}
	}
	TRACE(_T("\n total airline=%d"), m_allLineList.size());

	std::list<PAirLineDetailInfo>::iterator allLineListIt = m_allLineList.begin();
	int iLinesPerThread = iAirlineTotalCount / ThreadCount;
	for (UINT n = 0; n < ThreadCount-1; n++)
	{
		PTAirLineDetailProcess pProcess = new TAirLineDetailProcess;
		for (int i = 0; i < iLinesPerThread; i++)
		{
			PAirLineDetailInfo pDetailInfo = *allLineListIt;
			pProcess->pDlg = m_pDlgShowFlights;
			pProcess->listAirLine.push_back(pDetailInfo);
			allLineListIt++;
		}
		TRACE(_T("\nthread%d airline count=%d"), n+1, pProcess->listAirLine.size());
		m_client1MapProcess.insert(pair<UINT, PTAirLineDetailProcess>(n, pProcess));
	}
	//剩余的航线全加入最后一个列表
	PTAirLineDetailProcess pProcess = new TAirLineDetailProcess;
	for (; allLineListIt  != m_allLineList.end(); allLineListIt++)
	{
		PAirLineDetailInfo pDetailInfo = *allLineListIt;
		pProcess->pDlg = m_pDlgShowFlights;
		pProcess->listAirLine.push_back(pDetailInfo);
	}
	TRACE(_T("\nthread%d airline count=%d"), ThreadCount-1, pProcess->listAirLine.size());
	m_client1MapProcess.insert(pair<UINT, PTAirLineDetailProcess>(ThreadCount-1, pProcess));
	//

	//开始处理数据
	if (m_client1MapProcess.size() <= 0)
	{
		return 0;
	}
	m_bStart = true;
	CDataProcess::GetInstance()->m_bStarting = true;
	m_nThreadEndCount = 0;

	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}

	SetEvent(m_hContinueDoEvt);

	return 0;
}

void CGetFlightsDlg::ClearAllLineList()
{
	std::list<PAirLineDetailInfo>::iterator it;
	for (it = m_allLineList.begin(); it != m_allLineList.end(); it++)
	{
		PAirLineDetailInfo pInfo = *it;
		delete pInfo;
		pInfo = NULL;
	}
	m_allLineList.clear();
}
void CGetFlightsDlg::AddLog(CString strLog)
{
	//m_logLock.Lock();
	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->AddLog(strLog);
	}
	//m_logLock.Unlock();
}

void CGetFlightsDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	CDialogEx::OnTimer(nIDEvent);
}


void CGetFlightsDlg::Stop(void)
{
	m_bStart = false;
	ResetEvent(m_hContinueDoEvt);
	CDataProcess::GetInstance()->m_bStarting = false;
	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}
}


// 清理map
int CGetFlightsDlg::ClearProcessMap(map<CString,PTAirLineDateProcess>& mapProcess)
{
	map<CString,PTAirLineDateProcess>::iterator itMap;
	list<PTAirLineInfo>::iterator itList;
	int nIndex = 2;
	for (itMap = mapProcess.begin(); itMap != mapProcess.end(); itMap++)
	{
		for (itList = itMap->second->listAirLine.begin(); itList != itMap->second->listAirLine.end(); itList++)
		{
			delete *itList;
		}
		itMap->second->listAirLine.erase(itMap->second->listAirLine.begin(),itMap->second->listAirLine.end());

		delete itMap->second;

	}
	mapProcess.erase(mapProcess.begin(),mapProcess.end());

	return 0;
}

//线程处理函数
void CGetFlightsDlg::ThreadProcess( LPVOID pParam )
{
	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}

	PTAirLineDateProcess pProcess = (PTAirLineDateProcess)pParam;
	PTAirLineInfo	pAirline = NULL;

	UINT	uFlightCount = 1;
	list<PTAirLineInfo>::iterator itList;
	BOOL bChangeProxyIp = FALSE;
	BOOL bIgnoreP4Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP4Ticket;
	BOOL bIgnoreP5Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP5Ticket;
	BOOL bSelSpecialPriceTicket = CDataProcess::GetInstance()->m_configInfo.bSelSpecialPriceTicket;
	int nP4SpecialPrice = CDataProcess::GetInstance()->m_configInfo.nP4SpecialPrice;
	CTime tCur = CTime::GetCurrentTime();
	int iCurYear = tCur.GetYear();
	int iCurMonth = tCur.GetMonth();
	int iiCurDate = tCur.GetDay();
	CTime tSel9Start(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Minute, 0);
	CTime tSel9End(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Minute, 0);
	int nTryMaxTime = 3;
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	//循环每一个航线
	for (itList = pProcess->listAirLine.begin(); itList != pProcess->listAirLine.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pAirline = *itList;
		if (NULL == pAirline)
		{
			continue;
		}
		//获取携程的航班信息
		CStringA straCtripPostData;

		CStringA straDate;
		straDate.Format("%d-%02d-%02d",pProcess->tDate.GetYear(),pProcess->tDate.GetMonth(),pProcess->tDate.GetDay());
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;
		//change
		list<PT9CFlightInfo> listFlight;
		C9CHtmlParse parse9c;
		listFlight.clear();
		//end
		for (int i = 0; i < nTryMaxTime; i++)
		{
#ifdef GET_FLIGHT_FROM_NEW_SITE
			BOOL bRet = __GetSssFlightFromNewSite(bChangeProxyIp, pAirline->strFromCityCode, pAirline->strToCityCode, straDate,
				httpResponseContent);
#else
			BOOL bRet = __GetSssFlight(bChangeProxyIp, pAirline->strFromCityCode, pAirline->strToCityCode, straDate,
				straDate, httpResponseContent);
#endif

			if (bRet)
			{
				parse9c.ParseNewSiteJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirline->uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
				if (__IsJsonFlightDataOk(listFlight))
				{
					bGet = TRUE;
					break;
				}else
				{
					bChangeProxyIp = TRUE;
					DWORD dwSpan = 10;
					Sleep(dwSpan*(i+1));
					if(i == (nTryMaxTime-1))
					{
						CString strErr;
						strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息5次都错误!"),
							CStrA2CStrW(straDate), pAirline->strFromCityCode, pAirline->strToCityCode);
						OutputDebugString(strErr);
					}
				}
			}
			else
			{
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息5次都错误!"),
						CStrA2CStrW(straDate), pAirline->strFromCityCode, pAirline->strToCityCode);
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		{
			bChangeProxyIp = FALSE;
		}
//		if (!bGet)
//			continue;

		//从网页中解析得到的航班信息
		/*
		list<PT9CFlightInfo> listFlight;
		C9CHtmlParse parse9c;
		listFlight.clear();

		parse9c.ParseNewSiteJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirline->uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
		if (!__IsJsonFlightDataOk(listFlight))
			bChangeProxyIp = TRUE;
		*/

		//CDbCenter::GetInstance()->DeleteFlights("9C",CStrT2CStrA(pAirline->strFromCityCode),
		//	CStrT2CStrA(pAirline->strToCityCode),straDate);
		//CStringA strLogTmp;
		//strLogTmp.Format("delete %s|%s|%s", CStrT2CStrA(pAirline->strFromCityCode), CStrT2CStrA(pAirline->strToCityCode), straDate);
		//OutputDebugStringA(strLogTmp);

		list<PT9CFlightInfo>::iterator it;
		list<PT9CFlightInfo>::iterator itDblist;
		PT9CFlightInfo pFlight = NULL;
		PT9CFlightInfo pDbFlight = NULL;

		//处理查询过程中票已售完或当前这轮没抓到票的数据，票价清0
		list<PT9CFlightInfo> lastFlightList;
		lastFlightList.clear();
		CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirline->strFromCityCode));
		CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirline->strToCityCode));
		CDbCenter::GetInstance()->QueryFlights(lastFlightList, "9C", stra9CFromCityCode, stra9CToCityCode, straDate);
		m_pThis->__Reset9CFlightInfoList(lastFlightList);
		m_pThis->__Merge9CFlightInfoList(listFlight, lastFlightList);
		//

		//插入数据库
		for (it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			//pFlight->nMinHangPrice;
			pFlight = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			//strLogTmp1.Format("\r\nInsert flightNo=%s,%s:%s->%s, minHan=%d, p1=%d, p2=%d, p3=%d, min=%d"
			//	, pFlight->straFlightNo, pFlight->straFromDate,pFlight->straFromCityCode, pFlight->straToCityCode
			//	, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3, pFlight->nMinPrice);
			//OutputDebugStringA(strLogTmp1);
			CDbCenter::GetInstance()->InsertFligthInfo(pFlight);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d|%d", pFlight->straFromCityCode, pFlight->straToCityCode, pFlight->straFromDate, pFlight->straFlightNo
				, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}

		m_pThis->__Free9CFlightInfoList(lastFlightList);
		m_pThis->__Free9CFlightInfoList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}

//近两天的所有航线在一台电脑上跑的线程处理函数
void CGetFlightsDlg::ThreadProcessClient1( LPVOID pParam )
{
	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}

	PTAirLineDetailProcess pProcess = (PTAirLineDetailProcess)pParam;
	PAirLineDetailInfo	pAirlineDetail = NULL;
	UINT	uFlightCount = 1;
	BOOL bChangeProxyIp = FALSE;
	BOOL bIgnoreP4Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP4Ticket;
	BOOL bIgnoreP5Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP5Ticket;
	BOOL bSelSpecialPriceTicket = CDataProcess::GetInstance()->m_configInfo.bSelSpecialPriceTicket;
	int nP4SpecialPrice = CDataProcess::GetInstance()->m_configInfo.nP4SpecialPrice;
	CTime tCur = CTime::GetCurrentTime();
	int iCurYear = tCur.GetYear();
	int iCurMonth = tCur.GetMonth();
	int iiCurDate = tCur.GetDay();
	CTime tSel9Start(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Minute, 0);
	CTime tSel9End(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Minute, 0);
	int nTryMaxTime = 3;
	TRACE(_T("\n thread process line count=%d"), pProcess->listAirLine.size());
	//循环每一个航线
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	list<PAirLineDetailInfo>::iterator itList;
	for (itList = pProcess->listAirLine.begin(); itList != pProcess->listAirLine.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pAirlineDetail = *itList;
		if (NULL == pAirlineDetail)
		{
			continue;
		}
		//获取携程的航班信息
		CStringA straCtripPostData;
		CStringA straDate;
		straDate.Format("%d-%02d-%02d", pAirlineDetail->tStart.GetYear(), pAirlineDetail->tStart.GetMonth(), pAirlineDetail->tStart.GetDay());
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;

		for (int i = 0; i < nTryMaxTime; i++)
		{
#ifdef GET_FLIGHT_FROM_NEW_SITE
			BOOL bRet = __GetSssFlightFromNewSite(bChangeProxyIp, pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode, straDate,
				httpResponseContent);
#else
			BOOL bRet = __GetSssFlight(bChangeProxyIp, pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode, straDate, straDate
				, httpResponseContent);
#endif

			
			if (bRet)
			{
				
					bGet = TRUE;
					break;
			}
			else
			{
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息5次都错误!"),
						CStrA2CStrW(straDate), pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode);
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		{
			bChangeProxyIp = FALSE;
		}
		if (!bGet)
			continue;

		//从网页中解析得到的航班信息
		list<PT9CFlightInfo> listFlight;
		C9CHtmlParse parse9c;
		listFlight.clear();

		parse9c.ParseNewSiteJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirlineDetail->_airInfo.uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
		if (!__IsJsonFlightDataOk(listFlight))
			bChangeProxyIp = TRUE;

		list<PT9CFlightInfo>::iterator it;
		list<PT9CFlightInfo>::iterator itDblist;
		PT9CFlightInfo pFlight = NULL;
		PT9CFlightInfo pDbFlight = NULL;

		//处理查询过程中票已售完或当前这轮没抓到票的数据，票价清0
		list<PT9CFlightInfo> lastFlightList;
		lastFlightList.clear();
		CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		CDbCenter::GetInstance()->QueryFlights(lastFlightList, "9C", stra9CFromCityCode, stra9CToCityCode, straDate);
		m_pThis->__Reset9CFlightInfoList(lastFlightList);
		m_pThis->__Merge9CFlightInfoList(listFlight, lastFlightList);
		//

		//插入数据库
		for (it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			//pFlight->nMinHangPrice;
			pFlight = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			//strLogTmp1.Format("\r\nInsert flightNo=%s,%s:%s->%s, minHan=%d, p1=%d, p2=%d, p3=%d, min=%d"
			//	, pFlight->straFlightNo, pFlight->straFromDate,pFlight->straFromCityCode, pFlight->straToCityCode
			//	, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3, pFlight->nMinPrice);
			//OutputDebugStringA(strLogTmp1);
			CDbCenter::GetInstance()->InsertFligthInfo(pFlight);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d|%d", pFlight->straFromCityCode, pFlight->straToCityCode, pFlight->straFromDate, pFlight->straFlightNo
				, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}

		m_pThis->__Free9CFlightInfoList(lastFlightList);
		m_pThis->__Free9CFlightInfoList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}


void CGetFlightsDlg::ThreadProcessKn( LPVOID pParam )
{
	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}

	PTAirLineDetailProcess pProcess = (PTAirLineDetailProcess)pParam;
	PAirLineDetailInfo	pAirlineDetail = NULL;
	UINT	uFlightCount = 1;
	BOOL bChangeProxyIp = FALSE;
	BOOL bIgnoreP4Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP4Ticket;
	BOOL bIgnoreP5Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP5Ticket;
	BOOL bSelSpecialPriceTicket = CDataProcess::GetInstance()->m_configInfo.bSelSpecialPriceTicket;
	int nP4SpecialPrice = CDataProcess::GetInstance()->m_configInfo.nP4SpecialPrice;
	CTime tCur = CTime::GetCurrentTime();
	int iCurYear = tCur.GetYear();
	int iCurMonth = tCur.GetMonth();
	int iiCurDate = tCur.GetDay();
	CTime tSel9Start(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Minute, 0);
	CTime tSel9End(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Minute, 0);
	int nTryMaxTime = 3;

	TRACE(_T("\n thread process line count=%d"), pProcess->listAirLine.size());
	//循环每一个航线
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	list<PAirLineDetailInfo>::iterator itList;
	for (itList = pProcess->listAirLine.begin(); itList != pProcess->listAirLine.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pAirlineDetail = *itList;
		if (NULL == pAirlineDetail)
		{
			continue;
		}
		//获取携程的航班信息
		CStringA straCtripPostData;
		CStringA straDate;
		straDate.Format("%d-%02d-%02d", pAirlineDetail->tStart.GetYear(), pAirlineDetail->tStart.GetMonth(), pAirlineDetail->tStart.GetDay());
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;
		////
		list<PT9CFlightInfo> listFlight;
		CKnHtmlParse parsekn;
		listFlight.clear();

		//		CTime tt = CTime::GetCurrentTime();
		//		if(pAirlineDetail->tStart <= (tt+CTimeSpan(3,0,0,0))&& tt>tSel9End )
		//			goto __nineteen;

		for (int i = 0; i < nTryMaxTime; i++)
		{
			BOOL bRet = __GetKnFlight(bChangeProxyIp, pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode, straDate,
				httpResponseContent);
			////////////

			if(bRet)
			{
				UINT nCount = parsekn.ParseKnJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirlineDetail->_airInfo.uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
				if (!__IsJsonFlightDataOk(listFlight)&& nCount == 0 )
				{
					bChangeProxyIp = TRUE;
					DWORD dwSpan = 10;
					Sleep(dwSpan*(i+1));
					if(i == (nTryMaxTime-1))
					{
						CString strErr;
						strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息3次都错误!"),
							CStrA2CStrW(straDate), pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode);
						OutputDebugString(strErr);
					}
				}
				else
				{
					bGet = TRUE;
					break;
				}
			}else
			{
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息3次都错误!"),
						CStrA2CStrW(straDate), pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode);
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
		//		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		//		{
		//			bChangeProxyIp = FALSE;
		//		}
		//		if (!bGet)
		//			continue;

		//从网页中解析得到的航班信息
		//		list<PT9CFlightInfo> listFlight;
		//		CKnHtmlParse parsekn;
		//		listFlight.clear();

		//		parsekn.ParseKnJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirlineDetail->_airInfo.uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
		//		if (!__IsJsonFlightDataOk(listFlight))
		//			bChangeProxyIp = TRUE;
		//__nineteen:
		list<PT9CFlightInfo>::iterator it;
		list<PT9CFlightInfo>::iterator itDblist;
		PT9CFlightInfo pFlight = NULL;
		PT9CFlightInfo pDbFlight = NULL;

		//处理查询过程中票已售完或当前这轮没抓到票的数据，票价清0
		list<PT9CFlightInfo> lastFlightList;
		lastFlightList.clear();
		CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		CDbCenter::GetInstance()->QueryKnFlights(lastFlightList, "KN", stra9CFromCityCode, stra9CToCityCode, straDate);
		m_pThis->__Reset9CFlightInfoList(lastFlightList);
		m_pThis->__Merge9CFlightInfoList(listFlight, lastFlightList);
		//

		//插入数据库
		for (it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			//pFlight->nMinHangPrice;
			pFlight = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			//strLogTmp1.Format("\r\nInsert flightNo=%s,%s:%s->%s, minHan=%d, p1=%d, p2=%d, p3=%d, min=%d"
			//	, pFlight->straFlightNo, pFlight->straFromDate,pFlight->straFromCityCode, pFlight->straToCityCode
			//	, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3, pFlight->nMinPrice);
			//OutputDebugStringA(strLogTmp1);
			CDbCenter::GetInstance()->InsertKnFligthInfo(pFlight);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d|%d", pFlight->straFromCityCode, pFlight->straToCityCode, pFlight->straFromDate, pFlight->straFlightNo
				, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}

		m_pThis->__Free9CFlightInfoList(lastFlightList);
		m_pThis->__Free9CFlightInfoList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}

BOOL CGetFlightsDlg::__GetSssFlight(BOOL bChangeProxy, CString strFromCityCode,CString strToCityCode, 
	CStringA straStartDate, CStringA straEndDate, wstring& httpResponseContent)
{
	BOOL bOk = TRUE;
	CStringA straPostData;
	straPostData.Format("OriCityCode=%s&DestCityCode=%s&FlightDate=%s&MoneyType=0&IsReturn=false&FlightDateReturn=%s",
		CStrT2CStrA(strFromCityCode),CStrT2CStrA(strToCityCode), straStartDate, straEndDate);

	WinHttpClient httpClient(L"http://www.china-sss.com/AirFlights/SearchByTime");
	httpClient.SetTimeouts(0, 25000U, 25000U, 40000U);
	CDataProcess::GetInstance()->ChangeProxy(httpClient);
	httpClient.SetAdditionalDataToSend((BYTE *)straPostData.GetBuffer(0),straPostData.GetLength());

	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", straPostData.GetLength());
	wstring headers = L"Content-Length: ";
	headers += szSize;
	headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");

	if (0 != httpClient.GetResponseStatusCode().compare(L"200"))
	{
		//CString str;
		//str.Format(_T("-----error code=%s\n"), httpClient.GetResponseStatusCode().c_str());
		//OutputDebugString(str);
		bOk = FALSE;
	}

	if (bOk)
	{
		wstring httpResponseHeader = httpClient.GetResponseHeader();
		httpResponseContent = httpClient.GetResponseContent();
	}

	return bOk;
}

std::string CGetFlightsDlg::__GetCityNameUrlCode(CString & strCityCode)
{
	CString strCityName("");
	if (0 == strCityCode.CompareNoCase(_T("SWA")))
		strCityName	= _T("揭阳(汕头)");
	else if(0 == strCityCode.CompareNoCase(_T("JJN")))
		strCityName	= _T("泉州");
	else
		strCityName	= CDataProcess::GetInstance()->GetCityNameByCode(strCityCode);
	CStringA straCityName = CStrW2CStrA(strCityName);
	std::string stdStrrCityName(straCityName);
	strconv_t strConv;
	stdStrrCityName = strConv.a2utf8(stdStrrCityName.c_str());
	std::string  stdStrCityNameUrl = UrlEncode(stdStrrCityName);

	return stdStrCityNameUrl;
}


BOOL CGetFlightsDlg::__GetKnFlight(BOOL bChangeProxy, CString & strFromCityCode,CString & strToCityCode, 
	CStringA & straStartDate, wstring& httpResponseContent)
{
	BOOL bOk = TRUE;
	CStringA straPostData;

	std::string stdStrDCityUrl = __GetCityNameUrlCode(strFromCityCode);
	std::string stdStrACityUrl = __GetCityNameUrlCode(strToCityCode);

	CStringA straFromCityCode = CStrW2CStrA(strFromCityCode);
	CStringA straToCityCode = CStrW2CStrA(strToCityCode);

	CStringA straFromCityName = CityCode2CityNameCode(CStrW2CStrA(strFromCityCode));
	CStringA straToCityName = CityCode2CityNameCode(CStrW2CStrA(strToCityCode));

//	straPostData.Format("SType=01&IfRet=true&OriCity=%s&OriCode=%s&DestCity=%s&DestCode=%s&MType=0&FDate=%s&RetFDate=%s&ANum=1&CNum=0&INum=0", stdStrDCityUrl.c_str(), straFromCityCode
//		, stdStrACityUrl.c_str(), straToCityCode, straStartDate, straStartDate);
	straPostData.Format("searchCond={\"tripType\":\"OW\",\"adtCount\":1,\"chdCount\":0,\"infCount\":0,\"currency\":\"CNY\",\"sortType\":\"a\",\"segmentList\":[{\"deptCd\":\"%s\",\"arrCd\":\"%s\",\"deptDt\":\"%s\",\"deptCityCode\":\"%s\",\"arrCityCode\":\"%s\"}],\"sortExec\":\"a\",\"page\":\"0\"}", 
		straFromCityCode, straToCityCode, straStartDate, straFromCityName , straToCityName);


	WinHttpClient httpClient(L"http://www.flycua.com/otabooking/flight-search!doFlightSearch.shtml?rand=0.3512624071445316");//(L"http://search.ch.com/default/SearchByTime");
	httpClient.SetTimeouts(0, 25000U, 25000U, 40000U);
	CDataProcess::GetInstance()->ChangeProxy(httpClient);
	httpClient.SetAdditionalDataToSend((BYTE *)straPostData.GetBuffer(0),straPostData.GetLength());

	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", straPostData.GetLength());
	wstring headers = L"Content-Length: ";
	headers += szSize;
	headers += L"\r\nHost:www.flycua.com";
	headers += L"\r\nConnection:keep-alive";
	headers += L"\r\nAccept:application/json,text/javascript,*/*;q=0.01";
	headers += L"\r\nOrigin:http://www.flycua.com";
	headers += L"\r\nX-Requested-With:XMLHttpRequest";
	headers += L"\r\nUser-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.118 Safari/537.36";
	headers += L"\r\nContent-Type:application/x-www-form-urlencoded; charset=UTF-8\r\n";
	headers += L"\r\nAccept-Encoding:gzip, deflate";
	headers += L"\r\nAccept-Language:zh-CN,zh;q=0.8";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");

	if (0 != httpClient.GetResponseStatusCode().compare(L"200"))
	{
		//CString str;
		//str.Format(_T("-----error code=%s\n"), httpClient.GetResponseStatusCode().c_str());
		//OutputDebugString(str);
		bOk = FALSE;
	}

	if (bOk)
	{
		wstring httpResponseHeader = httpClient.GetResponseHeader();
		//接收utf-8，然后转换为unicode
		const BYTE* pUtf8 = httpClient.GetRawResponseContent();
		//////////////
		//end
//		
		DWORD dwNum = MultiByteToWideChar (CP_UTF8, 0, (LPCSTR)pUtf8, -1, NULL, 0);
  

		wchar_t *pwText;
		pwText = new wchar_t[dwNum];
		 if(!pwText)
		  {
		   delete []pwText;
		  }
  
		MultiByteToWideChar (CP_UTF8, 0, (LPCSTR)pUtf8, -1, pwText, dwNum);
  
		//
		httpResponseContent = pwText;
		delete [] pwText;

	}

	return bOk;
}

BOOL CGetFlightsDlg::__GetSssFlightFromNewSite(BOOL bChangeProxy, CString & strFromCityCode,CString & strToCityCode, 
	CStringA & straStartDate, wstring& httpResponseContent)
{
	BOOL bOk = TRUE;
	CStringA straPostData;

	std::string stdStrDCityUrl = __GetCityNameUrlCode(strFromCityCode);
	std::string stdStrACityUrl = __GetCityNameUrlCode(strToCityCode);

	CStringA straFromCityCode = CStrW2CStrA(strFromCityCode);
	CStringA straToCityCode = CStrW2CStrA(strToCityCode);

	straPostData.Format("SType=01&IfRet=true&OriCity=%s&OriCode=%s&DestCity=%s&DestCode=%s&MType=0&FDate=%s&RetFDate=%s&ANum=1&CNum=0&INum=0", stdStrDCityUrl.c_str(), straFromCityCode
		, stdStrACityUrl.c_str(), straToCityCode, straStartDate, straStartDate);

	WinHttpClient httpClient(L"http://search.ch.com/default/SearchByTime");
	httpClient.SetTimeouts(0, 25000U, 25000U, 40000U);
	CDataProcess::GetInstance()->ChangeProxy(httpClient);
	httpClient.SetAdditionalDataToSend((BYTE *)straPostData.GetBuffer(0),straPostData.GetLength());

	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", straPostData.GetLength());
	wstring headers = L"Content-Length: ";
	headers += szSize;
	headers += L"\r\nHost:search.ch.com";
	headers += L"\r\nOrigin:http://search.ch.com";
	headers += L"\r\nContent-Type:application/x-www-form-urlencoded; charset=UTF-8\r\n";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");

	if (0 != httpClient.GetResponseStatusCode().compare(L"200"))
	{
		//CString str;
		//str.Format(_T("-----error code=%s\n"), httpClient.GetResponseStatusCode().c_str());
		//OutputDebugString(str);
		bOk = FALSE;
	}

	if (bOk)
	{
		wstring httpResponseHeader = httpClient.GetResponseHeader();
		httpResponseContent = httpClient.GetResponseContent();
	}

	return bOk;
}

BOOL CGetFlightsDlg::__GetCaLowPriceFlight(BOOL bChangeProxy, int iCityCode, const CString & strCity, wstring& httpResponseContent)
{
	BOOL bOk = TRUE;
	CStringA straPostData;
	CString strUrl;
	strUrl.Format(_T("http://tuan.airchina.com/team-1000%d-%s.html"), iCityCode, strCity);
	WinHttpClient httpClient(strUrl.GetBuffer(0));//(L"http://tuan.airchina.com/team-10006241-beijing.html");
	httpClient.SetTimeouts(0, 25000U, 25000U, 40000U);
//	if (bChangeProxy)
		CDataProcess::GetInstance()->ChangeProxy(httpClient);

	wstring headers;
	headers += L"tuan.airchina.com";
	headers += L"\r\nContent-Type:application/x-www-form-urlencoded; charset=UTF-8\r\n";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");

	if (0 != httpClient.GetResponseStatusCode().compare(L"200"))
	{
		//CString str;
		//str.Format(_T("-----error code=%s\n"), httpClient.GetResponseStatusCode().c_str());
		//OutputDebugString(str);
		bOk = FALSE;
	}

	if (bOk)
	{
		wstring httpResponseHeader = httpClient.GetResponseHeader();
		httpResponseContent = httpClient.GetResponseContent();
	}

	return bOk;
}

BOOL CGetFlightsDlg::__IsJsonFlightDataOk(list<PT9CFlightInfo>& listFlight)
{
	BOOL bOk = FALSE;
	list<PT9CFlightInfo>::iterator it;
	PT9CFlightInfo pFlight = NULL;
	if (0 == listFlight.size())
		goto CGetFlightsDlg_IsJsonFlightDataOk_Ret;
#ifndef GET_KN_FLIGHT
	for (it = listFlight.begin(); it != listFlight.end(); it++)
	{
		pFlight = *it;

		//ip 可能有问题，下个航线立即换ip
		if ((0 == pFlight->nPrice1) && (0 == pFlight->nPrice2) && (0 == pFlight->nPrice3))
			goto CGetFlightsDlg_IsJsonFlightDataOk_Ret;
	}
#endif

	bOk = TRUE;
CGetFlightsDlg_IsJsonFlightDataOk_Ret:
	return bOk;
}
void CGetFlightsDlg::__Free9CFlightInfoList(list<PT9CFlightInfo> &flightList)
{
	list<PT9CFlightInfo>::iterator it;
	PT9CFlightInfo pFlight = NULL;

	if (flightList.empty())
		return;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		delete pFlight;
		pFlight = NULL;
	}
	flightList.clear();
}
//消息响应函数，处理线程已经结束
LRESULT CGetFlightsDlg::OnProcessThreadEnd(WPARAM wParam, LPARAM lParam)
{
	m_nThreadEndCount++;
	if (m_nThreadEndCount >= __GetCurThreadCount())
	{
		//所有处理线程已结束，一轮完成
		if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
		{
			AddLog(_T("一轮数据处理已完成"));
			m_pDlgConfig->AddProcessTime();
			m_nThreadEndCount = 0;
			//if (m_bStart && CDataProcess::GetInstance()->m_bStarting)
			//{
			//	Start(true);
			//}
			//else
			//{
			//	Start(false);
			//}
		}

		if (m_bCloseClicked)
		{
			CDialogEx::OnClose();
		}
	}


	return 0;
}

LRESULT CGetFlightsDlg::OnUpdateFlight(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	USES_CONVERSION;
	char* pLog = (char*)wParam;
	strLog.Format(_T("%s"), A2W(pLog));

	AddLog(strLog);
	delete pLog;

	return 0;
}

void CGetFlightsDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}

void CGetFlightsDlg::__WailAllJobFinished(bool bWaitAll)
{
	if (!bWaitAll)
		return;
	while(1)
	{
		DWORD dwRunningCount = g_threadPoolProcess.GetRunningSize();
		if (0 == dwRunningCount)
		{
			break;
		}
		else
			Sleep(20);
	}
}

UINT CGetFlightsDlg::__ThreadInsertJobToThreadPool( void* pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	map<CString,PTAirLineDateProcess>::iterator itMapprocess;
	map<CString,PTAirLineDateProcess>::iterator itTask2Start;//任务2的起点，默认为第8天
	map<CString,PTAirLineDateProcess>::iterator itTask3Start;//任务3的起点，默认为第15天
	map<CString,PTAirLineDateProcess>::iterator itEnd;
	PTAirLineDateProcess pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);
	int		nPollNoChangeFlightTime = 0;
	int		nPollNoChangeFlightMaxTime = 2;
	int		nPollLessChangeFlightTime = 0;
	int		nPollLessChangeFlightMaxTime = 2;
	BOOL bDoNoChangeFlightPoll = FALSE;
	BOOL bDoLessChangeFlightPoll = FALSE;

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto CGetFlightsDlg_ThreadInsertJobToThreadPool_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();

		CString strKey;
		CTime t = CDataProcess::GetQueryStartTime();
		CTimeSpan tSpan(7,0,0,0);
		CTime t2 = t + tSpan;
		strKey.Format(_T("%d-%02d-%02d"),t2.GetYear(),t2.GetMonth(),t2.GetDay());
		itTask2Start = pThis->m_mapProcess.find(strKey);

		CTimeSpan tSpan3(14,0,0,0);
		CTime t3 = t + tSpan;
		strKey.Format(_T("%d-%02d-%02d"),t3.GetYear(),t3.GetMonth(),t3.GetDay());
		itTask3Start = pThis->m_mapProcess.find(strKey);

		if (CDataProcess::GetInstance()->m_configInfo.bAutoModDate)//自动修改日期
		{
			bDoNoChangeFlightPoll = CDataProcess::GetInstance()->NeedNoChangeFlightPoll();
			bDoLessChangeFlightPoll = CDataProcess::GetInstance()->NeedLessChangeFlightPoll();
			if (bDoNoChangeFlightPoll)
			{
				//轮询几乎不变航线时已经轮询了很少变的航线
				if (bDoLessChangeFlightPoll)
				{
					CDataProcess::GetInstance()->SaveLessChangeFlightPollTime();
					bDoLessChangeFlightPoll = FALSE;
				}

				if (nPollNoChangeFlightTime > nPollNoChangeFlightMaxTime)
				{
					while(1)
					{
						DWORD dwRunningCount = g_threadPoolProcess.GetRunningSize();
						if (0 == dwRunningCount)
						{
							//::PostMessage(pThis->m_hWnd, WM_QUERY_LESS_CHANGE_FLIGHT_OK, 0, 0);
							nPollNoChangeFlightTime = 0;
							CDataProcess::GetInstance()->SaveNoChangeFlightPollTime();

							break;
						}
						else
							Sleep(30);
						TRACE(_T("\n--------threadpool running count:%d\n"), dwRunningCount);
					}
				}
			}

			if (bDoLessChangeFlightPoll)
			{
				if (nPollLessChangeFlightTime > nPollLessChangeFlightMaxTime)
				{
					while(1)
					{
						DWORD dwRunningCount = g_threadPoolProcess.GetRunningSize();
						if (0 == dwRunningCount)
						{
							//::PostMessage(pThis->m_hWnd, WM_QUERY_LESS_CHANGE_FLIGHT_OK, 0, 0);
							nPollLessChangeFlightTime = 0;
							CDataProcess::GetInstance()->SaveLessChangeFlightPollTime();

							break;
						}
						else
							Sleep(30);
						TRACE(_T("\n--------threadpool running count:%d\n"), dwRunningCount);
					}
				}
			}

			//判断轮询终点
			if (bDoNoChangeFlightPoll)
			{
				itEnd = pThis->m_mapProcess.end();
			}
			else if (nPollLessChangeFlightTime)
			{
				itEnd = itTask3Start;
			}
			else
			{
				itEnd = itTask2Start;
			}
			//end 判断轮询终点
		}
		else //不自动修改日期
		{
			itEnd = pThis->m_mapProcess.end();
		}

		UINT uThreadCount = 0;
		for (itMapprocess = pThis->m_mapProcess.begin(); itMapprocess != itEnd; itMapprocess++)
		{
			uThreadCount++;
		}
		pThis->__SetCurThreadCount(uThreadCount);

		for (itMapprocess = pThis->m_mapProcess.begin(); itMapprocess != itEnd; itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(ThreadProcess,(LPVOID)pProcess);
		}

		if (CDataProcess::GetInstance()->m_configInfo.bAutoModDate)//自动修改日期
		{
			if (bDoNoChangeFlightPoll)
			{
				nPollNoChangeFlightTime++;
			}

			if (bDoLessChangeFlightPoll)
			{
				nPollLessChangeFlightTime++;
			}
		}

		Sleep(dwQuerySpan);
	}

CGetFlightsDlg_ThreadInsertJobToThreadPool_Ret:

	return 0;
}

UINT CGetFlightsDlg::__ThreadInsertJobToThreadPoolClient1( void* pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	PTAirLineDetailProcess pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto ThreadInsertJobToThreadPoolEx_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();

		map<UINT, PTAirLineDetailProcess>::iterator itMapprocess;
		pThis->__SetCurThreadCount(ThreadCount);

		for (itMapprocess = pThis->m_client1MapProcess.begin(); itMapprocess != pThis->m_client1MapProcess.end(); itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(ThreadProcessClient1,(LPVOID)pProcess);
		}

		Sleep(dwQuerySpan);
	}

ThreadInsertJobToThreadPoolEx_Ret:

	return 0;
}

UINT CGetFlightsDlg::__ThreadInsertKnJobToThreadPool( void* pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	PTAirLineDetailProcess pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto ThreadInsertJobToThreadPoolEx_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();

		map<UINT, PTAirLineDetailProcess>::iterator itMapprocess;
		pThis->__SetCurThreadCount(ThreadCount);

		for (itMapprocess = pThis->m_client1MapProcess.begin(); itMapprocess != pThis->m_client1MapProcess.end(); itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(ThreadProcessKn,(LPVOID)pProcess);
		}

		Sleep(dwQuerySpan);
	}

ThreadInsertJobToThreadPoolEx_Ret:

	return 0;
}

LRESULT CGetFlightsDlg::OnQueryLessChangeFlightOk(WPARAM wParam, LPARAM lParam)
{
	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnabeleQueryLessChangeFlightBtn(TRUE);
	}
	return 0L;
}

void CGetFlightsDlg::__Reset9CFlightInfoList(list<PT9CFlightInfo> &flightList)
{
	list<PT9CFlightInfo>::iterator it;
	PT9CFlightInfo pFlight = NULL;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		pFlight->nMinHangPrice = 0;
		pFlight->nPrice1 = 0;
		pFlight->nPrice2 = 0;
		pFlight->nPrice3 = 0;
		pFlight->nMinPrice = 0;
		pFlight->straPrice1Json = "";
		pFlight->straPrice2Json = "";
		pFlight->straPrice3Json = "";
	}
}

//在航班list中查找航班
PT9CFlightInfo CGetFlightsDlg::__Find9CFlight(list<PT9CFlightInfo> &flightList, PT9CFlightInfo pKey, BOOL *pbFind)
{
	list<PT9CFlightInfo>::iterator it;
	PT9CFlightInfo pFlight = NULL;	
	PT9CFlightInfo pFindResult = NULL;

	*pbFind = FALSE;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		if ( (0 == pKey->straFlightNo.CompareNoCase(pFlight->straFlightNo)) 
			&& (0 == pKey->straFromCityCode.CompareNoCase(pFlight->straFromCityCode))
			&& (0 == pKey->straToCityCode.CompareNoCase(pFlight->straToCityCode)) 
			&& (0 == pKey->straFromDate.CompareNoCase(pFlight->straFromDate))
			&& (0 == pKey->straToDate.CompareNoCase(pFlight->straToDate))
			&& (0 == pKey->straCompany.CompareNoCase(pFlight->straCompany))
			)
		{
			pFindResult = pFlight;
			*pbFind = TRUE;
			break;
		}
	}
	return pFindResult;
}

void CGetFlightsDlg::__Merge9CFlightInfoList(list<PT9CFlightInfo> & fromFlightList, list<PT9CFlightInfo> & toFlightList)
{
	list<PT9CFlightInfo>::iterator it;
	PT9CFlightInfo pFlight = NULL;

	for (it = fromFlightList.begin(); it != fromFlightList.end(); it++)
	{
		pFlight = *it;
		
		BOOL bFind = FALSE;
		PT9CFlightInfo pFindRes = __Find9CFlight(toFlightList, pFlight, &bFind);
		if (bFind)
		{
			//modify						
			pFindRes->straFromTime = pFlight->straFromTime;		
			pFindRes->straToTime = pFlight->straToTime;				
			pFindRes->nMinHangPrice = pFlight->nMinHangPrice;			
			pFindRes->nPrice1 = pFlight->nPrice1;			
			pFindRes->nPrice2 = pFlight->nPrice2;			
			pFindRes->nPrice3 = pFlight->nPrice3;			
			pFindRes->nMinPrice = pFlight->nMinPrice;				
			pFindRes->bHavePolicy = pFlight->bHavePolicy;		
			pFindRes->straPrice1Json = pFlight->straPrice1Json;
			pFindRes->straPrice2Json = pFlight->straPrice2Json;
			pFindRes->straPrice3Json = pFlight->straPrice3Json;
		}
		else
		{
			//append
			PT9CFlightInfo pTmpFlight = new T9CFlightInfo;
			*pTmpFlight = *pFlight;
			toFlightList.push_back(pTmpFlight);
		}
	}

}
void CGetFlightsDlg::__LoadCaLowPriceAirLineList()
{
	CDataProcess*pDataProcess = CDataProcess::GetInstance();
	const char* pChar = NULL;
	TiXmlElement* pElement = NULL;
	CString strXmlFile = pDataProcess->GetExePath() + _T("\\caLowPriceAirLineList_Get.xml");
	TiXmlDocument tiDoc(pDataProcess->Unicode2Ansi(strXmlFile.GetBuffer(0)).c_str());
	tiDoc.LoadFile();
	if ( tiDoc.Error() )
	{
		TRACE( "CConfig::LoadCtripOrder Error in %s: %s\n", tiDoc.Value(), tiDoc.ErrorDesc() );
		return;
	}

	TiXmlNode* pNode = tiDoc.FirstChild( "FlightList" );
	assert( NULL != pNode );
	TiXmlElement* pParentElement = pNode->ToElement();
	assert( NULL != pParentElement  );

	TiXmlElement* pRoot = tiDoc.RootElement();
	TiXmlNode*  pRootNode = pRoot;
	TiXmlHandle rootHandle(pRootNode);
	const char* pBuf = NULL;

	pElement = rootHandle.Child("flight", 0).ToElement();
	if (NULL == pElement)
		goto LoadCaLowPriceFlightList_Ret;
	for(TiXmlNode*  pNode = pElement; pNode;   pNode = pNode->NextSibling())
	{
		SCaLowPriceFlightInfo* pInfo = new SCaLowPriceFlightInfo;
		TiXmlHandle flightInfoHandle(pNode);
		TiXmlElement* pFlightElement = flightInfoHandle.ToElement();


		pBuf = pFlightElement->Attribute( "productId");
		pInfo->iProductId = atoi(pBuf);
		pBuf = pFlightElement->Attribute( "productType");
		pInfo->iProductType = atoi(pBuf);
		pBuf = pFlightElement->Attribute( "from");
		pInfo->straDCityCode.Format("%s", pBuf);
		pBuf = pFlightElement->Attribute( "to");
		pInfo->straACityCode.Format("%s", pBuf);
		pBuf = pFlightElement->Attribute( "minHangPrice");
		pInfo->iMinHangPrice = atoi(pBuf);
		pBuf = pFlightElement->Attribute( "code");
		pInfo->iCityCode = atoi(pBuf);
		pBuf = pFlightElement->Attribute( "city");
		pInfo->straCity.Format("%s", pBuf);

		m_caLowPriceAirLineList.push_back(pInfo);
	}

LoadCaLowPriceFlightList_Ret:
	TRACE(_T("\r\n-----------load:%d ca low price flight lines\r\n"), m_caLowPriceAirLineList.size());
	return;
}

void CGetFlightsDlg::__ReleaseCaLowPriceFlightList()
{
	SCaLowPriceFlightInfo* pInfo = NULL;
	for (std::list<SCaLowPriceFlightInfo*>::iterator it = m_caLowPriceAirLineList.begin(); it != m_caLowPriceAirLineList.end(); it++)
	{
		pInfo = *it;
		delete pInfo;
		pInfo = NULL;
	}
	m_caLowPriceAirLineList.clear();
}

BOOL CGetFlightsDlg::__GetAqFlightFromSite(BOOL bChangeProxy, CString & strFromCityCode,CString & strToCityCode, 
	CStringA & straStartDate, wstring& httpResponseContent)
{
	BOOL bOk = TRUE;
	CStringA straPostData;

	std::string stdStrDCityUrl = __GetCityNameUrlCode(strFromCityCode);
	std::string stdStrACityUrl = __GetCityNameUrlCode(strToCityCode);

	CStringA straFromCityCode = CStrW2CStrA(strFromCityCode);
	CStringA straToCityCode = CStrW2CStrA(strToCityCode);
	straPostData.Format("traveldate=%s&ori=%s&currency=CNY&dest=%s",straStartDate,straFromCityCode
		, straToCityCode);
	WinHttpClient httpClient(L"http://www.9air.com/app/GetFlight");//(L"http://search.ch.com/default/SearchByTime");
	httpClient.SetTimeouts(0, 25000U, 25000U, 40000U);
	//这一点是更换代理IP的代码
	httpClient.SetProxy(L"117.177.243.114:8081");
	if (bChangeProxy)
		CDataProcess::GetInstance()->ChangeProxy(httpClient);
	httpClient.SetAdditionalDataToSend((BYTE *)straPostData.GetBuffer(0),straPostData.GetLength());
	/*
	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", straPostData.GetLength());
	wstring headers = L"Content-Length: ";
	headers += szSize;
	headers += L"\r\nHost:search.ch.com";
	headers += L"\r\nOrigin:http://search.ch.com";
	headers += L"\r\nContent-Type:application/x-www-form-urlencoded; charset=UTF-8\r\n";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");
	*/
	wstring headers = L"android_version: 1.24";
	headers += L"\r\nContent-Length: 51";
	headers += L"\r\nContent-Type:application/x-www-form-urlencoded";
	headers += L"\r\nHost:www.9air.com";
	headers += L"\r\nConnection: Keep-Alive";
	headers += L"\r\nUser-Agent: Apache-HttpClient/UNAVAILABLE (java 1.4)\r\n";
	httpClient.SetAdditionalRequestHeaders(headers);
	httpClient.SendHttpRequest(L"POST");
	if (0 != httpClient.GetResponseStatusCode().compare(L"200"))
	{
		//CString str;
		//str.Format(_T("-----error code=%s\n"), httpClient.GetResponseStatusCode().c_str());
		//OutputDebugString(str);
		bOk = FALSE;
	}

	if (bOk)
	{
		wstring httpResponseHeader = httpClient.GetResponseHeader();
		httpResponseContent = httpClient.GetResponseContent();
	}

	return bOk;
}
//AQ
void CGetFlightsDlg::__ThreadAqFlightProcess(LPVOID pParam)
{
	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}
	//九元的票没有折扣的区分，这里不用做判断，但为了节省时间，暂时不改，因为配置文件中的配置符合现在的情况
	PTAirLineDetailProcess pProcess = (PTAirLineDetailProcess)pParam;
	PAirLineDetailInfo	pAirlineDetail = NULL;
	UINT	uFlightCount = 1;
	BOOL bChangeProxyIp = FALSE;
	BOOL bIgnoreP4Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP4Ticket;
	BOOL bIgnoreP5Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP5Ticket;
	BOOL bSelSpecialPriceTicket = CDataProcess::GetInstance()->m_configInfo.bSelSpecialPriceTicket;
	int nP4SpecialPrice = CDataProcess::GetInstance()->m_configInfo.nP4SpecialPrice;
	CTime tCur = CTime::GetCurrentTime();
	int iCurYear = tCur.GetYear();
	int iCurMonth = tCur.GetMonth();
	int iiCurDate = tCur.GetDay();
	CTime tSel9Start(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9Start.sTime.u8Minute, 0);
	CTime tSel9End(iCurYear, iCurMonth, iiCurDate, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Hour, CDataProcess::GetInstance()->m_configInfo.uSel9End.sTime.u8Minute, 0);
	int nTryMaxTime = 6;
	TRACE(_T("\n thread process line count=%d"), pProcess->listAirLine.size());
	//循环每一个航线
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	list<PAirLineDetailInfo>::iterator itList;
	for (itList = pProcess->listAirLine.begin(); itList != pProcess->listAirLine.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pAirlineDetail = *itList;
		if (NULL == pAirlineDetail)
		{
			continue;
		}
		//获取携程的航班信息
		CStringA straCtripPostData;
		CStringA straDate;
		straDate.Format("%d-%02d-%02d", pAirlineDetail->tStart.GetYear(), pAirlineDetail->tStart.GetMonth(), pAirlineDetail->tStart.GetDay());
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;
		//change
		list<PT9CFlightInfo> listFlight;
		CAQHtmlParse parseaq;
		listFlight.clear();
		//endchange
		for (int i = 0; i < nTryMaxTime; i++)
		{
			BOOL bRet = __GetAqFlightFromSite(bChangeProxyIp, pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode, straDate,
				httpResponseContent);
			//
			parseaq.ParseJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirlineDetail->_airInfo.uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
			if (!__IsJsonFlightDataOk(listFlight))
				bRet = FALSE;
			//
			if (bRet)
			{
				bGet = TRUE;
				break;
			}
			else
			{//这一步永远很难执行，即使没有抓到有时也会返回TRUE;
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息5次都错误!"),
						CStrA2CStrW(straDate), pAirlineDetail->_airInfo.strFromCityCode, pAirlineDetail->_airInfo.strToCityCode);
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
	
		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		{
			bChangeProxyIp = FALSE;
		}
		
//		if (!bGet)
//			continue;

		//从网页中解析得到的航班信息
/*		list<PT9CFlightInfo> listFlight;
		CAQHtmlParse parseaq;
		listFlight.clear();

		parseaq.ParseJsonFlights(CDataProcess::Unicode2Ansi(httpResponseContent.c_str()),listFlight, pAirlineDetail->_airInfo.uMinHangPrice, tSel9Start, tSel9End, bIgnoreP4Ticket, bIgnoreP5Ticket, bSelSpecialPriceTicket, nP4SpecialPrice);
		if (!__IsJsonFlightDataOk(listFlight))
			bChangeProxyIp = TRUE;*/

		list<PT9CFlightInfo>::iterator it;
		list<PT9CFlightInfo>::iterator itDblist;
		PT9CFlightInfo pFlight = NULL;
		PT9CFlightInfo pDbFlight = NULL;

		//处理查询过程中票已售完或当前这轮没抓到票的数据，票价清0
		list<PT9CFlightInfo> lastFlightList;
		lastFlightList.clear();
		CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		CDbCenter::GetInstance()->QueryAqFlights(lastFlightList, "AQ", stra9CFromCityCode, stra9CToCityCode, straDate);
		m_pThis->__Reset9CFlightInfoList(lastFlightList);
		m_pThis->__Merge9CFlightInfoList(listFlight, lastFlightList);
		//

		//插入数据库
		for (it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			//pFlight->nMinHangPrice;
			pFlight = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			//strLogTmp1.Format("\r\nInsert flightNo=%s,%s:%s->%s, minHan=%d, p1=%d, p2=%d, p3=%d, min=%d"
			//	, pFlight->straFlightNo, pFlight->straFromDate,pFlight->straFromCityCode, pFlight->straToCityCode
			//	, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3, pFlight->nMinPrice);
			//OutputDebugStringA(strLogTmp1);
			CDbCenter::GetInstance()->InsertAqFligthInfo(pFlight);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d|%d", pFlight->straFromCityCode, pFlight->straToCityCode, pFlight->straFromDate, pFlight->straFlightNo
				, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}

		m_pThis->__Free9CFlightInfoList(lastFlightList);
		m_pThis->__Free9CFlightInfoList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}

UINT CGetFlightsDlg::__ThreadInsertAqJobToThreadPool( void* pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	PTAirLineDetailProcess pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto ThreadInsertJobToThreadPoolEx_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();

		map<UINT, PTAirLineDetailProcess>::iterator itMapprocess;
		pThis->__SetCurThreadCount(ThreadCount);

		for (itMapprocess = pThis->m_client1MapProcess.begin(); itMapprocess != pThis->m_client1MapProcess.end(); itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(__ThreadAqFlightProcess,(LPVOID)pProcess);
		}

		Sleep(dwQuerySpan);
	}

ThreadInsertJobToThreadPoolEx_Ret:

	return 0;
}

UINT CGetFlightsDlg::__ThreadInsertCaJobToThreadPool( void* pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	SCaAirLineDetailProcess* pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto ThreadInsertCaJobToThreadPool_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();
		pThis->__SetCurThreadCount(ThreadCount);

		for (map<UINT, SCaAirLineDetailProcess*>::iterator itMapprocess = pThis->m_caMapProcess.begin(); itMapprocess != pThis->m_caMapProcess.end(); itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(__ThreadCaLowPriceFlightProcess,(LPVOID)pProcess);
		}

		Sleep(dwQuerySpan);
	}

ThreadInsertCaJobToThreadPool_Ret:

	return 0;
}

int CGetFlightsDlg::CaClientStart(bool bLoop)
{
	OutputDebugString(_T("开始处理//停止"));
	if (!bLoop && m_bStart)
	{
		Stop();
		return 0;
	}

	//国航只抓低价及团购，map的key不再是日期，仅是一个序号，map的value是航线列表，里面的日期是不同的
	SCaLowPriceFlightInfo * pInfo = NULL;
	AddLog(_T("\r\n开始处理...\r\n"));

	//m_caMapProcess.erase(m_caMapProcess.begin(), m_caMapProcess.end());
	m_caMapProcess.clear();
	int iAirlineTotalCount = m_caLowPriceAirLineList.size();
	if (iAirlineTotalCount <= 0)
		return FALSE;

	//将航线列表分配到map中,map中的list成员中的元素为指针，指向m_caLowPriceAirLineList中的成员
	std::list<SCaLowPriceFlightInfo*>::iterator allLineListIt = m_caLowPriceAirLineList.begin();
	int iLinesPerThread = iAirlineTotalCount / ThreadCount;
	SCaLowPriceFlightInfo* pLowPriceFlightInfo = NULL;
	CTime tDef(2014, 12, 9, 0, 0, 0);
	for (UINT n = 0; n < ThreadCount-1; n++)
	{
		SCaAirLineDetailProcess* pProcess = new SCaAirLineDetailProcess;
		pProcess->pDlg = m_pDlgShowFlights;
		for (int i = 0; i < iLinesPerThread; i++)
		{
			SCaLowPriceFlightInfo *pLineInfo = *allLineListIt;
			pProcess->lowPriceFlightList.push_back(pLineInfo);
			allLineListIt++;
		}
		TRACE(_T("\nthread%d airline count=%d"), n+1, pProcess->lowPriceFlightList.size());
		m_caMapProcess.insert(pair<UINT, SCaAirLineDetailProcess*>(n, pProcess));
	}
	//剩余的航线全加入最后一个列表
	SCaAirLineDetailProcess* pProcess = new SCaAirLineDetailProcess;
	pProcess->pDlg = m_pDlgShowFlights;
	for (; allLineListIt  != m_caLowPriceAirLineList.end(); allLineListIt++)
	{
		SCaLowPriceFlightInfo *pLineInfo = *allLineListIt;
		pProcess->lowPriceFlightList.push_back(pLineInfo);
	}
	TRACE(_T("\nthread%d airline count=%d"), ThreadCount-1, pProcess->lowPriceFlightList.size());
	m_caMapProcess.insert(pair<UINT, SCaAirLineDetailProcess*>(ThreadCount-1, pProcess));
	//

	//开始处理数据
	if (m_caMapProcess.size() <= 0)
	{
		return 0;
	}
	m_bStart = true;
	CDataProcess::GetInstance()->m_bStarting = true;
	m_nThreadEndCount = 0;

	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}

	SetEvent(m_hContinueDoEvt);

	return 0;
}


int CGetFlightsDlg::ClientMobileEStart(bool bLoop)
{
	OutputDebugString(_T("开始处理//停止"));
	if (!bLoop && m_bStart)
	{
		Stop();
		return 0;
	}

	//整理要处理的航线日期信息，将同一个日期的数据放在一个list中，多个日期的数据形成一个map，以日期为key

	PTAirLineDateInfo pInfo = NULL;
	list<PTAirLineDateInfo>::iterator it;  
	CString strKey;
	map<CString,PTAirLineDateProcess>::iterator itMap;
	list<PTAirLineDateInfo> listAirDate;

	if (!(m_pDlgConfig && m_pDlgConfig->GetSafeHwnd() && m_pDlgConfig->m_listAirDateData.size() > 0))
	{
		AddLog(_T("没有要处理的航线信息"));
		return 0;
	}

	listAirDate = m_pDlgConfig->m_listAirDateData;

	AddLog(_T("\r\n开始处理...\r\n"));
	//先清空map
	ClearProcessMap(m_mapProcess);
	OutputDebugString(_T("\r\n开始取得航线日期信息\r\n"));
	for (it = listAirDate.begin(); it != listAirDate.end();it++)  
	{  
		pInfo = *it;
		CTime t = pInfo->tStart;
		CTimeSpan ts = pInfo->tEnd - t;
		while (ts.GetDays() >= 0)
		{
			PTAirLineInfo pAirline = new TAirLineInfo;
			*pAirline = pInfo->_airInfo;

			strKey.Format(_T("%d-%02d-%02d"),t.GetYear(),t.GetMonth(),t.GetDay());
			itMap = m_mapProcess.find(strKey);
			if (itMap != m_mapProcess.end())
			{
				itMap->second->listAirLine.push_back(pAirline);
			}
			else
			{
				PTAirLineDateProcess pProcess = new TAirLineDateProcess;
				pProcess->tDate = t;
				pProcess->listAirLine.push_back(pAirline);

				pProcess->pDlg = m_pDlgShowFlights;
				m_mapProcess.insert(pair<CString,PTAirLineDateProcess>(strKey,pProcess));
			}

			CTimeSpan tsTmp(1,0,0,0);
			t += tsTmp;
			ts = pInfo->tEnd - t;
		}
	}

	//开始处理数据
	if (m_mapProcess.size() <= 0)
	{
		return 0;
	}
	m_bStart = true;
	CDataProcess::GetInstance()->m_bStarting = true;
	m_nThreadEndCount = 0;

	if (m_pDlgConfig && m_pDlgConfig->GetSafeHwnd())
	{
		m_pDlgConfig->EnableCtrls(FALSE);
	}

	SetEvent(m_hContinueDoEvt);

	return 0;
}
void CGetFlightsDlg::__ThreadCaLowPriceFlightProcess( LPVOID pParam )
{
	//map<UINT, SCaAirLineDetailProcess*>::iterator itMapprocess;

	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}

	SCaAirLineDetailProcess* pProcess = (SCaAirLineDetailProcess*)pParam;
	SCaLowPriceFlightInfo*	pLowPriceFlightInfo = NULL;
	UINT	uFlightCount = 1;
	BOOL bChangeProxyIp = FALSE;
	CTime tCur = CTime::GetCurrentTime();
	int iCurYear = tCur.GetYear();
	int iCurMonth = tCur.GetMonth();
	int iiCurDate = tCur.GetDay();
	int nTryMaxTime = 6;
	TRACE(_T("\n thread process line count=%d"), pProcess->lowPriceFlightList.size());
	//循环每一个航线
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	for (std::list<SCaLowPriceFlightInfo*>::iterator itList = pProcess->lowPriceFlightList.begin(); itList != pProcess->lowPriceFlightList.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pLowPriceFlightInfo = *itList;
		if (NULL == pLowPriceFlightInfo)
		{
			continue;
		}
		//获取携程的航班信息
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;
		int iCityCode = pLowPriceFlightInfo->iCityCode;
		CString strCity = CStrA2CStrW(pLowPriceFlightInfo->straCity);
		CStringA straDCode = pDataProcess->ConvertCtripTo9C(pLowPriceFlightInfo->straDCityCode);
		CStringA straACode = pDataProcess->ConvertCtripTo9C(pLowPriceFlightInfo->straACityCode);
		for (int i = 0; i < nTryMaxTime; i++)
		{
			
			BOOL bRet = __GetCaLowPriceFlight(bChangeProxyIp, iCityCode, strCity, httpResponseContent);
			if (bRet)
			{
				bGet = TRUE;
				break;
			}
			else
			{
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s->%s 返回航班信息5次都错误!"), iCityCode, strCity);
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		{
			bChangeProxyIp = FALSE;
		}
	//	if (!bGet)
	//		continue;

		//从网页中解析得到的航班信息
		list<SCaLowPriceFlightDetail*> listFlight;
		listFlight.clear();
		CCaHtmlParse caParser;
		TRACE("Get %s->%s flight\r\n", straDCode, straACode);
		caParser.ParseCaHtmlFlights(listFlight, CDataProcess::Unicode2Ansi(httpResponseContent.c_str()), straDCode, straACode, pLowPriceFlightInfo);

		//处理查询过程中票已售完或当前这轮没抓到票的数据，余票清0
		list<SCaLowPriceFlightDetail*> lastFlightList;
		lastFlightList.clear();
		//CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		//CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		CDbCenter::GetInstance()->QueryCaTuanFlights(lastFlightList, pLowPriceFlightInfo->iProductId);
		m_pThis->__ResetCaTuanFlightInfoList(lastFlightList);
		m_pThis->__MergeCaTuanFlightInfoList(listFlight, lastFlightList);

		//插入数据库
		SCaLowPriceFlightDetail* pDetail = NULL;
		for (list<SCaLowPriceFlightDetail*>::iterator it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			pDetail = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			CDbCenter::GetInstance()->InsertCaTuanFligthInfo(pDetail);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d", pDetail->straFromCityCode, pDetail->straToCityCode, pDetail->straFromDate, pDetail->straFlightNo
				, pDetail->nPrice, pDetail->nRemainSeat, pDetail->nProductId);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}
		m_pThis->__FreeFlightList(lastFlightList);
		m_pThis->__FreeFlightList(listFlight);

		//list<PT9CFlightInfo>::iterator it;
		//list<PT9CFlightInfo>::iterator itDblist;
		//PT9CFlightInfo pFlight = NULL;
		//PT9CFlightInfo pDbFlight = NULL;

		////处理查询过程中票已售完或当前这轮没抓到票的数据，票价清0
		//list<PT9CFlightInfo> lastFlightList;
		//lastFlightList.clear();
		//CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		//CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		//CDbCenter::GetInstance()->QueryFlights(lastFlightList, "9C", stra9CFromCityCode, stra9CToCityCode, straDate);
		//m_pThis->__Reset9CFlightInfoList(lastFlightList);
		//m_pThis->__Merge9CFlightInfoList(listFlight, lastFlightList);
		////

		////插入数据库
		//for (it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		//{
		//	//pFlight->nMinHangPrice;
		//	pFlight = *it;
		//	char* pBuf = new char[128];
		//	CStringA strLogTmp1;
		//	//strLogTmp1.Format("\r\nInsert flightNo=%s,%s:%s->%s, minHan=%d, p1=%d, p2=%d, p3=%d, min=%d"
		//	//	, pFlight->straFlightNo, pFlight->straFromDate,pFlight->straFromCityCode, pFlight->straToCityCode
		//	//	, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3, pFlight->nMinPrice);
		//	//OutputDebugStringA(strLogTmp1);
		//	CDbCenter::GetInstance()->InsertFligthInfo(pFlight);

		//	sprintf_s(pBuf, 128, "%s-%s(%s:%s):%d|%d|%d|%d", pFlight->straFromCityCode, pFlight->straToCityCode, pFlight->straFromDate, pFlight->straFlightNo
		//		, pFlight->nMinHangPrice, pFlight->nPrice1, pFlight->nPrice2, pFlight->nPrice3);
		//	::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		//}

		//m_pThis->__Free9CFlightInfoList(lastFlightList);
		//m_pThis->__Free9CFlightInfoList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}


UINT CGetFlightsDlg::__ThreadInsertMobileEJobToThreadPool( LPVOID pParam )
{
	DWORD dwRet = 0;
	CGetFlightsDlg* pThis = (CGetFlightsDlg* )pParam;
	PTAirLineDateProcess pProcess = NULL;
	DWORD dwQuerySpan = (DWORD)(15 * 1000);

	while (1)
	{
		dwRet = WaitForSingleObject(m_hExitEvt, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			goto ThreadInsertMobileEJobToThreadPool_Ret;
		}

		dwRet = WaitForSingleObject(m_hContinueDoEvt, INFINITE);

		__WailAllJobFinished();
		pThis->__SetCurThreadCount(ThreadCount);

		for (map<CString,PTAirLineDateProcess>::iterator itMapprocess = pThis->m_mapProcess.begin(); itMapprocess != pThis->m_mapProcess.end(); itMapprocess++)
		{
			pProcess = itMapprocess->second;
			g_threadPoolProcess.Call(__CeairMobileEThreadProcess,(LPVOID)pProcess);
		}

		Sleep(dwQuerySpan);
	}

ThreadInsertMobileEJobToThreadPool_Ret:

	return 0;
}

void CGetFlightsDlg::__CeairMobileEThreadProcess( LPVOID pParam )
{
	//OutputDebugString(_T("\r\n开启处理线程\r\n"));
	if (pParam == NULL)
	{
		//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
		//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

		return;
	}

	PTAirLineDateProcess pProcess = (PTAirLineDateProcess)pParam;
	PTAirLineInfo	pAirline = NULL;

	UINT	uFlightCount = 1;
	list<PTAirLineInfo>::iterator itList;
	BOOL bChangeProxyIp = FALSE;
	//BOOL bIgnoreP4Ticket = !CDataProcess::GetInstance()->m_configInfo.bSelP4Ticket;
	int nTryMaxTime = 6;
	CDataProcess* pDataProcess = CDataProcess::GetInstance();
	std::list<SCeairFlightInfo*> listFlight;
	//循环每一个航线
	for (itList = pProcess->listAirLine.begin(); itList != pProcess->listAirLine.end(); itList++)
	{
		if (!CDataProcess::GetInstance()->m_bStarting)
		{
			//退出线程
			//CGetFlightsDlg* pDlg = (CGetFlightsDlg*)AfxGetMainWnd();
			//pDlg->PostMessage(WM_PROCESS_THREAD_END,0,0);

			return;
		}
		pAirline = *itList;
		if (NULL == pAirline)
		{
			continue;
		}

		CStringA straDate;
		straDate.Format("%d-%02d-%02d",pProcess->tDate.GetYear(),pProcess->tDate.GetMonth(),pProcess->tDate.GetDay());
		wstring httpResponseContent(L"");
		BOOL bGet = FALSE;
		
		CStringA straFromCityCode = CStrW2CStrA(pAirline->strFromCityCode);
		CStringA straToCityCode = CStrW2CStrA(pAirline->strToCityCode);
		if(0 == straFromCityCode.CompareNoCase("BJS"))
			straFromCityCode = "PEK";
		else if (0 == straFromCityCode.CompareNoCase("SIA"))
			straFromCityCode = "XIY";
		else
		{

		}
		if(0 == straToCityCode.CompareNoCase("BJS"))
			straToCityCode = "PEK";
		else if (0 == straToCityCode.CompareNoCase("SIA"))
			straToCityCode = "XIY";
		else
		{

		}
		for (int i = 0; i < nTryMaxTime; i++)
		{
			listFlight.clear();
			BOOL bRet = CCeairMobileE::GetCeairMobileEFlight(/*bChangeProxyIp*/FALSE, straFromCityCode, straToCityCode, straDate, listFlight);

			if (bRet)
			{
				bGet = TRUE;
				break;
			}
			else
			{
				bChangeProxyIp = TRUE;
				DWORD dwSpan = 10;
				Sleep(dwSpan*(i+1));
				if(i == (nTryMaxTime-1))
				{
					CString strErr;
					strErr.Format(_T("\n错误:%s:%s->%s 返回航班信息5次都错误!"),
						CStrA2CStrW(straDate), CStrA2CStrW(straFromCityCode),  CStrA2CStrW(straToCityCode));
					OutputDebugString(strErr);
				}
			}
		}
		uFlightCount++;
		if (0 == (uFlightCount % 3) || bChangeProxyIp)
		{
			bChangeProxyIp = FALSE;
		}
		if (!bGet)
			continue;
		/*
		SCeairFlightInfo* pText=(SCeairFlightInfo*)(listFlight.front());
		int nText=pText->nPrice;
		CString Text;
		Text.Format(_T("%d"),nText);
		AfxMessageBox(Text);
		*/
		list<SCeairFlightInfo*>::iterator it;
		list<SCeairFlightInfo*>::iterator itDblist;
		PT9CFlightInfo pFlight = NULL;
		PT9CFlightInfo pDbFlight = NULL;

		//处理查询过程中票已售完或当前这轮没抓到票的数据，余票清0
		list<SCeairFlightInfo*> lastFlightList;
		lastFlightList.clear();
		//CStringA stra9CFromCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strFromCityCode));
		//CStringA stra9CToCityCode = pDataProcess->ConvertCtripTo9C(CStrW2CStrA(pAirlineDetail->_airInfo.strToCityCode));
		CDbCenter::GetInstance()->QueryCeairMEFlights(lastFlightList, straFromCityCode, straToCityCode, straDate);
		m_pThis->__ResetCeairMEFlightInfoList(lastFlightList);
		m_pThis->__MergeCeairMEFlightInfoList(listFlight, lastFlightList);

		//插入数据库
		SCeairFlightInfo* pDetail = NULL;
		for (list<SCeairFlightInfo*>::iterator it = lastFlightList.begin(); it != lastFlightList.end(); it++)
		{
			pDetail = *it;
			char* pBuf = new char[128];
			CStringA strLogTmp1;
			CDbCenter::GetInstance()->InsertCeairMEFlightInfo(pDetail);

			sprintf_s(pBuf, 128, "%s-%s(%s:%s%s):%d|%d", straFromCityCode, straToCityCode, pDetail->straFromDate, pDetail->straCompany, pDetail->straFlightNo
				, pDetail->nPrice, pDetail->nRemainSeat);
			::PostMessage(m_pThis->m_hWnd, WM_UPDATE_FLIGHT, (WPARAM)pBuf, 0);
		}

		m_pThis->__FreeFlightList(lastFlightList);
		m_pThis->__FreeFlightList(listFlight);
	}

	::PostMessage(m_pThis->m_hWnd, WM_PROCESS_THREAD_END, 0, 0);
	//OutputDebugString(_T("------------------------getflight thread exit"));
	return;
}

void CGetFlightsDlg::__ResetCaTuanFlightInfoList(list<SCaLowPriceFlightDetail*> &flightList)
{
	list<SCaLowPriceFlightDetail*>::iterator it;
	SCaLowPriceFlightDetail* pFlight = NULL;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		pFlight->nRemainSeat = 0;
	}
}

void CGetFlightsDlg::__MergeCaTuanFlightInfoList(list<SCaLowPriceFlightDetail*> & fromFlightList, list<SCaLowPriceFlightDetail*> & toFlightList)
{
	list<SCaLowPriceFlightDetail*>::iterator it;
	SCaLowPriceFlightDetail* pFlight = NULL;

	for (it = fromFlightList.begin(); it != fromFlightList.end(); it++)
	{
		pFlight = *it;

		BOOL bFind = FALSE;
		SCaLowPriceFlightDetail* pFindRes = __FindCaTuanFlight(toFlightList, pFlight, &bFind);
		if (bFind)
		{
			//modify
			pFindRes->straSaleEndDate = pFlight->straSaleEndDate;
			pFindRes->straSaleEndTime = pFlight->straSaleEndTime;
			pFindRes->nRemainSeat = pFlight->nRemainSeat;
			pFindRes->nPrice = pFlight->nPrice;
		}
		else
		{
			//append
			SCaLowPriceFlightDetail* pTmpFlight = new SCaLowPriceFlightDetail;
			*pTmpFlight = *pFlight;
			toFlightList.push_back(pTmpFlight);
		}
	}
}

SCaLowPriceFlightDetail* CGetFlightsDlg::__FindCaTuanFlight(list<SCaLowPriceFlightDetail*>  &flightList, SCaLowPriceFlightDetail* pKey, BOOL *pbFind)
{
	list<SCaLowPriceFlightDetail*>::iterator it;
	SCaLowPriceFlightDetail* pFlight = NULL;	
	SCaLowPriceFlightDetail* pFindResult = NULL;

	*pbFind = FALSE;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		if ( (pKey->nProductId == pFlight->nProductId)
			&& (0 == pKey->straFlightNo.CompareNoCase(pFlight->straFlightNo)) 
			&& (0 == pKey->straFromCityCode.CompareNoCase(pFlight->straFromCityCode))
			&& (0 == pKey->straToCityCode.CompareNoCase(pFlight->straToCityCode)) 
			&& (0 == pKey->straFromDate.CompareNoCase(pFlight->straFromDate))
			)
		{
			pFindResult = pFlight;
			*pbFind = TRUE;
			break;
		}
	}
	return pFindResult;
}

void CGetFlightsDlg::__ResetCeairMEFlightInfoList(list<SCeairFlightInfo*> &flightList)
{
	list<SCeairFlightInfo*>::iterator it;
	SCeairFlightInfo* pFlight = NULL;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		pFlight->nRemainSeat = 0;
	}
}

void CGetFlightsDlg::__MergeCeairMEFlightInfoList(list<SCeairFlightInfo*> & fromFlightList, list<SCeairFlightInfo*> & toFlightList)
{
	list<SCeairFlightInfo*>::iterator it;
	SCeairFlightInfo* pFlight = NULL;

	for (it = fromFlightList.begin(); it != fromFlightList.end(); it++)
	{
		pFlight = *it;

		BOOL bFind = FALSE;
		SCeairFlightInfo* pFindRes = __FindCeairMEFlight(toFlightList, pFlight, &bFind);
		if (bFind)
		{
			//modify
			pFindRes->nRemainSeat = pFlight->nRemainSeat;
			pFindRes->nPrice = pFlight->nPrice;
		}
		else
		{
			//append
			SCeairFlightInfo* pTmpFlight = new SCeairFlightInfo;
			*pTmpFlight = *pFlight;
			toFlightList.push_back(pTmpFlight);
		}
	}
}

SCeairFlightInfo* CGetFlightsDlg::__FindCeairMEFlight(list<SCeairFlightInfo*>  &flightList, SCeairFlightInfo* pKey, BOOL *pbFind)
{
	list<SCeairFlightInfo*>::iterator it;
	SCeairFlightInfo* pFlight = NULL;	
	SCeairFlightInfo* pFindResult = NULL;

	*pbFind = FALSE;
	for (it = flightList.begin(); it != flightList.end(); it++)
	{
		pFlight = *it;
		if ( (0 == pKey->straFlightNo.CompareNoCase(pFlight->straFlightNo)) 
			&& (0 == pKey->straFromCityCode.CompareNoCase(pFlight->straFromCityCode))
			&& (0 == pKey->straToCityCode.CompareNoCase(pFlight->straToCityCode)) 
			&& (0 == pKey->straFromDate.CompareNoCase(pFlight->straFromDate))
			)
		{
			pFindResult = pFlight;
			*pbFind = TRUE;
			break;
		}
	}
	return pFindResult;
}