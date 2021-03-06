#include "StdAfx.h"
#include "DbCenter.h"
#include "./common/CStringToolEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//flight 表的列号
enum
{
	FL_COMPANY_COL=0,
	FL_FROM_DATE_COL,
	FL_FLIGHT_NO_COL,
	FL_FROM_CITY_COL,
	FL_TO_CITY_COL,
	FL_TO_DATE_COL,
	FL_MINHAN_PRICE_COL,
	FL_PRICE1_COL,
	FL_PRICE2_COL,
	FL_PRICE3_COL,
	FL_MIN_PRICE_COL,
	FL_EXT1_COL,
	FL_EXT2_COL
};

//caTuanflight 表的列号
enum
{
	CATUAN_COMPANY_COL=0,
	CATUAN_FROM_DATE_COL,
	CATUAN_FLIGHT_NO_COL,
	CATUAN_FROM_CITY_COL,
	CATUAN_TO_CITY_COL,
	CATUAN_SALE_END_DATE_COL,
	CATUAN_SALE_END_TIME_COL,
	CATUAN_PRICE_COL,
	CATUAN_REMAIN_SEAT_COL,
	CATUAN_PRODUCT_ID_COL,
	CATUAN_PRODUCT_TYPE_COL,
	CATUAN_EXT1_COL,
	CATUAN_EXT2_COL
};


//CeairMEflights 表的列号
enum
{
	CEAIR_ME_COMPANY_COL=0,
	CEAIR_ME_FROM_DATE_COL,
	CEAIR_ME_FLIGHT_NO_COL,
	CEAIR_ME_FROM_CITY_COL,
	CEAIR_ME_TO_CITY_COL,
	CEAIR_ME_PRICE_COL,
	CEAIR_ME_REMAIN_SEAT_COL,
	CEAIR_ME_EXT1_COL,
	CEAIR_ME_EXT2_COL
};
CDbCenter::CDbCenter(void)
{

}


CDbCenter::~CDbCenter(void)
{
}

CDbCenter* CDbCenter::GetInstance()
{
	static CDbCenter db;
	return &db;
}

int CDbCenter::Connect()
{
	return m_dbMysql.connect(CDataProcess::GetInstance()->m_configInfo.strDbHost.GetBuffer(0),
		CDataProcess::GetInstance()->m_configInfo.strDbUser.GetBuffer(0),
		CDataProcess::GetInstance()->m_configInfo.strDbPwd.GetBuffer(0),
		CDataProcess::GetInstance()->m_configInfo.strDbName.GetBuffer(0),
		CDataProcess::GetInstance()->m_configInfo.nDbPort,
		NULL,
		CDataProcess::GetInstance()->m_configInfo.uClientId); //pa5sw0rd
	//return m_dbMysql.connect("127.0.0.1","root","pa5sw0rd","myhifb");
}

// 查询特殊政策
//eDbResult CDbCenter::QuerySpecialPolicy(PTSPolicyLocal pSPLocal,CStringA strPolicyID)
//{
//	if (pSPLocal == NULL)
//	{
//		return DBR_Fail;
//	}
//
//	eDbResult eRet = DBR_Fail;
//
//	CStringA straSql;
//	straSql.Format("select * from SpecialPolicy where PolicyID = '%s'",strPolicyID);
//	m_dbMysqlLock.Lock();
//	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
//	{
//		bool bEmpty = true;
//		while (m_dbMysql.usenext())
//		{
//			pSPLocal->straID = strPolicyID;
//			pSPLocal->straPolicyCode.Format("%s",m_dbMysql.getstring(1).c_str());
//			pSPLocal->straCompany.Format("%s",m_dbMysql.getstring(2).c_str());
//			pSPLocal->straDCode.Format("%s",m_dbMysql.getstring(3).c_str());
//			pSPLocal->straACode.Format("%s",m_dbMysql.getstring(4).c_str());
//			pSPLocal->straSaleEffectDate.Format("%s",m_dbMysql.getstring(5).c_str());
//			pSPLocal->straSaleExpiryDate.Format("%s",m_dbMysql.getstring(6).c_str());
//			pSPLocal->straFlightEffectDates.Format("%s",m_dbMysql.getstring(7).c_str());
//			pSPLocal->straFlightExpiryDates.Format("%s",m_dbMysql.getstring(8).c_str());
//			pSPLocal->straFlightNos.Format("%s",m_dbMysql.getstring(9).c_str());
//			pSPLocal->nSetPrice = m_dbMysql.getint(10);
//
//			bEmpty = false;
//
//			eRet = DBR_Succ;
//			break;
//		}
//
//		if (bEmpty)
//		{
//			eRet = DBR_Empty;
//		}
//
//	}
//	m_dbMysqlLock.Unlock();
//
//	return eRet;
//}

eDbResult CDbCenter::QueryProxyIP(vector<string>& vecIPs)
{
	eDbResult eRet = DBR_Fail;

	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query("select DISTINCT address from sss_proxys_http"))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			vecIPs.push_back(m_dbMysql.getstring(0));

			bEmpty = false;
			eRet = DBR_Succ;
		}

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

