#pragma once
#include "htmlparse.h"
#include "common/json/json.h"

using namespace Json;
class CAQHtmlParse :
	public CHtmlParse
{
public:
	CAQHtmlParse(void);
	~CAQHtmlParse(void);
public:
	static void SetExtraAddPrice(int nPrice){m_iExtraAddPrice = nPrice;};

public:
	int ParseHtmlFlights(std::string& strHtmlData, list<PT9CFlightInfo>& listFlight);
	int ParseJsonFlights(std::string& strHtmlData, list<PT9CFlightInfo>& listFlight, UINT uMinHangPrice, const CTime & tSel9Start, const CTime & tSel9End, BOOL bIgnoreP4Ticket=TRUE, BOOL bIgnoreP5Ticket=TRUE, BOOL bSelSpecialPriceTicket=TRUE, int nP4SpecialPrice=800);

	int CalcJsonPrice(const char* pstrRate, const char* pstrPrice);
	int CalcJsonPrice2(const char* pstrRate, const char* pstrPrice);

	int GetMinPrice(int nPrice1, int nPrice2, int nPrice3);
	int ParsePrice(CStringA stra);

private:
	static int		m_iExtraAddPrice;	//����Ӽۡ�����ϵͳά��ʱ�������Ǯ������
};

