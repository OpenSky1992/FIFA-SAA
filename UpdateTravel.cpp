#pragma once
#include "UpdateTravel.h"


UpdateTravel::UpdateTravel(Rib *pRib,Fib *pFib)
{
	pRibTrie=pRib;
	pFibTrie=pFib;
	m_pWithdraw=new WithdrawInfo();
	m_pAnnounce=new AnnounceInfo();
	m_pTotalStat=new UpdateTotalStatistic();
	m_pAllNHS=pRib->getAllNextHopSet();
}


UpdateTravel::~UpdateTravel(void)
{
	pRibTrie=NULL;
	pFibTrie=NULL;
	delete m_pWithdraw;
	delete m_pAnnounce;
	delete m_pTotalStat;
}


void UpdateTravel::printInfor()
{
	m_pTotalStat->printInfor();
}

void UpdateTravel::Update(UpdatePara *para)
{
	if(para->operate==UPDATE_ANNOUNCE)
	{
		#if STATISTICS_PERFORMANCE
			m_pTotalStat->announceNum++;
		#endif
		announceTravel(para->path,para->nextHop);
	}
	else
	{
		#if STATISTICS_PERFORMANCE
			m_pTotalStat->withdrawNum++;
		#endif
		withdrawTravel(para->path);
	}
}

void UpdateTravel::announceTravel(char *travelPath,int iNextHop)
{
	RibTrie *insertNodeRib=pRibTrie->getRibTrie();
	RibTrie *pNewRib;
	FibTrie *insertNodeFib=pFibTrie->getFibTrie();
	FibTrie *pLastVisit=insertNodeFib;
	FibTrie *pNTrie,*pCounterTrie;
	int default_oldport=DEFAULTHOP;
	int outDeep=0;
	int travelLen=(int)strlen(travelPath);
	int indexOfNextHop=m_pAllNHS->existNextHop(iNextHop);
	if(indexOfNextHop<0)
	{
		indexOfNextHop=-indexOfNextHop;
		if(indexOfNextHop==pFibTrie->BitmapCapacity)
		{
			cout<<"the number of next hop bigger than initial setting."<<endl;
			exit(0);
		}
	}
	for (int i=0;i<travelLen;i++)
	{
		if ('0'==travelPath[i])
		{
			if (NULL==insertNodeFib->pLeftChild)
			{//turn left, if left child is empty, create new node
				pFibTrie->CreateNewNode(pNTrie);
				insertNodeFib->pLeftChild=pNTrie;
				pNTrie->pParent=insertNodeFib;

				pFibTrie->CreateNewNode(pCounterTrie);
				insertNodeFib->pRightChild=pCounterTrie;
				pCounterTrie->pParent=insertNodeFib;
				pFibTrie->bitmapCopy(pCounterTrie->pNextHop,pLastVisit->pNextHop);
				pCounterTrie->intersection=true;
				outDeep++;
				
				pRibTrie->CreateNewNode(pNewRib);
				pNewRib->pParent=insertNodeRib;
				insertNodeRib->pLeftChild=pNewRib;
			}
			else 
			{
				if(NULL==insertNodeRib->pLeftChild)
				{
					pRibTrie->CreateNewNode(pNewRib);
					pNewRib->pParent=insertNodeRib;
					insertNodeRib->pLeftChild=pNewRib;
				}
				pLastVisit=insertNodeFib->pLeftChild;
			}
			insertNodeRib=insertNodeRib->pLeftChild;
			insertNodeFib=insertNodeFib->pLeftChild;
		}
		else
		{//turn right, if right child is empty, create new node
			if (NULL==insertNodeFib->pRightChild)
			{
				pFibTrie->CreateNewNode(pNTrie);
				insertNodeFib->pRightChild=pNTrie;
				pNTrie->pParent=insertNodeFib;

				pFibTrie->CreateNewNode(pCounterTrie);
				insertNodeFib->pLeftChild=pCounterTrie;
				pCounterTrie->pParent=insertNodeFib;
				pFibTrie->bitmapCopy(pCounterTrie->pNextHop,pLastVisit->pNextHop);
				pCounterTrie->intersection=true;
				outDeep++;

				pRibTrie->CreateNewNode(pNewRib);
				pNewRib->pParent=insertNodeRib;
				insertNodeRib->pRightChild=pNewRib;
			}
			else 
			{
				if(NULL==insertNodeRib->pRightChild)
				{
					pRibTrie->CreateNewNode(pNewRib);
					pNewRib->pParent=insertNodeRib;
					insertNodeRib->pRightChild=pNewRib;
				}
				pLastVisit=insertNodeFib->pRightChild;
			}
			insertNodeRib=insertNodeRib->pRightChild;
			insertNodeFib=insertNodeFib->pRightChild;
		}
		if (insertNodeRib->pParent!=NULL)
			if (insertNodeRib->pParent->iNextHop!=EMPTYHOP)
				default_oldport=insertNodeRib->pParent->iNextHop;
	}

	m_pAnnounce->inheritHop=default_oldport;
	if (insertNodeRib->iNextHop==indexOfNextHop)
	{
		#if STATISTICS_PERFORMANCE
			m_pTotalStat->A_invalid++;
		#endif
		return ;
	}
	if(insertNodeRib->pLeftChild==NULL&&insertNodeRib->pRightChild==NULL)
	{
		m_pAnnounce->isLeaf=true;
		m_pAnnounce->pLastRib=NULL;//Leaf node don't need this parameter
	}
	else
	{
		m_pAnnounce->isLeaf=false;
		m_pAnnounce->pLastRib=insertNodeRib;
		if(insertNodeRib->iNextHop==EMPTYHOP)
			m_pAnnounce->isEmpty=true;
		else
			m_pAnnounce->isEmpty=false;
	}
	insertNodeRib->iNextHop=indexOfNextHop;

	m_pAnnounce->iNextHop=indexOfNextHop;
	m_pAnnounce->pLastFib=pLastVisit;
	m_pAnnounce->pInsertFib=insertNodeFib;
	m_pAnnounce->outNumber=outDeep;

	pFibTrie->updateAnnounce(m_pAnnounce);
}

