#pragma once
#include "UpdateTravel.h"

class TestModule
{
public:
	TestModule(Rib *pRib,Fib *pFib)
	{
		pRibTrie=pRib;
		pFibTrie=pFib;
		pUpdate=new UpdateTravel(pRib,pFib);
	}
	~TestModule(void){};
	virtual void updateParameter(UpdatePara *para)=0;
	void printInfor()
	{
		pUpdate->printInfor();
	}

protected:
	Rib *pRibTrie;
	Fib *pFibTrie;
	UpdateTravel *pUpdate;
};


