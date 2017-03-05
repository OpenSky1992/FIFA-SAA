#pragma once
#include "TestModule.h"

class TestCorrect:public TestModule
{
public:
	TestCorrect(Rib *pRib,Fib *pFib);
	virtual void updateParameter(UpdatePara *para);
	void examineAlogrithm();
	~TestCorrect(void);
private:
	bool isCorrect(FibTrie *pFibSrc,FibTrie *pTarget);
};

