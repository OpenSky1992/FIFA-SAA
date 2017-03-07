#pragma once
#include "common.h"

class AllNextHop
{
private:
	NextHop *m_pHead;
	int length;
public:
	void addNextHop(int iVal);
	int getSizeOfNHS();
	AllNextHop(void);
	~AllNextHop(void);
};