//查询使用次数最少的代理IP
eDbResult CDbCenter::QueryProxyIP(int& id, CStringA& strIP, int iFlag)
{
	eDbResult eRet = DBR_Fail;

	strIP = "";
	UINT uClientId = m_dbMysql.GetClientFlag();
	BOOL bMoveToRecord2 = FALSE;
	CStringA straSql;
	straSql.Format("select sss_sid, sss_address from sss_proxys_http  where \
				   sss_sid in (select sss_sid from sss_proxys_http where (sss_sid%%%d) = %d) ORDER BY sss_sssValidIp ASC",
				   MAX_CLIENT_NUM, uClientId);
	//OutputDebugStringA(straSql);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			//春秋ip测试线程开两个，第一个线程取使用最少的第一个ip，第2个线程取使用最少的第2个ip
			if (0 == iFlag)//第一个测试ip的线程
			{
				id = m_dbMysql.getint(0);
				strIP.Format("%s",m_dbMysql.getstring(1).c_str());

				bEmpty = false;
				eRet = DBR_Succ;
				break;
			}
			else//第2个测试ip的线程
			{
				if(bMoveToRecord2)
				{
					id = m_dbMysql.getint(0);
					strIP.Format("%s",m_dbMysql.getstring(1).c_str());

					bEmpty = false;
					eRet = DBR_Succ;
					bMoveToRecord2 = FALSE;
					break;
				}
				else
				{
					bMoveToRecord2 = TRUE;
					continue;
				}
			}
		}

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	else
	{
		eRet = DBR_Fail;
	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

//查询使用次数最少的代理IP
eDbResult CDbCenter::QueryProxyIP(SGetIp* pGetIpBuf, int *pGetCount, int nElementNum, int iFlag)
{
	eDbResult eRet = DBR_Fail;
	*pGetCount = 0;
	SGetIp* pIpBuf = pGetIpBuf;
	for (int i = 0; i < nElementNum; i++)
	{
		pIpBuf->iRowId = 1;
		pIpBuf->szIpBuf[0] = '\0';
	}
	UINT uClientId = m_dbMysql.GetClientFlag();
	BOOL bMoveToRecord2 = FALSE;
	CStringA straSql;
	straSql.Format("select sss_sid, sss_address from sss_proxys_http  where \
				   sss_sid in (select sss_sid from sss_proxys_http where (sss_sid%%%d) = %d) ORDER BY sss_sssValidIp ASC",
				   MAX_CLIENT_NUM, uClientId);
	//OutputDebugStringA(straSql);

	int iFetchCount = 0;
	int iCurRowId = 0;
	int id = 0;
	pIpBuf = pGetIpBuf;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql))
	{
		while (m_dbMysql.usenext())
		{
			//春秋ip测试线程开两个，第一个线程取使用最少的第一个ip，第2个线程取使用最少的第2个ip
			if (0 == iFlag)//第一个测试ip的线程
			{
				if (0 == (iCurRowId %2))//只取该线程对应的ip（编号为偶数的），剩下的由另一个线程取
				{
					id = m_dbMysql.getint(0);
					//TRACE(_T("\r\nFetch ip Thread %d get %d"), iFlag, id);
					iFetchCount++;
					pIpBuf->iRowId = id;
					sprintf_s(pIpBuf->szIpBuf, PROXY_IP_MAX_CHAR, "%s", m_dbMysql.getstring(1).c_str());
					pIpBuf++;
				}

				if (PROXY_IP_BUF_MAX_COUNT == iFetchCount)
				{
					break;
				}
			}
			else//第2个测试ip的线程
			{
				if (1 == (iCurRowId %2))//只取该线程对应的ip（编号为奇数的），剩下的由另一个线程取
				{
					id = m_dbMysql.getint(0);
					//TRACE(_T("\r\nFetch ip Thread %d get %d"), iFlag, id);
					iFetchCount++;
					pIpBuf->iRowId = id;
					sprintf_s(pIpBuf->szIpBuf, PROXY_IP_MAX_CHAR, "%s", m_dbMysql.getstring(1).c_str());
					pIpBuf++;
				}
				if (PROXY_IP_BUF_MAX_COUNT == iFetchCount)
				{
					break;
				}
			}
			iCurRowId++;
		}

		*pGetCount = iFetchCount;
		if (0 != iFetchCount)
		{
			eRet = DBR_Succ;
		}
		else
		{
			eRet = DBR_Empty;
		}

	}
	else
	{
		eRet = DBR_Fail;
	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

//增加代理IP的使用次数
eDbResult CDbCenter::UpdateProxyUsedCount(CStringA& strIP,int nAddCount)
{
	eDbResult eRet = DBR_Fail;
	CStringA straSql;
	//！！！携程和春秋的ip有效性标志分开，春秋用 sssValidIp列， 携程用 ctripValidIp列
	straSql.Format("update sss_proxys_http set sss_sssValidIp = sss_sssValidIp+%d where sss_address = '%s'",nAddCount, strIP);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		eRet = DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	return eRet;
}
//增加代理IP的使用次数
eDbResult CDbCenter::UpdateProxyUsedCount(int iKey,int nAddCount)
{
	eDbResult eRet = DBR_Fail;
	CStringA straSql;
	straSql.Format("update sss_proxys_http set sss_sssValidIp = sss_sssValidIp+%d where sss_sid = '%d'",nAddCount, iKey);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		eRet = DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	return eRet;
}


eDbResult CDbCenter::QueryKnFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate)
{
	CStringA straSql;

	//上海有两个机场，查询时默认查询SHA，再加上PVG，才为完整的上海到上海或从上海出发的航线列表 
	if (0 == straDCode.CompareNoCase("SHA") || 0 == straDCode.CompareNoCase("PVG"))
	{
		straSql.Format("select * from knFlights where Company = '%s' and (FromCityCode = 'SHA' or FromCityCode = 'PVG') and \
					   ToCityCode = '%s' and FromDate = '%s'",
					   straCompany, straACode, straFlightStartDate);
	}
	else if (0 == straACode.CompareNoCase("SHA") || 0 == straACode.CompareNoCase("PVG"))
	{
		straSql.Format("select * from knFlights where Company = '%s' and FromCityCode = '%s' and \
					   (ToCityCode = 'SHA' or ToCityCode = 'PVG') and FromDate = '%s'",
					   straCompany,straDCode, straFlightStartDate);
	}
	else
	{
		straSql.Format("select * from knFlights where Company = '%s' and FromCityCode = '%s' and \
					   ToCityCode = '%s' and FromDate = '%s'",
					   straCompany,straDCode, straACode, straFlightStartDate);
	}

	eDbResult eRet = DBR_Fail;
	listFlights.clear();

	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			PT9CFlightInfo pInfo = new T9CFlightInfo;
			pInfo->straCompany.Format("%s",m_dbMysql.getstring(FL_COMPANY_COL).c_str());
			pInfo->straFromCityCode.Format("%s",m_dbMysql.getstring(FL_FROM_CITY_COL).c_str());
			pInfo->straToCityCode.Format("%s",m_dbMysql.getstring(FL_TO_CITY_COL).c_str());
			pInfo->straFlightNo.Format("%s",m_dbMysql.getstring(FL_FLIGHT_NO_COL).c_str());
			pInfo->straFromDate.Format("%s",m_dbMysql.getstring(FL_FROM_DATE_COL).c_str());
			pInfo->straToDate.Format("%s",m_dbMysql.getstring(FL_TO_DATE_COL).c_str());
			pInfo->nMinHangPrice = m_dbMysql.getint(FL_MINHAN_PRICE_COL);
			pInfo->nPrice1 = m_dbMysql.getint(FL_PRICE1_COL);
			pInfo->nPrice2 = m_dbMysql.getint(FL_PRICE2_COL);
			pInfo->nPrice3 = m_dbMysql.getint(FL_PRICE3_COL);
			pInfo->nMinPrice = m_dbMysql.getint(FL_MIN_PRICE_COL);

			listFlights.push_back(pInfo);

			bEmpty = false;
		}
		eRet = DBR_Succ;

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

