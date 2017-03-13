#pragma once
#include "Rib.h"
#include "Fib.h"

class UpdateTravel
{
public:

	UpdateTravel(Rib *pRib,Fib *pFib);

	void Update(UpdatePara *para);
	void printInfor();

	~UpdateTravel(void);

private:
	Rib *pRibTrie;
	Fib *pFibTrie;

	AnnounceInfo* m_pAnnounce;
	WithdrawInfo* m_pWithdraw;

	UpdateTotalStatistic* m_pTotalStat;

	void withdrawTravel(char *travel);
	void announceTravel(char *travel,int iNextHop);
};

