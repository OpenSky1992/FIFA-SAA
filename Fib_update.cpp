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


FibTrie* Fib::LastVisitNode(int iNextHop,char *insert_C,int &outNumber)
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
				pLastVisit=insertNode;
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
				pLastVisit=insertNode;
			insertNode=insertNode->pRightChild;
		}
	}
	outNumber=outDeep;
	if(outDeep>0)
	{
		insertNode->intersection=true;
		insertNode->pNextHop->iVal=iNextHop;
		if(iNextHop!=pLastVisit->pNextHop->iVal)
		{
			insertNode->iNewPort=iNextHop;
			if(outDeep==1)
				pLastVisit->intersection=false;
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
	}
	return pCopyHead;
}


void Fib::Update(int iNextHop,char *insert_C,char operation_type,RibTrie* pRibLast,int outRibDeep,int inheritHopRib)
{
	int outFibDeep;
	FibTrie *pFibLast=LastVisitNode(iNextHop,insert_C,outFibDeep);
	NextHop *oldNextHopSet=CopyNextHopSet(pFibLast->pNextHop);


	//update Nexthop set
	if(outRibDeep>0)  //this is update class updateOutRib
	{
		PassOneTwo(pFibLast);
		switch (outFibDeep)
		{
		case 0:if(iNextHop==pFibLast->pNextHop->iVal){return ;}else{pFibLast->pNextHop->iVal=iNextHop;}break;
		case 1:if(iNextHop==pFibLast->pNextHop->iVal){return ;}break;
		default:
			return ;break;
		}
	}
	else    //this is update class updateInRib
	{
		if(operation_type=='A')//announce
		{
			if(pRibLast->iNextHop==EMPTYHOP)
			{
				if(inheritHopRib==iNextHop)
					return ;
			}
			pRibLast->iNextHop=EMPTYHOP;
			updateGoDown_Merge(pRibLast,pFibLast,iNextHop);
			pRibLast->iNextHop=iNextHop;
		}
		else  //withdraw
		{
			pRibLast->iNextHop=EMPTYHOP;
			updateGoDown_Merge(pRibLast,pFibLast,inheritHopRib);
			pRibLast->iNextHop=iNextHop;
		}
	}


	//Nexthop set changing expand
	FibTrie *pMostBNCF=pFibLast,*pMostTCF=NULL;  //pMostBottomNoChangeFib,pMostTopChangeFib
	int goTopLevel=0,inheritOldHopFib,inheritNewHopFib;
	while(!EqualNextHopSet(oldNextHopSet,pMostBNCF->pNextHop))
	{
		freeNextHopSet(oldNextHopSet);
		pMostTCF=pMostBNCF;
		pMostBNCF=pMostBNCF->pParent;
		if(pMostBNCF==NULL)
			break;
		goTopLevel++;
		oldNextHopSet=CopyNextHopSet(pMostBNCF->pNextHop);
		NextHopMerge(pMostBNCF);
	}
	if(pMostBNCF==pFibLast)    //all nodes' nexthop set don't change,so exit.
		return ;
	
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


	//select precess
	int startBit=(int)strlen(insert_C)-outFibDeep-goTopLevel;
	for(int i=startBit;i<goTopLevel;i++)
	{
		if('0'==insert_C[i])
		{

		}
		else
		{
		}
	}
}

void Fib::updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop)
{
	if(pRib->iNextHop!=EMPTYHOP)
		return;
	if(pRib->pLeftChild==NULL&&pRib->pRightChild!=NULL)
	{
		pFib->pLeftChild->iNewPort=pFib->pLeftChild->pNextHop->iVal=inheritHop;
		updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		NextHopMerge(pFib);
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild==NULL)
	{
		pFib->pRightChild->iNewPort=pFib->pRightChild->pNextHop->iVal=inheritHop;
		updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		NextHopMerge(pFib);
	}
	else if(pRib->pLeftChild!=NULL&&pRib->pRightChild!=NULL)
	{
		updateGoDown_Merge(pRib->pLeftChild,pFib->pLeftChild,inheritHop);
		updateGoDown_Merge(pRib->pRightChild,pFib->pRightChild,inheritHop);
		NextHopMerge(pFib);
	}
	else
	{
		return ;
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