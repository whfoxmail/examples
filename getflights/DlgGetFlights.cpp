// DlgGetFlights.cpp : 实现文件
//

#include "stdafx.h"
#include "GetFlights.h"
#include "DlgGetFlights.h"
#include "afxdialogex.h"
#include "C9CHtmlParse.h"
#include "GetFlightsDlg.h"
#include "DbCenter.h"


// CDlgGetFlights 对话框

IMPLEMENT_DYNAMIC(CDlgGetFlights, CDialogEx)

CDlgGetFlights::CDlgGetFlights(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGetFlights::IDD, pParent)
{
	m_bBrowser1Inuse = false;
	m_bBrowser2Inuse = true;
	m_bBrowser3Inuse = true;
	m_bBrowser4Inuse = true;
	m_bBrowser5Inuse = true;
	m_bBrowser6Inuse = true;

}

CDlgGetFlights::~CDlgGetFlights()
{
}

void CDlgGetFlights::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgGetFlights, CDialogEx)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER1,OnDocumentComplete1)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER2,OnDocumentComplete2)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER3,OnDocumentComplete3)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER4,OnDocumentComplete4)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER5,OnDocumentComplete5)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER6,OnDocumentComplete6)
	ON_NOTIFY(SimpleBrowser::NavigateComplete2,IDC_BROWSER1,OnNavigateComplete2)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER1,OnDocumentComplete1)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER2,OnDocumentComplete2)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER3,OnDocumentComplete3)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER4,OnDocumentComplete4)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER5,OnDocumentComplete5)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER6,OnDocumentComplete6)
	//ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER1,OnNavigateComplete2)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgGetFlights::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDlgGetFlights 消息处理程序


BOOL CDlgGetFlights::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LayoutCtrls();

	// TODO:  在此添加额外的初始化
	m_browser1.CreateFromControl(this,IDC_BROWSER1);
	m_browser2.CreateFromControl(this,IDC_BROWSER2);
	m_browser3.CreateFromControl(this,IDC_BROWSER3);
	m_browser4.CreateFromControl(this,IDC_BROWSER4);
	m_browser5.CreateFromControl(this,IDC_BROWSER5);
	m_browser6.CreateFromControl(this,IDC_BROWSER6);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgGetFlights::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::DestroyWindow();
}


void CDlgGetFlights::LayoutCtrls(void)
{
	CRect rc;
	GetClientRect(&rc);
	int cx = rc.Width();
	int cy = rc.Height();

	int nSep = 10;

	int nCellwidth = (cx - nSep*4)/3;
	int nCellheight = (cy - nSep*3)/2;

	if (m_browser1.GetSafeHwnd())
	{
		m_browser1.SetWindowPos(&wndTop,nSep,nSep,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_browser2.GetSafeHwnd())
	{
		m_browser2.SetWindowPos(&wndTop,nSep*2 + nCellwidth,nSep,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_browser3.GetSafeHwnd())
	{
		m_browser3.SetWindowPos(&wndTop,nSep*3 + nCellwidth*2,nSep,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_browser4.GetSafeHwnd())
	{
		m_browser4.SetWindowPos(&wndTop,nSep,nSep*2 + nCellheight,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_browser5.GetSafeHwnd())
	{
		m_browser5.SetWindowPos(&wndTop,nSep*2 + nCellwidth,nSep*2 + nCellheight,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	if (m_browser6.GetSafeHwnd())
	{
		m_browser6.SetWindowPos(&wndTop,nSep*3 + nCellwidth*2,nSep*2 + nCellheight,nCellwidth,nCellheight,SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
}

void CDlgGetFlights::OnDocumentComplete1(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;
	/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser1.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser1);
	//	lpHtmlDocument->close();
	}
//	m_browser1.Clear();
	
	m_bBrowser1Inuse = false;

	*pResult = 0;
}

void CDlgGetFlights::OnDocumentComplete2(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser2.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser2);
	//	lpHtmlDocument->close();
	}
//	m_browser2.Clear();
	m_bBrowser2Inuse = false;

	*pResult = 0;
}

void CDlgGetFlights::OnDocumentComplete3(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser3.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser3);
	//	lpHtmlDocument->close();
	}
//	m_browser3.Clear();
	m_bBrowser3Inuse = false;

	*pResult = 0;
}

void CDlgGetFlights::OnDocumentComplete4(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser4.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser4);
	//	lpHtmlDocument->close();
	}
//	m_browser4.Clear();
	m_bBrowser4Inuse = false;

	*pResult = 0;
}

void CDlgGetFlights::OnDocumentComplete5(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser5.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser5);
	//	lpHtmlDocument->close();
	}
//	m_browser5.Clear();
	m_bBrowser5Inuse = false;

	*pResult = 0;
}

void CDlgGetFlights::OnDocumentComplete6(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;/*
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}*/

	IHTMLDocument2 *lpHtmlDocument = m_browser6.GetDocument();
	if (NULL != lpHtmlDocument)
	{
		ParseDocument(m_browser6);
	//	lpHtmlDocument->close();
	}
//	m_browser6.Clear();
	m_bBrowser6Inuse = false;

	*pResult = 0;
}


void CDlgGetFlights::OnNavigateComplete2(NMHDR *pNMHDR,LRESULT *pResult)
{
	SimpleBrowser::Notification	
		*notification = (SimpleBrowser::Notification *)pNMHDR;
	if (!(notification->progress > 0 && notification->progress ==	notification->progress_max))
	{
		return;
	}

	IHTMLDocument2 *lpHtmlDocument = m_browser1.GetDocument();
//	ParseDocument(lpHtmlDocument);

	*pResult = 0;
}