// 判断是否已有该条航班信息
bool CDbCenter::IsHaveKnFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo)
{
	CStringA straSql;
	straSql.Format("select * from knFlights where Company = '%s' and FromCityCode = '%s' and \
				   ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
				   straCompany,straDCode, straACode, straFlightStartDate,straFlightNo);
	bool bRet = false;
	*pLastQueryTime = 0;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			int iTotalTimes = m_dbMysql.getint(FL_EXT2_COL);
			if (iTotalTimes < 0)
			{
				*pLastQueryTime = 0;
			}
			else
				*pLastQueryTime = iTotalTimes;

			bRet = true;
			break;
		}
	}
	m_dbMysqlLock.Unlock();

	return bRet;
}

// 根据相关信息查询航班
eDbResult CDbCenter::QueryFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate)
{
	CStringA straSql;

	//上海有两个机场，查询时默认查询SHA，再加上PVG，才为完整的上海到上海或从上海出发的航线列表 
	if (0 == straDCode.CompareNoCase("SHA") || 0 == straDCode.CompareNoCase("PVG"))
	{
		straSql.Format("select * from Flights where Company = '%s' and (FromCityCode = 'SHA' or FromCityCode = 'PVG') and \
					   ToCityCode = '%s' and FromDate = '%s'",
					   straCompany, straACode, straFlightStartDate);
	}
	else if (0 == straACode.CompareNoCase("SHA") || 0 == straACode.CompareNoCase("PVG"))
	{
		straSql.Format("select * from Flights where Company = '%s' and FromCityCode = '%s' and \
					   (ToCityCode = 'SHA' or ToCityCode = 'PVG') and FromDate = '%s'",
					   straCompany,straDCode, straFlightStartDate);
	}
	else
	{
		straSql.Format("select * from Flights where Company = '%s' and FromCityCode = '%s' and \
					   ToCityCode = '%s' and FromDate = '%s'",
					   straCompany,straDCode, straACode, straFlightStartDate);
	}

	eDbResult eRet = DBR_Fail;
	listFlights.clear();

	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			PT9CFlightInfo pInfo = new T9CFlightInfo;
			pInfo->straCompany.Format("%s",m_dbMysql.getstring(FL_COMPANY_COL).c_str());
			pInfo->straFromCityCode.Format("%s",m_dbMysql.getstring(FL_FROM_CITY_COL).c_str());
			pInfo->straToCityCode.Format("%s",m_dbMysql.getstring(FL_TO_CITY_COL).c_str());
			pInfo->straFlightNo.Format("%s",m_dbMysql.getstring(FL_FLIGHT_NO_COL).c_str());
			pInfo->straFromDate.Format("%s",m_dbMysql.getstring(FL_FROM_DATE_COL).c_str());
			pInfo->straToDate.Format("%s",m_dbMysql.getstring(FL_TO_DATE_COL).c_str());
			pInfo->nMinHangPrice = m_dbMysql.getint(FL_MINHAN_PRICE_COL);
			pInfo->nPrice1 = m_dbMysql.getint(FL_PRICE1_COL);
			pInfo->nPrice2 = m_dbMysql.getint(FL_PRICE2_COL);
			pInfo->nPrice3 = m_dbMysql.getint(FL_PRICE3_COL);
			pInfo->nMinPrice = m_dbMysql.getint(FL_MIN_PRICE_COL);

			listFlights.push_back(pInfo);

			bEmpty = false;
		}
		eRet = DBR_Succ;

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

