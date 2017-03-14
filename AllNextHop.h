#pragma once
#include "common.h"

class AllNextHop
{
private:
	std::hash_map<int,int> m_mNextList;
public:
	int existNextHop(int iVal);
	int getSizeOfNHS();
	AllNextHop(void);
	~AllNextHop(void);
};

