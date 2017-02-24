#pragma once
#include "Rib.h"
#include "Fib.h"

class TestModule
{
public:
	TestModule(){}
	~TestModule(void){};
	virtual void updateParameter(UpdatePara *para)=0;

protected:
	Rib *pRibTrie;
	Fib *pFibTrie;
};


