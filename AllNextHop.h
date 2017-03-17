#pragma once
#include "common.h"
using namespace std;

class AllNextHop
{
private:
	unordered_map<int,int> m_mNextList;
public:
	int existNextHop(int iVal);
	int getSizeOfNHS();
	AllNextHop(void);
	~AllNextHop(void);
};

