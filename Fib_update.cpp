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


FibTrie* Fib::LastVisitNode(int iNextHop,char *insert_C,int &outDeepFib)
{
	int outDeep=0;
	FibTrie *insertNode=m_pTrie;
	FibTrie *pNTrie,*pCounterTrie;
	FibTrie* pLastVisit;

	for (int i=0;i<(int)strlen(insert_C);i++)
	{
		if ('0'==insert_C[i])
		{
			if (NULL==insertNode->pLeftChild)
			{//turn left, if left child is empty, create new node
				CreateNewNode(pNTrie);
				insertNode->pLeftChild=pNTrie;
				pNTrie->pParent=insertNode;

				CreateNewNode(pCounterTrie);
				insertNode->pRightChild=pCounterTrie;
				pCounterTrie->pParent=insertNode;
				pCounterTrie->iNewPort=pCounterTrie->pNextHop->iVal=pLastVisit->pNextHop->iVal;
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
				pCounterTrie->iNewPort=pCounterTrie->pNextHop->iVal=pLastVisit->pNextHop->iVal;
				pCounterTrie->intersection=true;
				outDeep++;
			}
			else
				pLastVisit=insertNode->pRightChild;
			insertNode=insertNode->pRightChild;
		}
	}
	outDeepFib=outDeep;
	if(outDeep>0)
	{
		insertNode->intersection=true;
		insertNode->pNextHop->iVal=iNextHop;
		if(iNextHop!=pLastVisit->pNextHop->iVal)
		{
			if(outDeep==1)
				pLastVisit->intersection=false;
			else //for outDeep>=2
				insertNode->iNewPort=iNextHop;
		}
	}
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
		 pNHop= (struct NextHop*)malloc(NEXTHOPLEN);
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

bool Fib::update_NextHopSet(int iNextHop,char operation,RibTrie* pLastRib,int outDeep,int inherit,FibTrie *pLastFib)
{
	if(operation=='A')
	{
		if(pLastRib->pLeftChild==NULL&&pLastRib->pRightChild==NULL)//announce leaf node
		{
			switch (outDeep)
			{
			case 0:if(iNextHop==pLastFib->pNextHop->iVal){return false;}else{pLastFib->pNextHop->iVal=iNextHop;}break;
			case 1:if(iNextHop==pLastFib->pNextHop->iVal){return false;}NextHopMerge(pLastFib);break;
			default:PassOneTwo(pLastFib);return false;break;
			}
		}
		else
		{
			if(pLastRib->iNextHop==EMPTYHOP)
				if(inherit==iNextHop)
					return false;				
			pLastRib->iNextHop=EMPTYHOP;
			if(updateGoDown_Merge(pLastRib,pLastFib,iNextHop))
			{
				pLastRib->iNextHop=iNextHop;
				return false;
			}
			pLastRib->iNextHop=iNextHop;
		}
	}
	else
	{
		if(outDeepRib>0)//withdraw leaf node
		{
			switch(outDeepRib)
			{
			}
		}
		else          //withdraw to NonLeaf node
		{
			if(pLastRib->iNextHop!=EMPTYHOP)
				if(pLastRib->iNextHop==inherit)
					return false;
			pLastRib->iNextHop=EMPTYHOP;
			if(updateGoDown_Merge(pLastRib,pLastFib,inherit))
				return false;
		}
	}
	return true;
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
		if(pMostBNCF->pLeftChild==pMostTCF)
			pMostBNCF->pRightChild->is_NNC_area=true;
		else
			pMostBNCF->pLeftChild->is_NNC_area=true;
		if(pMostBNCF==NULL)
			break;
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
		inheritOldHopFib=GetAncestorHop(pMostBNCF);
		inheritNewHopFib=inheritOldHopFib;
		pMostBNCF=pMostTCF;
	}
	update_select(pMostBNCF,inheritOldHopFib,inheritNewHopFib);
}

void Fib::update_select(FibTrie *pFib,int oldHop,int newHop)
{
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

void Fib::Update(int iNextHop,char *insert_C,char operation,RibTrie* pLastRib,int outDeepRib,int inherit)
{
	int outDeep,iNextHop_withdraw;
	NextHop *oldNextHopSet;
	FibTrie *pLastFib=LastVisitNode(iNextHop,insert_C,outDeep);
	if(operation!='A')
	{
		pLastFib=withdrawLeaf(pLastFib,outDeepRib);
		outDeep=outDeepRib;
		//iNextHop_withdraw=-1
	}
	oldNextHopSet=CopyNextHopSet(pLastFib->pNextHop);	
	bool ifchange=update_NextHopSet(iNextHop,operation,pLastRib,outDeep,inherit,pLastFib);  //update Nexthop set
	if(ifchange)
		update_process(pLastFib,oldNextHopSet);
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
		pFib->pLeftChild->iNewPort=pFib->pLeftChild->pNextHop->iVal=inheritHop;
		updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		NextHopMerge(pFib);
		return false;
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild==NULL)
	{
		pFib->pRightChild->iNewPort=pFib->pRightChild->pNextHop->iVal=inheritHop;
		updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		NextHopMerge(pFib);
		return false;
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild!=NULL)
	{
		bool leftReturn=updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		bool rightReturn=updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		pFib->is_NNC_area=leftReturn&&rightReturn;
		NextHopMerge(pFib);
		return pFib->is_NNC_area;
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
	int selectHop;
	if(oldSelect!=EMPTYHOP)
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
		pTrie=pTrie->pParent;
	FreeSubTree(pTrie->pLeftChild);
	FreeSubTree(pTrie->pRightChild);
	pTrie->pLeftChild=NULL;
	pTrie->pRightChild=NULL;
	return pTrie;
}