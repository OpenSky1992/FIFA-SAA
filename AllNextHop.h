#pragma once
#include "common.h"
#include <map>

class AllNextHop
{
private:
	std::map<int,int> m_mNextList;
public:
	void addNextHop(int iVal);
	int getSizeOfNHS();
	AllNextHop(void);
	~AllNextHop(void);
};

