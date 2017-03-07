#include "AllNextHop.h"


AllNextHop::AllNextHop(void)
{
	m_pHead=NULL;
	length=0;
}

void AllNextHop::addNextHop(int iVal)
{
	NextHop* pTmp=m_pHead;
	NextHop* pLast=NULL;
	while(pTmp!=NULL)
	{
		if(pTmp->iVal==iVal)
			return ;
		pLast=pTmp;
		pTmp=pTmp->pNext;
	}
	length++;
	pTmp=new NextHop();
	pTmp->iVal=iVal;
	pTmp->pNext=NULL;
	if(pLast==NULL)
		m_pHead=pTmp;
	else
		pLast->pNext=pTmp;
}


AllNextHop::~AllNextHop(void)
{
	NextHop *pTmp=m_pHead,*pNeed=NULL;
	while(pTmp!=NULL)
	{
		pNeed=pTmp->pNext;
		delete pTmp;
		pTmp=pNeed;
	}
}


int AllNextHop::getSizeOfNHS()
{
	return length;
}