// 判断是否已有该条航班信息
bool CDbCenter::IsHaveFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo)
{
	CStringA straSql;
	straSql.Format("select * from Flights where Company = '%s' and FromCityCode = '%s' and \
				   ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
				   straCompany,straDCode, straACode, straFlightStartDate,straFlightNo);
	bool bRet = false;
	*pLastQueryTime = 0;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			int iTotalTimes = m_dbMysql.getint(FL_EXT2_COL);
			if (iTotalTimes < 0)
			{
				*pLastQueryTime = 0;
			}
			else
				*pLastQueryTime = iTotalTimes;

			bRet = true;
			break;
		}
	}
	m_dbMysqlLock.Unlock();

	return bRet;
}

//插入一条航班信息
eDbResult CDbCenter::InsertFligthInfo(PT9CFlightInfo pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CTime tCur = CTime::GetCurrentTime();
	CStringA straTime;
	straTime.Format("%d:%02d:%02d", tCur.GetHour(),tCur.GetMinute(),tCur.GetSecond());
	CStringA straSql;
	int iLastQueryTimes = 0;
	if (IsHaveFlights(&iLastQueryTimes, pInfo->straCompany, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straFromDate, pInfo->straFlightNo))
	{
		if (iLastQueryTimes > 32700)
			iLastQueryTimes = 1000;
		else
			iLastQueryTimes++;
		straSql.Format("update Flights set ToDate = '%s', MinHangPrice='%d', Price1='%d', Price2='%d', Price3='%d', MinPrice='%d', Extend1='%s', Extend2=%d where FromCityCode='%s'\
					   and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s' "
					   , pInfo->straToDate, pInfo->nMinHangPrice, pInfo->nPrice1,pInfo->nPrice2, pInfo->nPrice3, pInfo->nMinPrice
					   , straTime, iLastQueryTimes, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo);

		//straSql.Format("replace into Flights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
		//			   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '')",
		//			   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
		//			   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
		//			   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}
	else
	{
		straSql.Format("insert into Flights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
					   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '0')",
					   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
					   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
					   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}

	//插入日志
	CStringA straLogSql;
	CStringA straLogTableName;
	straLogTableName.Format("Flights%02d", tCur.GetHour());
	straLogSql.Format("insert into %s(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, InsertTime, price1Json,price2Json,price3Json, Extend2) \
				   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '%s', '%s', '%s', '')", straLogTableName,
				   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
				   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
				   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime, pInfo->straPrice1Json, pInfo->straPrice2Json, pInfo->straPrice3Json);
	//end 插入日志

	//TRACE("%s", straSql);
	eDbResult eRet = DBR_Fail;
	CStringA straLog;

	m_dbMysqlLock.Lock();
	int iRet = m_dbMysql.update(straLogSql.GetBuffer(0));
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		//straLog.Format("\r\n数据库中新加入一条航班信息\r\n");
		//OutputDebugStringA(straLog);
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	//OutputDebugString(_T("\r\n添加政策到数据库失败！\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return eRet;
}


//联航
eDbResult CDbCenter::InsertKnFligthInfo(PT9CFlightInfo pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CTime tCur = CTime::GetCurrentTime();
	CStringA straTime;
	straTime.Format("%d:%02d:%02d", tCur.GetHour(),tCur.GetMinute(),tCur.GetSecond());
	CStringA straSql;
	int iLastQueryTimes = 0;
	if (IsHaveKnFlights(&iLastQueryTimes, pInfo->straCompany, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straFromDate, pInfo->straFlightNo))
	{
		if (iLastQueryTimes > 32700)
			iLastQueryTimes = 1000;
		else
			iLastQueryTimes++;
		straSql.Format("update KnFlights set ToDate = '%s', MinHangPrice='%d', Price1='%d', Price2='%d', Price3='%d', MinPrice='%d', Extend1='%s', Extend2=%d where FromCityCode='%s'\
					   and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s' "
					   , pInfo->straToDate, pInfo->nMinHangPrice, pInfo->nPrice1,pInfo->nPrice2, pInfo->nPrice3, pInfo->nMinPrice
					   , straTime, iLastQueryTimes, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo);

		//straSql.Format("replace into Flights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
		//			   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '')",
		//			   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
		//			   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
		//			   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}
	else
	{
		straSql.Format("insert into KnFlights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
					   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '0')",
					   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
					   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
					   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}

	//插入日志
/*	CStringA straLogSql;
	CStringA straLogTableName;
	straLogTableName.Format("Flights%02d", tCur.GetHour());
	straLogSql.Format("insert into %s(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, InsertTime, price1Json,price2Json,price3Json, Extend2) \
				   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '%s', '%s', '%s', '')", straLogTableName,
				   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
				   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
				   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime, pInfo->straPrice1Json, pInfo->straPrice2Json, pInfo->straPrice3Json);*/
	//end 插入日志

	//TRACE("%s", straSql);
	eDbResult eRet = DBR_Fail;
	CStringA straLog;

	m_dbMysqlLock.Lock();
//	int iRet = m_dbMysql.update(straLogSql.GetBuffer(0));
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		//straLog.Format("\r\n数据库中新加入一条航班信息\r\n");
		//OutputDebugStringA(straLog);
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	//OutputDebugString(_T("\r\n添加政策到数据库失败！\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return eRet;
}
//清除加票标志位
void CDbCenter::ClearAddTicketFlag(const T9CFlightInfo* pFlightInfo)
{
	eDbResult eRet = DBR_Fail;
	CStringA straSql;
	straSql.Format("update Flights set Extend2 = 0 where  Company = '%s' and FromCityCode = '%s' and ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
		pFlightInfo->straCompany, pFlightInfo->straFromCityCode, pFlightInfo->straToCityCode, pFlightInfo->straFromDate, pFlightInfo->straFlightNo);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		eRet = DBR_Succ;
	}
	m_dbMysqlLock.Unlock();
}

//更新一条航班信息
eDbResult CDbCenter::UpdateFlightInfo(PT9CFlightInfo pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CStringA straSql;

	straSql.Format("update Flights set MinHangPrice=%d,Price1=%d,Price2=%d,Price3=%d,MinPrice=%d \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s'",
				   pInfo->nMinHangPrice,pInfo->nPrice1,pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice,
				   pInfo->straFromCityCode,pInfo->straToCityCode,pInfo->straCompany,pInfo->straFromDate,
				   pInfo->straFlightNo);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

//删除指定的航班信息
eDbResult CDbCenter::DeleteFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate)
{
	CStringA straSql;

	straSql.Format("delete from Flights \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s'",
				   straDCode, straACode, straCompany, straFlightStartDate);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
	//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

//删除指定的航班信息
eDbResult CDbCenter::DeleteFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate, CStringA straFlightNo)
{
	CStringA straSql;

	straSql.Format("delete from Flights \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s'",
				   straDCode, straACode, straCompany, straFlightStartDate, straFlightNo);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
	//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

// 删除数据库中无效的航班信息
int CDbCenter::ClearInvalidFlights(void)
{
	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update("delete from specialpolicy"))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();

	OutputDebugString(_T("delete from specialpolicy\r\n"));
	return DBR_Fail;
}


// 判断是否已有该条航班信息
bool CDbCenter::IsHaveAqFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo)
{
	CStringA straSql;
	straSql.Format("select * from aqFlights where Company = '%s' and FromCityCode = '%s' and \
				   ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
				   straCompany,straDCode, straACode, straFlightStartDate,straFlightNo);
	bool bRet = false;
	*pLastQueryTime = 0;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			int iTotalTimes = m_dbMysql.getint(FL_EXT2_COL);
			if (iTotalTimes < 0)
			{
				*pLastQueryTime = 0;
			}
			else
				*pLastQueryTime = iTotalTimes;

			bRet = true;
			break;
		}
	}
	m_dbMysqlLock.Unlock();

	return bRet;
}

