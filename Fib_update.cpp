#include "Fib.h"

bool Fib::EqualNextHopSet(NextHop *pNextA,NextHop *pNextB)
{
	NextHop *pA=pNextA,*pB=pNextB;
	int lenA=0,lenB=0;
	while(pA!=NULL)
	{
		lenA++;
		pA=pA->pNext;
	}
	while(pB!=NULL)
	{
		lenB++;
		pB=pB->pNext;
	}
	if(lenA!=lenB)
		return false;
	pA=pNextA;
	while(pA!=NULL)
	{
		if(!ifcontainFunc(pA->iVal,pNextB))
			return false;
		pA=pA->pNext;
	}
	return true;
}


NextHop* Fib::CopyNextHopSet(NextHop *ptmp)
{
	NextHop *pCopyHead,*pOld,*pCopy;
	struct NextHop* pNHop;
	pOld=ptmp;
	pCopyHead=NULL;
	pCopy=NULL;
	while(pOld!=NULL)
	{
		 pNHop= new NextHop();
		 pNHop->iVal=pOld->iVal;
		 pNHop->pNext=NULL;
		 if(pCopy==NULL)
			 pCopyHead=pNHop;
		 else
			 pCopy->pNext=pNHop;
		 pCopy=pNHop;
		 pOld=pOld->pNext;
	}
	return pCopyHead;
}

void Fib::updateAnnounce(AnnounceInfo *info)
{
	int intNextHop=info->iNextHop;
	FibTrie *insertNode=info->pInsertFib;
	FibTrie *pLastFib=info->pLastFib;
	NextHop *oldNHS=CopyNextHopSet(pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(info->outNumber)
		{
		case 0:
			if(intNextHop==pLastFib->pNextHop->iVal)
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->A_leaf_0++;
#endif
				return ;
			}
			else
				insertNode->pNextHop->iVal=intNextHop;
			break;
		case 1:
			insertNode->pNextHop->iVal=intNextHop;		
			if(intNextHop==pLastFib->pNextHop->iVal)
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->A_leaf_1++;
#endif
				return ;
			}
			else
				PassOneTwo(pLastFib);//not only compute the nexthop set of the pLastFib,but also change the intersection(property) of the pLastFib 
			break;
		default:
			insertNode->pNextHop->iVal=intNextHop;
			PassOneTwo(pLastFib);//change the intersection(property) of the subTrie of pLastFib
			if(intNextHop!=pLastFib->pNextHop->iVal)
				insertNode->iNewPort=intNextHop;
#if STATISTICS_PERFORMANCE
			m_pUpdateStat->A_leaf_2++;
#endif
			return;
		}
	}
	else
	{
		RibTrie *pLastRib=info->pLastRib;
		if(info->isEmpty)
			if(info->inheritHop==intNextHop)
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->A_inherit++;
#endif
				return ;
			}
		pLastRib->iNextHop=EMPTYHOP;
		if(updateGoDown_Merge(pLastRib,pLastFib,intNextHop))
		{
			pLastFib->is_NNC_area=false;
			pLastRib->iNextHop=intNextHop;
#if STATISTICS_PERFORMANCE
			m_pUpdateStat->A_true_goDown++;
#endif
			return ;
		}
		pLastRib->iNextHop=intNextHop;
	}
#if STATISTICS_PERFORMANCE
	m_pUpdateStat->A_select++;
#endif
	update_process(pLastFib,oldNHS);
}

void Fib::updateWithdraw(WithdrawInfo *info)
{
	FibTrie *pLastFib=info->pLastFib;
	NextHop *oldNHS=CopyNextHopSet(pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(info->outNumber)
		{
		case 0:
			if(info->inheritHop==info->oldHop)
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->W_leaf_0++;
#endif
				return ;
			}
			else
				pLastFib->pNextHop->iVal=info->inheritHop;
			break;
		case 1:
			if(info->inheritHop==info->oldHop)
			{
				withdrawLeaf(pLastFib,1);
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->W_leaf_1++;
#endif
				return ;
			}
			else
			{
				oldNHS=CopyNextHopSet(pLastFib->pParent->pNextHop);
				pLastFib=withdrawLeaf(pLastFib,1);
				pLastFib=pLastFib;
				pLastFib->pNextHop->iVal=info->inheritHop;
				freeNextHopSet(pLastFib->pNextHop->pNext);
				pLastFib->pNextHop->pNext=NULL;
			}
			break;
		default:
#if STATISTICS_PERFORMANCE
			m_pUpdateStat->W_leaf_2++;
#endif
			withdrawLeaf(pLastFib,info->outNumber);
			return ;
		}
	}
	else
	{
		RibTrie *pLastRib=info->pLastRib;
		int inherit=info->inheritHop;

		if(info->oldHop==inherit)
		{
#if STATISTICS_PERFORMANCE
			m_pUpdateStat->W_inherit++;
#endif
			return ;
		}
		if(updateGoDown_Merge(pLastRib,pLastFib,inherit))
		{
			pLastFib->is_NNC_area=false;
#if STATISTICS_PERFORMANCE
			m_pUpdateStat->W_true_goDown++;
#endif
			return ;
		}
	}
#if STATISTICS_PERFORMANCE
	m_pUpdateStat->W_select++;
#endif
	update_process(pLastFib,oldNHS);
}

