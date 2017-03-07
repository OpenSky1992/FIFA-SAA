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

FibTrie* Fib::lastVisitWithdraw(char *travelPath)
{
	FibTrie *pLastVisit=m_pTrie;
	int travelLen=(int)strlen(travelPath);
	for (int i=0;i<travelLen;i++)
	{
		if ('0'==travelPath[i])
			pLastVisit=pLastVisit->pLeftChild;
		else
			pLastVisit=pLastVisit->pRightChild;
	}
	return pLastVisit;
}

FibTrie* Fib::lastVisitAnnounce(char *travelPath,FibTrie* &insertLeaf,int &deep)
{
	FibTrie *insertNode=m_pTrie;
	FibTrie *pNTrie,*pCounterTrie;
	FibTrie *pLastVisit=m_pTrie;
	int outDeep=0;

	for (int i=0;i<(int)strlen(travelPath);i++)
	{
		if ('0'==travelPath[i])
		{
			if (NULL==insertNode->pLeftChild)
			{//turn left, if left child is empty, create new node
				CreateNewNode(pNTrie);
				insertNode->pLeftChild=pNTrie;
				pNTrie->pParent=insertNode;

				CreateNewNode(pCounterTrie);
				insertNode->pRightChild=pCounterTrie;
				pCounterTrie->pParent=insertNode;
				pCounterTrie->pNextHop->iVal=pLastVisit->pNextHop->iVal;
				pCounterTrie->intersection=true;
				outDeep++;
			}
			else
				pLastVisit=insertNode->pLeftChild;
			insertNode=insertNode->pLeftChild;
		}
		else
		{//turn right, if right child is empty, create new node
			if (NULL==insertNode->pRightChild)
			{
				CreateNewNode(pNTrie);
				insertNode->pRightChild=pNTrie;
				pNTrie->pParent=insertNode;

				CreateNewNode(pCounterTrie);
				insertNode->pLeftChild=pCounterTrie;
				pCounterTrie->pParent=insertNode;
				pCounterTrie->pNextHop->iVal=pLastVisit->pNextHop->iVal;
				pCounterTrie->intersection=true;
				outDeep++;
			}
			else
				pLastVisit=insertNode->pRightChild;
			insertNode=insertNode->pRightChild;
		}
	}
	deep=outDeep;
	insertLeaf=insertNode;
	return pLastVisit;
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

void Fib::Update(UpdatePara *para,UpdateRib *info)
{
	if(UPDATE_ANNOUNCE==para->operate)
	{
		m_pUpdateStat->AnnounceNum++;
		if(!info->valid)
		{
			m_pUpdateStat->A_inValidNum++;
			return ;
		}
		updateAnnounce(para->nextHop,para->path,info);
	}
	else
	{
		m_pUpdateStat->WithdrawNum++;
		if(!info->valid)
		{
			m_pUpdateStat->W_inValidNum++;
			return ;
		}
		updateWithdraw(para->path,info);
	}
}

void Fib::updateAnnounce(int intNextHop,char *travelPath,UpdateRib *info)
{
	FibTrie *insertNode=NULL;
	int outNumber=0;
	FibTrie *pLastFib=lastVisitAnnounce(travelPath,insertNode,outNumber);
	NextHop *oldNHS=CopyNextHopSet(pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(outNumber)
		{
		case 0:
			if(intNextHop==pLastFib->pNextHop->iVal)
			{
				m_pUpdateStat->A_leaf_0++;
				return ;
			}
			else
				insertNode->pNextHop->iVal=intNextHop;
			break;
		case 1:
			insertNode->intersection=true;
			insertNode->pNextHop->iVal=intNextHop;			//pLastFib->intersection=false;
			if(intNextHop==pLastFib->pNextHop->iVal)
			{
				m_pUpdateStat->A_leaf_1++;
				return ;
			}
			else
				PassOneTwo(pLastFib);//not only compute the nexthop set of the pLastFib,but also change the intersection(property) of the pLastFib 
			break;
		default:
			insertNode->intersection=true;
			insertNode->pNextHop->iVal=intNextHop;
			PassOneTwo(pLastFib);//change the intersection(property) of the subTrie of pLastFib
			if(intNextHop!=pLastFib->pNextHop->iVal)
				insertNode->iNewPort=intNextHop;
			m_pUpdateStat->A_leaf_2++;
			return;
		}
	}
	else
	{
		RibTrie *pLastRib=info->pLastRib;
		if(info->isEmpty)
			if(info->inheritHop==intNextHop)
			{
				m_pUpdateStat->A_inherit++;
				return ;
			}
		pLastRib->iNextHop=EMPTYHOP;
		if(updateGoDown_Merge(pLastRib,pLastFib,intNextHop))
		{
			pLastFib->is_NNC_area=false;
			pLastRib->iNextHop=intNextHop;
			m_pUpdateStat->A_true_goDown++;
			return ;
		}
		pLastRib->iNextHop=intNextHop;
	}
	update_process(pLastFib,oldNHS);
}

void Fib::updateWithdraw(char *travelPath,UpdateRib *info)
{
	FibTrie *pLastFib=lastVisitWithdraw(travelPath);
	NextHop *oldNHS=CopyNextHopSet(pLastFib->pNextHop);

	if(info->isLeaf)
	{
		switch(info->w_outNumber)
		{
		case 0:
			if(info->inheritHop==info->w_OldHop)
			{
				m_pUpdateStat->W_leaf_0++;
				return ;
			}
			else
				pLastFib->pNextHop->iVal=info->inheritHop;
			break;
		case 1:
			if(info->inheritHop==info->w_OldHop)
			{
				withdrawLeaf(pLastFib,1);
				m_pUpdateStat->W_leaf_1++;
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
			m_pUpdateStat->W_leaf_2++;
			withdrawLeaf(pLastFib,info->w_outNumber);
			return ;
		}
	}
	else
	{
		RibTrie *pLastRib=info->pLastRib;
		int inherit=info->inheritHop;

		if(!info->isEmpty)
			if(info->w_OldHop==inherit)
			{
				m_pUpdateStat->W_inherit++;
				return ;
			}
		if(updateGoDown_Merge(pLastRib,pLastFib,inherit))
		{
			pLastFib->is_NNC_area=false;
			m_pUpdateStat->W_true_goDown++;
			return ;
		}
	}
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
		bool clear=true;
		if(pFib->iNewPort!=EMPTYHOP)
			inheritOldHopFib=pFib->iNewPort;
		if(pFib->intersection)
		{
			if(!ifcontainFunc(newHop,pFib->pNextHop))
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
		pFib->pLeftChild->pNextHop->iVal=inheritHop;
		updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		NextHopMerge(pFib);
		return false;
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild==NULL)
	{
		pFib->pRightChild->pNextHop->iVal=inheritHop;
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
	oldHopIn=ifcontainFunc(oldHop,pFib->pNextHop);
	newHopIn=ifcontainFunc(newHop,pFib->pNextHop);
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
		if(ifcontainFunc(iNextHop,pTrie->pLeftChild->pNextHop))
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
		if(pNewTrie->iNewPort==newInheritHop)
			pNewTrie->iNewPort=EMPTYHOP;
		else
			NsNoChange_standard_select(pNewTrie,pNewTrie->iNewPort,newInheritHop);
	}
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
	pTrie->intersection=true;
	return pTrie;
}