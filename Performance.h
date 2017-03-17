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
	virtual void updateParameter(UpdatePara *para);
	void printUseTime();
	void AccUpdate();
private:
	int updateIndex;
	UpdatePara bufferSet[PERFORMANCE_BUFFER_SIZE];
	long long updateTimeUsed;
};