// 根据相关信息查询航班
eDbResult CDbCenter::QueryAqFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate)
{
	CStringA straSql;
	straSql.Format("select * from aqFlights where Company = '%s' and FromCityCode = '%s' and \
					   ToCityCode = '%s' and FromDate = '%s'",
					   straCompany,straDCode, straACode, straFlightStartDate);
	eDbResult eRet = DBR_Fail;
	listFlights.clear();

	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			PT9CFlightInfo pInfo = new T9CFlightInfo;
			pInfo->straCompany.Format("%s",m_dbMysql.getstring(FL_COMPANY_COL).c_str());
			pInfo->straFromCityCode.Format("%s",m_dbMysql.getstring(FL_FROM_CITY_COL).c_str());
			pInfo->straToCityCode.Format("%s",m_dbMysql.getstring(FL_TO_CITY_COL).c_str());
			pInfo->straFlightNo.Format("%s",m_dbMysql.getstring(FL_FLIGHT_NO_COL).c_str());
			pInfo->straFromDate.Format("%s",m_dbMysql.getstring(FL_FROM_DATE_COL).c_str());
			pInfo->straToDate.Format("%s",m_dbMysql.getstring(FL_TO_DATE_COL).c_str());
			pInfo->nMinHangPrice = m_dbMysql.getint(FL_MINHAN_PRICE_COL);
			pInfo->nPrice1 = m_dbMysql.getint(FL_PRICE1_COL);
			pInfo->nPrice2 = m_dbMysql.getint(FL_PRICE2_COL);
			pInfo->nPrice3 = m_dbMysql.getint(FL_PRICE3_COL);
			pInfo->nMinPrice = m_dbMysql.getint(FL_MIN_PRICE_COL);

			listFlights.push_back(pInfo);

			bEmpty = false;
		}
		eRet = DBR_Succ;

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

//九元
eDbResult CDbCenter::InsertAqFligthInfo(PT9CFlightInfo pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CTime tCur = CTime::GetCurrentTime();
	CStringA straTime;
	straTime.Format("%d:%02d:%02d", tCur.GetHour(),tCur.GetMinute(),tCur.GetSecond());
	CStringA straSql;
	int iLastQueryTimes = 0;
	if (IsHaveAqFlights(&iLastQueryTimes, pInfo->straCompany, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straFromDate, pInfo->straFlightNo))
	{
		if (iLastQueryTimes > 32700)
			iLastQueryTimes = 1000;
		else
			iLastQueryTimes++;
		straSql.Format("update aqflights set ToDate = '%s', MinHangPrice='%d', Price1='%d', Price2='%d', Price3='%d', MinPrice='%d', Extend1='%s', Extend2=%d where FromCityCode='%s'\
					   and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s' "
					   , pInfo->straToDate, pInfo->nMinHangPrice, pInfo->nPrice1,pInfo->nPrice2, pInfo->nPrice3, pInfo->nMinPrice
					   , straTime, iLastQueryTimes, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo);

		//straSql.Format("replace into Flights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
		//			   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '')",
		//			   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
		//			   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
		//			   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}
	else
	{
		straSql.Format("insert into aqflights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
					   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '0')",
					   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
					   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
					   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}

	//插入日志
//	CStringA straLogSql;
//	CStringA straLogTableName;
//	straLogTableName.Format("Flights%02d", tCur.GetHour());
//	straLogSql.Format("insert into %s(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, InsertTime, price1Json,price2Json,price3Json, Extend2) \
				   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '%s', '%s', '%s', '')", straLogTableName,
//				   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
//				   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
//				   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime, pInfo->straPrice1Json, pInfo->straPrice2Json, pInfo->straPrice3Json);
	//end 插入日志

	//TRACE("%s", straSql);
	eDbResult eRet = DBR_Fail;
	CStringA straLog;

	m_dbMysqlLock.Lock();
//	int iRet = m_dbMysql.update(straLogSql.GetBuffer(0));
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		//straLog.Format("\r\n数据库中新加入一条航班信息\r\n");
		//OutputDebugStringA(straLog);
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	//OutputDebugString(_T("\r\n添加政策到数据库失败！\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return eRet;
}
//清除加票标志位
void CDbCenter::AqClearAddTicketFlag(const T9CFlightInfo* pFlightInfo)
{
	eDbResult eRet = DBR_Fail;
	CStringA straSql;
	straSql.Format("update aqFlights set Extend2 = 0 where  Company = '%s' and FromCityCode = '%s' and ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
		pFlightInfo->straCompany, pFlightInfo->straFromCityCode, pFlightInfo->straToCityCode, pFlightInfo->straFromDate, pFlightInfo->straFlightNo);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		eRet = DBR_Succ;
	}
	m_dbMysqlLock.Unlock();
}

