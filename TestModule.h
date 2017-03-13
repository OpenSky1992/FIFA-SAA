#pragma once
#include "UpdateTravel.h"

class TestModule
{
public:
	TestModule(){}
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