void CDlgGetFlights::ParseDocument(SimpleBrowser& browser)
{
	IHTMLDocument2 *lpHtmlDocument = browser.GetDocument();

	if (NULL == lpHtmlDocument)
	{
		return;
	}

	IHTMLElement *lpBodyElm;
	IHTMLElement *lpParentElm;

	lpHtmlDocument->get_body(&lpBodyElm);
	ASSERT(lpBodyElm);
	lpHtmlDocument->Release();

	lpBodyElm->get_parentElement(&lpParentElm);
	ASSERT(lpParentElm);
	BSTR    bstr;
	lpParentElm->get_outerHTML(&bstr);
	//lpParentElm->get_innerHTML(&bstr);
	CString str = bstr;

	if (str.GetLength() > 0x30)
	{
		std::string html(CStrT2CStrA(str).GetBuffer(0));
		list<PT9CFlightInfo> flightsList;
		C9CHtmlParse parse;
		parse.ParseHtmlFlights(html,flightsList);

		list<PT9CFlightInfo>::iterator it;
		for (it = flightsList.begin(); it != flightsList.end(); it++)
		{
			PT9CFlightInfo pInfo = *it;
			if (CDbCenter::GetInstance()->IsHaveFlights(pInfo->straCompany,pInfo->straFromCityCode,pInfo->straToCityCode,pInfo->straFromDate,pInfo->straFlightNo))
			{
				CDbCenter::GetInstance()->UpdateFlightInfo(pInfo);
			}
			else
			{
				CDbCenter::GetInstance()->InsertFligthInfo(pInfo);
			}
			CStringA straLog;
			straLog.Format("%s-%s(%s:%s):%d|%d|%d",
				pInfo->straFromCityCode,pInfo->straToCityCode,
				pInfo->straFromDate,pInfo->straFlightNo,
				pInfo->nPrice1,pInfo->nPrice2,pInfo->nPrice3);
			((CGetFlightsDlg*)AfxGetMainWnd())->AddLog(CStrA2CStrT(straLog));

			delete pInfo;
		}
		flightsList.clear();
	}


	lpParentElm->Release();
	lpBodyElm->Release();
}

void CDlgGetFlights::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	LayoutCtrls();
}


bool CDlgGetFlights::GetNewFlight(PTAirLineDateSingle pInfo)
{
	if (NULL == pInfo)
	{
		return false;
	}
	CString strUrl;
	strUrl.Format(_T("http://www.china-sss.com/AirFlights/SearchFlights?OriCityCode=%s&DestCityCode=%s&FlightDate=%d-%02d-%02d&FlightDateReturn=%d-%02d-%02d&IsReturn=False&MoneyType=0&AdultNum=1&ChildNum=0&InfantNum=0"),
		pInfo->_airInfo.strFromCityCode,
		pInfo->_airInfo.strToCityCode,
		pInfo->tDate.GetYear(),pInfo->tDate.GetMonth(),pInfo->tDate.GetDay(),
		pInfo->tDate.GetYear(),pInfo->tDate.GetMonth(),pInfo->tDate.GetDay());

	bool bRet = false;
	if (!m_bBrowser1Inuse)
	{
		m_bBrowser1Inuse = true;
		m_browser1.Navigate(strUrl);
		bRet = true;
	}
	//else if (!m_bBrowser2Inuse)
	//{
	//	m_bBrowser2Inuse = true;
	//	m_browser2.Navigate(strUrl);
	//	bRet = true;
	//}
	//else if (!m_bBrowser3Inuse)
	//{
	//	m_bBrowser3Inuse = true;
	//	m_browser3.Navigate(strUrl);
	//	bRet = true;
	//}
	//else if (!m_bBrowser4Inuse)
	//{
	//	m_bBrowser4Inuse = true;
	//	m_browser4.Navigate(strUrl);
	//	bRet = true;
	//}
	//else if (!m_bBrowser5Inuse)
	//{
	//	m_bBrowser5Inuse = true;
	//	m_browser5.Navigate(strUrl);
	//	bRet = true;
	//}
	//else if (!m_bBrowser6Inuse)
	//{
	//	m_bBrowser6Inuse = true;
	//	m_browser6.Navigate(strUrl);
	//	bRet = true;
	//}

	if (bRet)
	{
		CString strLog;
		
		strLog.Format(_T("开始获取:%s-%s(%d%02d%02d)"),
			pInfo->_airInfo.strFromCityCode,pInfo->_airInfo.strToCityCode,
			pInfo->tDate.GetYear(),pInfo->tDate.GetMonth(),pInfo->tDate.GetDay());
		((CGetFlightsDlg*)AfxGetMainWnd())->AddLog(strLog);
	}

	return bRet;
}


// 取消正在获取的网页
void CDlgGetFlights::CancelAll(void)
{
	m_browser1.Stop();
	m_browser2.Stop();
	m_browser3.Stop();
	m_browser4.Stop();
	m_browser5.Stop();
	m_browser6.Stop();

	m_browser1.Clear();
	m_browser2.Clear();
	m_browser3.Clear();
	m_browser4.Clear();
	m_browser5.Clear();
	m_browser6.Clear();
	
	m_bBrowser1Inuse = false;
	m_bBrowser2Inuse = false;
	m_bBrowser3Inuse = false;
	m_bBrowser4Inuse = false;
	m_bBrowser5Inuse = false;
	m_bBrowser6Inuse = false;
}


void CDlgGetFlights::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	LRESULT ret;
	OnDocumentComplete1(NULL,&ret);
}