//更新一条航班信息
eDbResult CDbCenter::UpdateAqFlightInfo(PT9CFlightInfo pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CStringA straSql;

	straSql.Format("update aqFlights set MinHangPrice=%d,Price1=%d,Price2=%d,Price3=%d,MinPrice=%d \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s'",
				   pInfo->nMinHangPrice,pInfo->nPrice1,pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice,
				   pInfo->straFromCityCode,pInfo->straToCityCode,pInfo->straCompany,pInfo->straFromDate,
				   pInfo->straFlightNo);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

//删除指定的航班信息
eDbResult CDbCenter::DeleteAqFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate)
{
	CStringA straSql;

	straSql.Format("delete from aqFlights \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s'",
				   straDCode, straACode, straCompany, straFlightStartDate);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
	//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

//删除指定的航班信息
eDbResult CDbCenter::DeleteAqFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate, CStringA straFlightNo)
{
	CStringA straSql;

	straSql.Format("delete from aqFlights \
				   where FromCityCode='%s' and ToCityCode='%s' and Company='%s' and FromDate='%s' and FlightNo='%s'",
				   straDCode, straACode, straCompany, straFlightStartDate, straFlightNo);

	m_dbMysqlLock.Lock();
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}

	m_dbMysqlLock.Unlock();
	//	OutputDebugString(_T("\r\n更新政策到数据库失败！sql:\r\n%s\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return DBR_Fail;
}

//插入一条航班信息
eDbResult CDbCenter::InsertCaTuanFligthInfo(SCaLowPriceFlightDetail* pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CTime tCur = CTime::GetCurrentTime();
	CStringA straTime;
	straTime.Format("%d:%02d:%02d", tCur.GetHour(),tCur.GetMinute(),tCur.GetSecond());
	CStringA straSql;
	int iLastQueryTimes = 0;
	if (IsHaveCaTuanFlights(&iLastQueryTimes, pInfo->straCompany, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straFromDate, pInfo->straFlightNo, pInfo->nProductId))
	{
		if (iLastQueryTimes > 32700)
			iLastQueryTimes = 1000;
		else
			iLastQueryTimes++;
		straSql.Format("update CaTuanflights set SaleEndDate = '%s', SaleEndTime = '%s', Price = '%d', RemainSeat = '%d', ProductType = '%d', Extend1 = '%s', Extend2 = '%d' \
					   where Company= '%s' and FromDate = '%s' and FlightNo= '%s' and FromCityCode = '%s' and ToCityCode = '%s' and ProductId = '%d'"
					   , pInfo->straSaleEndDate, pInfo->straSaleEndTime, pInfo->nPrice, pInfo->nRemainSeat, pInfo->nProductType, straTime, iLastQueryTimes
					   , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->nProductId);
		//straSql.Format("replace into Flights(Company,FromDate,FlightNo,FromCityCode,ToCityCode,ToDate,MinHangPrice,Price1,Price2,Price3,MinPrice, Extend1, Extend2) \
		//			   values('%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%s', '')",
		//			   pInfo->straCompany,pInfo->straFromDate,pInfo->straFlightNo,pInfo->straFromCityCode,
		//			   pInfo->straToCityCode,pInfo->straToDate,pInfo->nMinHangPrice,pInfo->nPrice1,
		//			   pInfo->nPrice2,pInfo->nPrice3,pInfo->nMinPrice, straTime);
	}
	else
	{
		straSql.Format("insert into CaTuanflights(Company, FromDate, FlightNo, FromCityCode, ToCityCode, SaleEndDate,SaleEndTime\
					   , Price, RemainSeat, ProductId, ProductType, Extend1, Extend2) \
					   values('%s','%s','%s','%s','%s','%s','%s', '%d','%d','%d', '%d', '%s','%d')"
					   , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straSaleEndDate
					   , pInfo->straSaleEndTime, pInfo->nPrice, pInfo->nRemainSeat, pInfo->nProductId, pInfo->nProductType, straTime, iLastQueryTimes);
	}

	//插入日志
	CStringA straLogSql;
	CStringA straLogTableName;
	straLogTableName.Format("CaTuanflights%02d", tCur.GetHour());
	straLogSql.Format("insert into %s(Company, FromDate, FlightNo, FromCityCode, ToCityCode, SaleEndDate,SaleEndTime, InsertTime, Price, RemainSeat, ProductId) \
					  values('%s','%s','%s','%s','%s','%s','%s','%s', '%d','%d','%d')", straLogTableName
					  , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straSaleEndDate
					  , pInfo->straSaleEndTime, straTime, pInfo->nPrice, pInfo->nRemainSeat, pInfo->nProductId);
	//end 插入日志

	//TRACE("%s", straSql);
	eDbResult eRet = DBR_Fail;
	CStringA straLog;

	m_dbMysqlLock.Lock();
	int iRet = m_dbMysql.update(straLogSql.GetBuffer(0));
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		//straLog.Format("\r\n数据库中新加入一条航班信息\r\n");
		//OutputDebugStringA(straLog);
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	//OutputDebugString(_T("\r\n添加政策到数据库失败！\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return eRet;
}

