#include "Fib.h"


void Fib::updateAnnounce(AnnounceInfo *info)
{
	int intNextHop=info->iNextHop;
	FibTrie *insertNode=info->pInsertFib;
	FibTrie *pLastFib=info->pLastFib;
	BitMap oldNHS;
	bitmapCopy(oldNHS,pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(info->outNumber)
		{
		case 0:
			if(intNextHop==bitmapSelect(pLastFib->pNextHop))
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->A_leaf_0++;
#endif
				return ;
			}
			else
				bitmapInitial(insertNode->pNextHop,intNextHop);
			break;
		case 1:
			insertNode->intersection=true;
			bitmapInitial(insertNode->pNextHop,intNextHop);//pLastFib->intersection=false;
			if(intNextHop==bitmapSelect(pLastFib->pNextHop))
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
			insertNode->intersection=true;
			bitmapInitial(insertNode->pNextHop,intNextHop);
			PassOneTwo(pLastFib);//change the intersection(property) of the subTrie of pLastFib
			if(intNextHop!=bitmapSelect(pLastFib->pNextHop))
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
	BitMap oldNHS;
	int inherit=info->inheritHop;
	bitmapCopy(oldNHS,pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(info->outNumber)
		{
		case 0:
			if(inherit==info->oldHop)
			{
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->W_leaf_0++;
#endif
				return ;
			}
			else
				bitmapInitial(pLastFib->pNextHop,inherit);
				//pLastFib->pNextHop->iVal=info->inheritHop;
			break;
		case 1:
			if(inherit==info->oldHop)
			{
				withdrawLeaf(pLastFib,1);
#if STATISTICS_PERFORMANCE
				m_pUpdateStat->W_leaf_1++;
#endif
				return ;
			}
			else
			{
				bitmapCopy(oldNHS,pLastFib->pParent->pNextHop);
				pLastFib=withdrawLeaf(pLastFib,1);
				bitmapInitial(pLastFib->pNextHop,inherit);
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
		if(!info->isEmpty)
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

void Fib::update_process(FibTrie *pLastFib,BitMap oldNHS)
{	
	//Nexthop set changing expand
	FibTrie *pMostBNCF=pLastFib,*pMostTCF=NULL;  //pMostBottomNoChangeFib,pMostTopChangeFib
	int inheritOldHopFib,inheritNewHopFib;
	BitMap oldNextHopSet;
	bitmapCopy(oldNextHopSet,oldNHS);
	while(!bitmapEqual(oldNextHopSet,pMostBNCF->pNextHop))
	{
		pMostTCF=pMostBNCF;
		pMostBNCF=pMostBNCF->pParent;
		if(pMostBNCF==NULL)
			break;
		if(pMostBNCF->pLeftChild==pMostTCF)
			pMostBNCF->pRightChild->is_NNC_area=true;
		else
			pMostBNCF->pLeftChild->is_NNC_area=true;
		bitmapCopy(oldNextHopSet,pMostBNCF->pNextHop);
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

void Fib::update_select(FibTrie *pFib,unsigned int oldHop,unsigned int newHop)
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
		bool clear=true;
		if(pFib->iNewPort!=EMPTYHOP)
			inheritOldHopFib=pFib->iNewPort;
		if(pFib->intersection)
		{
			if(!bitmapExist(newHop,pFib->pNextHop))
			{
				int selectHop=priority_select(pFib->iNewPort,inheritOldHopFib,pFib->pNextHop);
				pFib->iNewPort=selectHop;
				inheritNewHopFib=selectHop;
				clear=false;
			}
		}
		if(clear)
			pFib->iNewPort=EMPTYHOP;
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
	if(pRib->pLeftChild==NULL&&pRib->pRightChild!=NULL)
	{
		bitmapInitial(pFib->pLeftChild->pNextHop,inheritHop);
		updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		NextHopMerge(pFib);
		return false;
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild==NULL)
	{
		bitmapInitial(pFib->pRightChild->pNextHop,inheritHop);
		updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		NextHopMerge(pFib);
		return false;
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild!=NULL)
	{
		bool leftReturn=updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		bool rightReturn=updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		bool newReturn=leftReturn&&rightReturn;
		if(newReturn)
		{//is_NCC_area of the most top no change node is set true,other node must recover to false
			pFib->pLeftChild->is_NNC_area=false;
			pFib->pRightChild->is_NNC_area=false;
			pFib->is_NNC_area=true;//no change,no merge
		}
		else//some nexthop set change,so must merge
			NextHopMerge(pFib);
		return newReturn;
	}
	else
	{//this is the same effect in the first if_statement.because Leaf node must have label
		pFib->is_NNC_area=true;
		return true;
	}
}

void Fib::NsNoChange_common_select(FibTrie *pFib,int oldHop,int newHop)
{
	bool oldHopIn,newHopIn;
	if(oldHop==newHop)
		return ;
	oldHopIn=bitmapExist(oldHop,pFib->pNextHop);
	newHopIn=bitmapExist(newHop,pFib->pNextHop);
	if(pFib->intersection)
	{
		if(!oldHopIn&&newHopIn)
		{
			if(pFib->iNewPort!=newHop)
				NsNoChange_standard_select(pFib,pFib->iNewPort,newHop);
			pFib->iNewPort=EMPTYHOP;
			return ;
		}
		else if(!oldHopIn&&!newHopIn)
			return ;
		else if(oldHopIn&&newHopIn)
		{
			NsNoChange_standard_select(pFib,oldHop,newHop);
			return ;
		}
		else
		{
			pFib->iNewPort=oldHop;
			return ;
		}
	}
	else
	{
		if(!oldHopIn&&newHopIn)
		{
			FibTrie *subTrieOfnewHop=NNC_SS_search(pFib,newHop);
			if(subTrieOfnewHop->iNewPort!=newHop)
				NsNoChange_standard_select(subTrieOfnewHop,subTrieOfnewHop->iNewPort,newHop);
			subTrieOfnewHop->iNewPort=EMPTYHOP;
			return ;
		}
		else if(!oldHopIn&&!newHopIn)
			return ;
		else if(oldHopIn&&newHopIn)
		{
			NNC_SS_Double_search(pFib,oldHop,newHop);
			return ;
		}
		else
		{
			FibTrie *subTrieOfnewHop=NNC_SS_search(pFib,oldHop);
			subTrieOfnewHop->iNewPort=oldHop;
			return ;
		}
	}
}  

void Fib::NsNoChange_standard_select(FibTrie *pFib,int oldHop,int newHop)
{
	NNC_SS_Double_search(pFib->pLeftChild,oldHop,newHop);
	NNC_SS_Double_search(pFib->pRightChild,oldHop,newHop);
}

FibTrie* Fib::NNC_SS_search(FibTrie *pFibRoot,int iNextHop)
{
	FibTrie *pTrie=pFibRoot;
	while(pTrie->intersection==false)
	{
		if(bitmapExist(iNextHop,pTrie->pLeftChild->pNextHop))
			pTrie=pTrie->pLeftChild;
		else
			pTrie=pTrie->pRightChild;
	}
	return pTrie;
}

void Fib::NNC_SS_Double_search(FibTrie *pFibRoot,int oldInheritHop,int newInheritHop)
{
	FibTrie *pOldTrie,*pNewTrie;
	pOldTrie=NNC_SS_search(pFibRoot,oldInheritHop);
	pNewTrie=NNC_SS_search(pFibRoot,newInheritHop);
	if(pOldTrie==pNewTrie)
		NsNoChange_standard_select(pNewTrie,oldInheritHop,newInheritHop);
	else
	{
		pOldTrie->iNewPort=oldInheritHop;
		if(pNewTrie->iNewPort!=newInheritHop)
			NsNoChange_standard_select(pNewTrie,pNewTrie->iNewPort,newInheritHop);
		pNewTrie->iNewPort=EMPTYHOP;
	}
}

unsigned int Fib::priority_select(unsigned int oldSelect,unsigned int oldInherit,BitMap ptmp)
{
	if(EMPTYHOP!=oldSelect)
	{
		if(bitmapExist(oldSelect,ptmp))
			return oldSelect;
		else
			return bitmapSelect(ptmp);
	}
	else
	{
		if(bitmapExist(oldInherit,ptmp))
			return oldInherit;
		else
			return bitmapSelect(ptmp);
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
	pTrie->intersection=true;
	return pTrie;
}