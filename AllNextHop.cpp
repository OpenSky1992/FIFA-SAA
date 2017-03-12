#pragma once
#include "AllNextHop.h"



AllNextHop::AllNextHop(void)
{
}

void AllNextHop::addNextHop(int iVal)
{
	int length;
	std::map<int,int>::iterator it=m_mNextList.find(iVal);
	if(it==m_mNextList.end())
	{
		length=m_mNextList.size()+1;
		m_mNextList.insert(std::map<int,int>::value_type(iVal,length));
	}
	else
		return ;
}


AllNextHop::~AllNextHop(void)
{

}


int AllNextHop::getSizeOfNHS()
{
	return m_mNextList.size();
}