// 判断是否已有该条航班信息
bool CDbCenter::IsHaveCaTuanFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo, int nProductId)
{
	CStringA straSql;
	straSql.Format("select * from catuanflights where Company = '%s' and FromCityCode = '%s' and \
				   ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s' and ProductId='%d'",
				   straCompany,straDCode, straACode, straFlightStartDate,straFlightNo, nProductId);
	bool bRet = false;
	*pLastQueryTime = 0;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			int iTotalTimes = m_dbMysql.getint(CATUAN_EXT2_COL);
			if (iTotalTimes < 0)
			{
				*pLastQueryTime = 0;
			}
			else
				*pLastQueryTime = iTotalTimes;

			bRet = true;
			break;
		}
	}
	m_dbMysqlLock.Unlock();

	return bRet;
}

// 根据相关信息查询航班,调用者负责释放list 元素的内存
eDbResult CDbCenter::QueryCaTuanFlights(std::list<SCaLowPriceFlightDetail*> & flightList, int iProductId)
{
	CStringA straSql;
	eDbResult eRet = DBR_Fail;
	flightList.clear();

	straSql.Format("select * from catuanflights where ProductId='%d'", iProductId);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			SCaLowPriceFlightDetail *pInfo = new SCaLowPriceFlightDetail;
			pInfo->straCompany.Format("%s",m_dbMysql.getstring(CATUAN_COMPANY_COL).c_str());
			pInfo->straFromDate.Format("%s",m_dbMysql.getstring(CATUAN_FROM_DATE_COL).c_str());
			pInfo->straFromCityCode.Format("%s",m_dbMysql.getstring(CATUAN_FROM_CITY_COL).c_str());
			pInfo->straToCityCode.Format("%s",m_dbMysql.getstring(CATUAN_TO_CITY_COL).c_str());
			pInfo->straFlightNo.Format("%s",m_dbMysql.getstring(CATUAN_FLIGHT_NO_COL).c_str());
			pInfo->straSaleEndDate.Format("%s",m_dbMysql.getstring(CATUAN_SALE_END_DATE_COL).c_str());
			pInfo->straSaleEndTime = m_dbMysql.getstring(CATUAN_SALE_END_TIME_COL).c_str();
			pInfo->nPrice = m_dbMysql.getint(CATUAN_PRICE_COL);
			pInfo->nRemainSeat = m_dbMysql.getint(CATUAN_REMAIN_SEAT_COL);
			pInfo->nProductId = m_dbMysql.getint(CATUAN_PRODUCT_ID_COL);
			pInfo->nProductType = m_dbMysql.getint(CATUAN_PRODUCT_TYPE_COL);

			flightList.push_back(pInfo);
			bEmpty = false;
		}
		eRet = DBR_Succ;

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

// 根据相关信息查询东航移动E航班
eDbResult CDbCenter::QueryCeairMEFlights(std::list<SCeairFlightInfo*> & flightList, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate)
{
	CStringA straSql;
	eDbResult eRet = DBR_Fail;
	flightList.clear();
	bool bEmpty = false;
	CStringA straQueryDcode("");
	CStringA straQueryAcode("");
	//出发机场匹配相关机场
	if (0 == straDCode.CompareNoCase("SHA") || 0 == straDCode.CompareNoCase("PVG"))
	{
		straQueryDcode = " (FromCityCode = 'SHA' or FromCityCode = 'PVG' ) ";
	}
	else if (0 == straDCode.CompareNoCase("PEK") || 0 == straDCode.CompareNoCase("NAY") || 0 == straDCode.CompareNoCase("BJS"))
	{
		straQueryDcode = " (FromCityCode = 'PEK' or FromCityCode = 'NAY' or FromCityCode = 'BJS') ";
	}
	else if (0 == straDCode.CompareNoCase("XIY") || 0 == straDCode.CompareNoCase("SIA"))
	{
		straQueryDcode = " (FromCityCode = 'XIY' or FromCityCode = 'SIA') ";
	}
	else
	{
		straQueryDcode.Format(" FromCityCode ='%s' ", straDCode);
	}

	//到达机场匹配相关机场
	if (0 == straACode.CompareNoCase("SHA") || 0 == straACode.CompareNoCase("PVG"))
	{
		straQueryAcode = " (ToCityCode = 'SHA' or ToCityCode = 'PVG' ) ";
	}
	else if (0 == straACode.CompareNoCase("PEK") || 0 == straACode.CompareNoCase("NAY") || 0 == straACode.CompareNoCase("BJS"))
	{
		straQueryAcode = " (ToCityCode = 'PEK' or ToCityCode = 'NAY' or ToCityCode = 'BJS') ";
	}
	else if (0 == straACode.CompareNoCase("XIY") || 0 == straACode.CompareNoCase("SIA"))
	{
		straQueryAcode = " (ToCityCode = 'XIY' or ToCityCode = 'SIA') ";
	}
	else
	{
		straQueryAcode.Format(" ToCityCode ='%s' ", straACode);
	}

	straSql.Format("select * from ceairmeflights where %s and \
				   %s and FromDate = '%s' and (Company = 'MU' or Company = 'FM' or Company = 'KN')", straQueryDcode,  straQueryAcode, straFlightStartDate);

	//straSql.Format("select * from ceairmeflights where FromCityCode = '%s' and \
	//			   ToCityCode = '%s' and FromDate = '%s' and (Company = 'MU' or Company = 'FM' or Company = 'KN')", straDCode,  straACode, straFlightStartDate);
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			SCeairFlightInfo *pInfo = new SCeairFlightInfo;
			pInfo->straCompany.Format("%s",m_dbMysql.getstring(CEAIR_ME_COMPANY_COL).c_str());
			pInfo->straFromDate.Format("%s",m_dbMysql.getstring(CEAIR_ME_FROM_DATE_COL).c_str());
			pInfo->straFromCityCode.Format("%s",m_dbMysql.getstring(CEAIR_ME_FROM_CITY_COL).c_str());
			pInfo->straToCityCode.Format("%s",m_dbMysql.getstring(CEAIR_ME_TO_CITY_COL).c_str());
			pInfo->straFlightNo.Format("%s",m_dbMysql.getstring(CEAIR_ME_FLIGHT_NO_COL).c_str());
			pInfo->nPrice = m_dbMysql.getint(CEAIR_ME_PRICE_COL);
			pInfo->nRemainSeat = m_dbMysql.getint(CEAIR_ME_REMAIN_SEAT_COL);

			flightList.push_back(pInfo);
			bEmpty = false;
		}
		eRet = DBR_Succ;

		if (bEmpty)
		{
			eRet = DBR_Empty;
		}

	}
	m_dbMysqlLock.Unlock();

	return eRet;
}

