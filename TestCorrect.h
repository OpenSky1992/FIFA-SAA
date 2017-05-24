#pragma once
#include "TestModule.h"

class TestCorrect:public TestModule
{
public:
	TestCorrect(Rib *pRib,Fib *pFib):TestModule(pRib,pFib){}
	virtual void updateParameter(UpdatePara *para);
	void examineAlgorithm();
	bool exammineOnebyOne();
private:
	bool forwardCorrect(RibTrie *pRib,int inheritR,FibTrie *pFib,int inheritF);
	bool NHS_isCorrect(FibTrie *pFibSrc,FibTrie *pTarget);
};

