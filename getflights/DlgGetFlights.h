#pragma once
#include "common/SimpleBrowser.h"
#include "afxwin.h"
#include "DataProcess.h"

// CDlgGetFlights 对话框

class CDlgGetFlights : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGetFlights)

public:
	CDlgGetFlights(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgGetFlights();

// 对话框数据
	enum { IDD = IDD_DLG_GET_FLIGHTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();

	void LayoutCtrls(void);
	void ParseDocument(SimpleBrowser& browser);

	afx_msg void OnDocumentComplete1(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnDocumentComplete2(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnDocumentComplete3(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnDocumentComplete4(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnDocumentComplete5(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnDocumentComplete6(NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg void OnNavigateComplete2(NMHDR *pNMHDR,LRESULT *pResult);

private:
	SimpleBrowser m_browser1;
	SimpleBrowser m_browser2;
	SimpleBrowser m_browser3;
	SimpleBrowser m_browser4;
	SimpleBrowser m_browser5;
	SimpleBrowser m_browser6;

	bool	m_bBrowser1Inuse;
	bool	m_bBrowser2Inuse;
	bool	m_bBrowser3Inuse;
	bool	m_bBrowser4Inuse;
	bool	m_bBrowser5Inuse;
	bool	m_bBrowser6Inuse;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//CEdit m_editCtrl1;
	//CEdit m_editCtrl2;
	//CEdit m_editCtrl3;
	//CEdit m_editCtrl4;
	//CEdit m_editCtrl5;
	//CEdit m_editCtrl6;
	bool GetNewFlight(PTAirLineDateSingle pInfo);
	// 取消正在获取的网页
	void CancelAll(void);
	afx_msg void OnBnClickedButton1();
};
