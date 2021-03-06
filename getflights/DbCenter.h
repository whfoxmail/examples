#pragma once

#include "stdafx.h"
#include "DBMysql.h"
#include "DataProcess.h"
#include "CeairMobileEConst.h"
#include <vector>

enum eDbResult
{
	DBR_Succ = 0,
	DBR_Fail = -1,
	DBR_Empty = 1,
};

class CDbCenter
{
public:
	CDbCenter(void);
	~CDbCenter(void);

public:

	CCriticalSection m_dbMysqlLock;
	CDBMysql	m_dbMysql;

	// 根据相关信息查询航班
	eDbResult QueryFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate);
	eDbResult QueryKnFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate);
	bool IsHaveKnFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo);
	//查询代理IP
	
	// 判断是否已有该条航班信息
	bool IsHaveFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo);

	//查询代理IP
	eDbResult QueryProxyIP(vector<string>& vecIPs);

	//查询使用次数最少的代理IP
	eDbResult QueryProxyIP(int& id, CStringA& strIP, int iFlag);
	eDbResult QueryProxyIP(SGetIp* pGetIpBuf, int *pGetCount, int nElementNum, int iFlag);
	//增加代理IP的使用次数
	eDbResult UpdateProxyUsedCount(CStringA& strIP,int nAddCount);
	eDbResult UpdateProxyUsedCount(int iKey, int nAddCount);

	//插入一条航班信息
	eDbResult InsertFligthInfo(PT9CFlightInfo pInfo);
	//联航
	eDbResult InsertKnFligthInfo(PT9CFlightInfo pInfo);

	//更新一条航班信息
	eDbResult UpdateFlightInfo(PT9CFlightInfo pInfo);
	eDbResult UpdateAqFlightInfo(PT9CFlightInfo pInfo);
	//清除加票标志位
	void ClearAddTicketFlag(const T9CFlightInfo* pFlightInfo);
	void AqClearAddTicketFlag(const T9CFlightInfo* pFlightInfo);
	//删除指定的航班信息
	eDbResult DeleteFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate);
	eDbResult DeleteFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate, CStringA straFlightNo);
	eDbResult DeleteAqFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate);
	eDbResult DeleteAqFlights(CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate, CStringA straFlightNo);

	int Connect();

	static CDbCenter* GetInstance();
	// 删除数据库中无效的航班信息
	int ClearInvalidFlights(void);

public:
	//插入一条航班信息
	eDbResult InsertCaTuanFligthInfo(SCaLowPriceFlightDetail* pInfo);
	// 判断是否已有该条航班信息
	bool IsHaveCaTuanFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo, int nProductId);
	// 根据相关信息查询航班
	eDbResult QueryCaTuanFlights(std::list<SCaLowPriceFlightDetail*> & flightList, int iProductId);

	//东航移动E相关函数声明
	// 根据相关信息查询东航移动E航班
	eDbResult QueryCeairMEFlights(std::list<SCeairFlightInfo*> & flightList, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate);
	//插入一条东航移动E航班信息
	eDbResult InsertCeairMEFlightInfo(SCeairFlightInfo* pInfo);
	// 判断是否已有该条航班信息
	bool IsHaveCeairMEFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo);
	//九元相关函数声明
	//插入一条航班信息
	eDbResult InsertAqFligthInfo(PT9CFlightInfo pInfo);
	//判断是否已有该条航班信息
	bool IsHaveAqFlights(int * pLastQueryTime, const CStringA & straCompany, const CStringA & straDCode, const CStringA & straACode, const CStringA & straFlightStartDate, const CStringA & straFlightNo);
	//根据相关信息查询航班
	eDbResult QueryAqFlights(list<PT9CFlightInfo>& listFlights,CStringA straCompany, CStringA straDCode, CStringA straACode, CStringA straFlightStartDate);

};