void UpdateTravel::withdrawTravel(char *travelPath)
{
	FibTrie *pLastVisitFib=pFibTrie->getFibTrie();
	RibTrie *pLastVisitRib=pRibTrie->getRibTrie();
	int default_oldport=DEFAULTHOP;
	int travelLen=(int)strlen(travelPath);
	for (int i=0;i<travelLen;i++)
	{
		if ('0'==travelPath[i])
		{
			if (NULL==pLastVisitRib->pLeftChild)
			{
				#if STATISTICS_PERFORMANCE
					m_pTotalStat->W_invalid++;
				#endif
				return ;
			}
			pLastVisitRib=pLastVisitRib->pLeftChild;
			pLastVisitFib=pLastVisitFib->pLeftChild;
		}
		else
		{
			if(NULL==pLastVisitRib->pRightChild)
			{
				#if STATISTICS_PERFORMANCE
					m_pTotalStat->W_invalid++;
				#endif
				return ;
			}
			pLastVisitRib=pLastVisitRib->pRightChild;
			pLastVisitFib=pLastVisitFib->pRightChild;
		}
		if(pLastVisitRib->pParent!=NULL)
			if(pLastVisitRib->pParent->iNextHop!=EMPTYHOP)
				default_oldport=pLastVisitRib->pParent->iNextHop;
	}

	if (EMPTYHOP==pLastVisitRib->iNextHop)//invalid delete operation
	{
		#if STATISTICS_PERFORMANCE
			m_pTotalStat->W_invalid++;
		#endif
		return ;
	}
	m_pWithdraw->inheritHop=default_oldport;
	if (pLastVisitRib->pLeftChild==NULL&&pLastVisitRib->pRightChild==NULL)
	{
		m_pWithdraw->isLeaf=true;
		m_pWithdraw->oldHop=pLastVisitRib->iNextHop;
		m_pWithdraw->outNumber=pRibTrie->withdrawLeafNode(pLastVisitRib);
		m_pWithdraw->pLastRib=NULL;  //Leaf node don't need this parameter
	}
	else
	{
		m_pWithdraw->isLeaf=false;
		if(pLastVisitRib->iNextHop==EMPTYHOP)
			m_pWithdraw->isEmpty=true;
		else
		{
			m_pWithdraw->oldHop=pLastVisitRib->iNextHop;
			m_pWithdraw->isEmpty=false;
			pLastVisitRib->iNextHop=EMPTYHOP;
		}
		m_pWithdraw->pLastRib=pLastVisitRib;
	}
	m_pWithdraw->pLastFib=pLastVisitFib;

	pFibTrie->updateWithdraw(m_pWithdraw);
}