void Fib::update_process(FibTrie *pLastFib,NextHop *oldNHS)
{	
	//Nexthop set changing expand
	FibTrie *pMostBNCF=pLastFib,*pMostTCF=NULL;  //pMostBottomNoChangeFib,pMostTopChangeFib
	int inheritOldHopFib,inheritNewHopFib;
	NextHop *oldNextHopSet=oldNHS;
	while(!EqualNextHopSet(oldNextHopSet,pMostBNCF->pNextHop))
	{
		freeNextHopSet(oldNextHopSet);
		pMostTCF=pMostBNCF;
		pMostBNCF=pMostBNCF->pParent;
		if(pMostBNCF==NULL)
			break;
		if(pMostBNCF->pLeftChild==pMostTCF)
			pMostBNCF->pRightChild->is_NNC_area=true;
		else
			pMostBNCF->pLeftChild->is_NNC_area=true;
		oldNextHopSet=CopyNextHopSet(pMostBNCF->pNextHop);
		NextHopMerge(pMostBNCF);
	}
	//select precess
	if(pMostBNCF==NULL)
	{
		inheritOldHopFib=DEFAULTHOP-1;
		inheritNewHopFib=DEFAULTHOP-1;
		pMostBNCF=m_pTrie;
	}
	else
	{
		if(pMostTCF!=NULL)
		{//inheritOldHopFib must came from its father
			inheritOldHopFib=GetAncestorHop(pMostBNCF);
			inheritNewHopFib=inheritOldHopFib;
			if(pMostBNCF->pLeftChild==pMostTCF)
				pMostBNCF->pRightChild->is_NNC_area=false;
			else
				pMostBNCF->pLeftChild->is_NNC_area=false;
			pMostBNCF=pMostTCF;
		}
		else
		{//inheritOldHopFib must came from its father
			inheritOldHopFib=GetAncestorHop(pMostBNCF->pParent);
			inheritNewHopFib=inheritOldHopFib;
		}
	}
	update_select(pMostBNCF,inheritOldHopFib,inheritNewHopFib);
}

void Fib::update_select(FibTrie *pFib,int oldHop,int newHop)
{
	if(NULL==pFib)
		return ;
	if(pFib->is_NNC_area)
	{
		NsNoChange_common_select(pFib,oldHop,newHop);
		pFib->is_NNC_area=false;
	}
	else
	{
		int inheritOldHopFib=oldHop,inheritNewHopFib=newHop;
		RecursiveSelect(pFib,inheritOldHopFib,inheritNewHopFib);
		update_select(pFib->pLeftChild,inheritOldHopFib,inheritNewHopFib);
		update_select(pFib->pRightChild,inheritOldHopFib,inheritNewHopFib);
	}
}

//this function only for those node is No-Leaf node
//this funciont can find the node that it is NULL in Rib trie but counterNode in Fib trie is not NULL
//and correct this nodes' nexthop set.
bool Fib::updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop)
{
	if(pRib->iNextHop!=EMPTYHOP)
	{
		pFib->is_NNC_area=true;
		return true;
	}
	bool leftReturn,rightReturn,newReturn;
	if(pRib->pLeftChild!=NULL)
		leftReturn=updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
	else
	{
		pFib->pLeftChild->pNextHop->iVal=inheritHop;
		leftReturn=false;
	}
	if(pRib->pRightChild!=NULL)
		rightReturn=updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
	else
	{
		pFib->pRightChild->pNextHop->iVal=inheritHop;
		rightReturn=false;
	}
	newReturn=leftReturn&&rightReturn;
	if(!newReturn)
		NextHopMerge(pFib);
	else
	{
		pFib->pLeftChild->is_NNC_area=false;
		pFib->pRightChild->is_NNC_area=false;
		pFib->is_NNC_area=true;
	}
	return newReturn;
}

void Fib::NsNoChange_common_select(FibTrie *pFib,int oldHop,int newHop)
{
	if(NULL==pFib)
		return ;
	if(oldHop==newHop)
		return ;
	int inheritOldHopFib=oldHop,inheritNewHopFib=newHop;
	RecursiveSelect(pFib,inheritOldHopFib,inheritNewHopFib);
	NsNoChange_common_select(pFib->pLeftChild,inheritOldHopFib,inheritNewHopFib);
	NsNoChange_common_select(pFib->pRightChild,inheritOldHopFib,inheritNewHopFib);
}  

void Fib::RecursiveSelect(FibTrie *pFib,int &inheritOldHopFib,int &inheritNewHopFib)
{
	bool clear=true;
	if(pFib->iNewPort!=EMPTYHOP)
		inheritOldHopFib=pFib->iNewPort;
	if(!ifcontainFunc(inheritNewHopFib,pFib->pNextHop))
	{
		int selectHop=priority_select(pFib->iNewPort,inheritOldHopFib,pFib->pNextHop);
		pFib->iNewPort=selectHop;
		inheritNewHopFib=selectHop;
		clear=false;
	}
	if(clear)
		pFib->iNewPort=EMPTYHOP;
}
int Fib::priority_select(int oldSelect,int oldInherit,NextHop *ptmp)
{
	if(EMPTYHOP!=oldSelect)
	{
		if(ifcontainFunc(oldSelect,ptmp))
			return oldSelect;
		else
			return ptmp->iVal;
	}
	else
	{
		if(ifcontainFunc(oldInherit,ptmp))
			return oldInherit;
		else
			return ptmp->iVal;
	}
}

FibTrie* Fib::withdrawLeaf(FibTrie *pFib,int upLevel)
{
	FibTrie *pTrie=pFib;
	for(int i=0;i<upLevel;i++)
	{
		if(pTrie->pParent==NULL)
			break;
		pTrie=pTrie->pParent;
	}
	FreeSubTree(pTrie->pLeftChild);
	FreeSubTree(pTrie->pRightChild);
	pTrie->pLeftChild=NULL;
	pTrie->pRightChild=NULL;
	return pTrie;
}