//插入一条东航移动E航班信息
eDbResult CDbCenter::InsertCeairMEFlightInfo(SCeairFlightInfo* pInfo)
{
	if (NULL == pInfo)
	{
		return DBR_Fail;
	}

	CTime tCur = CTime::GetCurrentTime();
	CStringA straTime;
	straTime.Format("%02d:%02d:%02d", tCur.GetHour(),tCur.GetMinute(),tCur.GetSecond());
	CStringA straSql;
	int iLastQueryTimes = 0;
	if (IsHaveCeairMEFlights(&iLastQueryTimes, pInfo->straCompany, pInfo->straFromCityCode, pInfo->straToCityCode, pInfo->straFromDate, pInfo->straFlightNo))
	{
		if (iLastQueryTimes > 32700)
			iLastQueryTimes = 1000;
		else
			iLastQueryTimes++;
		straSql.Format("update ceairmeflights set Price = '%d', RemainSeat = '%d', Extend1 = '%s', Extend2 = '%d' \
					   where Company = '%s' and FromDate = '%s' and FlightNo= '%s' and FromCityCode = '%s' and ToCityCode = '%s'"
					   , pInfo->nPrice, pInfo->nRemainSeat, straTime, iLastQueryTimes
					   , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode);
	}
	else
	{
		straSql.Format("insert into ceairmeflights(Company, FromDate, FlightNo, FromCityCode, ToCityCode \
					   , Price, RemainSeat, Extend1, Extend2) \
					   values('%s','%s','%s','%s','%s', '%d','%d', '%s','%d')"
					   , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode
					   , pInfo->nPrice, pInfo->nRemainSeat, straTime, iLastQueryTimes);
	}

	//插入日志
	CStringA straLogSql;
	CStringA straLogTableName;
	straLogTableName.Format("ceairmeflights%02d", tCur.GetHour());
	straLogSql.Format("insert into %s(Company, FromDate, FlightNo, FromCityCode, ToCityCode, InsertTime, Price, RemainSeat) \
					  values('%s','%s','%s','%s','%s','%s','%d','%d')", straLogTableName
					  , pInfo->straCompany, pInfo->straFromDate, pInfo->straFlightNo, pInfo->straFromCityCode, pInfo->straToCityCode
					  , straTime, pInfo->nPrice, pInfo->nRemainSeat);
	//end 插入日志

	//TRACE("%s", straSql);
	eDbResult eRet = DBR_Fail;
	CStringA straLog;

	m_dbMysqlLock.Lock();
	int iRet = m_dbMysql.update(straLogSql.GetBuffer(0));
	if (0 == m_dbMysql.update(straSql.GetBuffer(0)))
	{
		//straLog.Format("\r\n数据库中新加入一条航班信息\r\n");
		//OutputDebugStringA(straLog);
		m_dbMysqlLock.Unlock();
		return DBR_Succ;
	}
	m_dbMysqlLock.Unlock();

	//OutputDebugString(_T("\r\n添加政策到数据库失败！\r\n"));
	OutputDebugString(CStrA2CStrT(straSql));
	return eRet;
}

// 判断是否已有该条航班信息
bool CDbCenter::IsHaveCeairMEFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo)
{
	CStringA straSql;
	straSql.Format("select * from ceairmeflights where Company = '%s' and FromCityCode = '%s' and \
				   ToCityCode = '%s' and FromDate = '%s' and FlightNo='%s'",
				   straCompany,straDCode, straACode, straFlightStartDate,straFlightNo);
	bool bRet = false;
	*pLastQueryTime = 0;
	m_dbMysqlLock.Lock();
	if(-1 != m_dbMysql.query(straSql.GetBuffer(0)))
	{
		bool bEmpty = true;
		while (m_dbMysql.usenext())
		{
			int iTotalTimes = m_dbMysql.getint(CEAIR_ME_EXT2_COL);
			if (iTotalTimes < 0)
			{
				*pLastQueryTime = 0;
			}
			else
				*pLastQueryTime = iTotalTimes;

			bRet = true;
			break;
		}
	}
	m_dbMysqlLock.Unlock();

	return bRet;
}

