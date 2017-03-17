#include "AllNextHop.h"



AllNextHop::AllNextHop(void)
{
	m_mNextList[0]=0;//very import,because 0 represent default hop in our hash_map
}

int AllNextHop::existNextHop(int iVal)
{
	int length;
	std::hash_map<int,int>::iterator it=m_mNextList.find(iVal);
	if(it==m_mNextList.end())
	{
		length=m_mNextList.size();
		m_mNextList[iVal]=length;
		return -length;
	}
	else
		return it->second;		//should return a value;   it->second
}


AllNextHop::~AllNextHop(void)
{
	m_mNextList.clear();
}


int AllNextHop::getSizeOfNHS()
{
	return m_mNextList.size();
}