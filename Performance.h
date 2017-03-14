#pragma once
#include "TestModule.h"

class Performance :public TestModule
{
public:
	Performance(Rib *pRib,Fib *pFib):TestModule(pRib,pFib)
	{
		updateIndex=0;
		updateTimeUsed=0;
	}
	~Performance(void);
	virtual void updateParameter(UpdatePara *para);
	void printUseTime();
	void AccUpdate();
private:
	int updateIndex;
	UpdatePara bufferSet[100];
	long long updateTimeUsed;
};

