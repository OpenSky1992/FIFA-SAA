#pragma once 
#include "Fib.h"


Fib::Fib(void)
{
	CreateNewNode(m_pTrie);
	m_pStatics=new UpdateStatistic();
	m_pStatics->AnnounceNum=0;
	m_pStatics->A_inherit=0;
	m_pStatics->A_inValidNum=0;
	m_pStatics->A_leaf_0=0;
	m_pStatics->A_leaf_1=0;
	m_pStatics->A_leaf_2=0;
	m_pStatics->A_true_goDown=0;

	m_pStatics->WithdrawNum=0;
	m_pStatics->W_inherit=0;
	m_pStatics->W_inValidNum=0;
	m_pStatics->W_leaf_0=0;
	m_pStatics->W_leaf_1=0;
	m_pStatics->W_leaf_2=0;
	m_pStatics->W_true_goDown=0;
}

Fib::~Fib(void)
{
	FreeSubTree(m_pTrie);
	delete m_pStatics;
}

FibTrie* Fib::getFibTrie()
{
	return m_pTrie;
}

UpdateStatistic* Fib::getStatistics()
{
	return m_pStatics;
}

void Fib::prefixNumTravel(FibTrie *pTrie)
{
	if(pTrie==NULL)
		return ;
	m_iTotalNodeNum++;
	if(pTrie->iNewPort!=EMPTYHOP)
	{
		m_iPrefixNum++;
		if(pTrie->iNewPort==DEFAULTHOP)
			m_iNonRouteNum++;
	}
	prefixNumTravel(pTrie->pLeftChild);
	prefixNumTravel(pTrie->pRightChild);
}

int Fib::getPrefixNum()
{
	m_iPrefixNum=0;
	m_iNonRouteNum=0;
	m_iTotalNodeNum=0;
	prefixNumTravel(m_pTrie);
	return m_iPrefixNum;
}

int Fib::getNonRouteNum()
{
	return m_iNonRouteNum;
}

int Fib::getTotalNodeNum()
{
	return m_iTotalNodeNum;
}

void Fib::ConstructFromRib(RibTrie* pRibTrie)
{
	CopyTrieFromRib(pRibTrie,m_pTrie);
}

//copy RibTrie to FibTrie
void Fib::CopyTrieFromRib(RibTrie* pSrcTrie,FibTrie* pDesTrie)
{
	FibTrie* pTrie = pDesTrie;

	//if left child is not empty, copy it
	if(pSrcTrie->pLeftChild!=NULL){
		//new a child node
		FibTrie* pTChild;
		CreateNewNode(pTChild);

		pTChild->pParent = pTrie;
		pTrie->pLeftChild = pTChild;
		pTChild->pParent=pTrie;

		//copy left node
		CopyTrieFromRib(pSrcTrie->pLeftChild,pTrie->pLeftChild);
	}

	//if right child is not empty, copy it
	if(pSrcTrie->pRightChild!=NULL){
		//new a child node
		FibTrie* pTChild;
		CreateNewNode(pTChild);

		pTChild->pParent = pTrie;
		pTrie->pRightChild = pTChild;
		pTChild->pParent=pTrie;

		//copy right node
		CopyTrieFromRib(pSrcTrie->pRightChild,pTrie->pRightChild);
	}


	//assign pNHop to current node's nexthop
	pTrie->pNextHop->iVal=pTrie->iNewPort=pSrcTrie->iNextHop;
	pTrie->pNextHop->pNext=NULL;
}

//creat a new FIB trie node
void Fib::CreateNewNode(FibTrie* &pTrie)
{
	
	pTrie= (struct FibTrie*)malloc(FIBLEN);
	struct NextHop* pNHop = (struct NextHop*)malloc(NEXTHOPLEN);

	if (NULL==pNHop || NULL==pTrie)
	{
		printf("error 2..., exit(0)\n");
		exit(0);
	}

	pTrie->pParent = NULL;
	pTrie->pLeftChild = NULL;
	pTrie->pRightChild = NULL;
	pTrie->iNewPort = EMPTYHOP;
	pTrie->intersection=false;
	pTrie->is_NNC_area=false;

	pNHop->iVal = EMPTYHOP;
	pNHop->pNext = NULL;
	pTrie->pNextHop = pNHop;
}

int Fib::GetAncestorHop(FibTrie* pTrie)
{
	int iHop = DEFAULTHOP;
	while(pTrie!=NULL)
	{
		if(pTrie->iNewPort!=EMPTYHOP)
		{
			iHop=pTrie->iNewPort;
			break;
		}
		pTrie=pTrie->pParent;
	}
	return iHop;
}

void Fib::PassOneTwo(FibTrie *pTrie)
{
	FibTrie* pNTrie;      //creat a new node

	if(pTrie->pLeftChild==NULL && pTrie->pRightChild!=NULL)
	{
		PassOneTwo(pTrie->pRightChild);

		CreateNewNode(pNTrie);
		pNTrie->pParent=pTrie;
		if(pTrie->pNextHop->iVal==EMPTYHOP)
			pNTrie->pNextHop->iVal = GetAncestorHop(pTrie);
		else							   
			pNTrie->pNextHop->iVal = pTrie->pNextHop->iVal;
		pNTrie->intersection=true;
		pTrie->pLeftChild=pNTrie;

		NextHopMerge(pTrie);
	}
	else if(pTrie->pLeftChild!=NULL && pTrie->pRightChild==NULL)
	{
		PassOneTwo(pTrie->pLeftChild);
		
		CreateNewNode(pNTrie);
		pNTrie->pParent=pTrie;
		if(pTrie->pNextHop->iVal==EMPTYHOP)
			pNTrie->pNextHop->iVal = GetAncestorHop(pTrie);
		else						
			pNTrie->pNextHop->iVal = pTrie->pNextHop->iVal;
		pNTrie->intersection=true;
		pTrie->pRightChild=pNTrie;

		NextHopMerge(pTrie);
	}
	else if (pTrie->pLeftChild!=NULL && pTrie->pRightChild!=NULL)
	{
		PassOneTwo(pTrie->pLeftChild);
		PassOneTwo(pTrie->pRightChild);

		NextHopMerge(pTrie);
	}
	else
	{
		pTrie->intersection=true;
		return ;
	}
}

void Fib::NextHopMerge(FibTrie *pTrie)
{
	NextHop* pHopHead=NULL;
	NextHop* pHop=NULL;

	freeNextHopSet(pTrie->pNextHop->pNext);
	pTrie->pNextHop->pNext=NULL;
	//do the intersection 
	NextHop* pLNextHop=pTrie->pLeftChild->pNextHop;
	do{
		//compare the ival in left sub-tree with the ival in right sub-tree one by one
		int iHop=pLNextHop->iVal;
		NextHop* pRNextHop=pTrie->pRightChild->pNextHop;
		do{
			if(iHop==pRNextHop->iVal){
				if(pHop==NULL){
					//record the Nexthop set
					pHopHead = pTrie->pNextHop;
					pHop = pHopHead;
				}
				else{
					//creat a linklist
					pHop->pNext = (struct NextHop*)malloc(NEXTHOPLEN);
					if (NULL==pHop->pNext)
					{
						printf("error 3..., exit(0)\n");
						exit(0);
					}
					pHop = pHop->pNext;
				}

				//store the nexthop in phop
				pHop->iVal = iHop;
				pHop->pNext = NULL;
				break ;
			}
			//get the next right sub-tree
			pRNextHop = pRNextHop->pNext;
		}while(pRNextHop!=NULL);
		//get the next right sub-tree
		pLNextHop = pLNextHop->pNext;
	}while(pLNextHop!=NULL);

	//do the Union operation
	if( pHopHead==NULL){

		//get the nexthop of left sub-tree
		NextHop* pLNextHop=pTrie->pLeftChild->pNextHop;
		do{
			if( pHopHead==NULL){
				pHopHead = pTrie->pNextHop;		//get the father's Nexthop as the first element of the set
				pHop = pHopHead;				//mark the start point of the set
			}
			else{
				pHop->pNext = (struct NextHop*)malloc(NEXTHOPLEN);
				if (NULL==pHop->pNext)
				{
					printf("error 4..., exit(0)\n");
					exit(0);
				}
				pHop = pHop->pNext;
			}
			pHop->iVal=pLNextHop->iVal;
			pHop->pNext = NULL;

			//get the next right sub-tree
			pLNextHop = pLNextHop->pNext;
		}while(pLNextHop!=NULL);

		//get the nexthop of right sub-tree
		NextHop* pRNextHop=pTrie->pRightChild->pNextHop;
		do{
			if( pHopHead==NULL){
				pHopHead = pTrie->pNextHop;		//get the father's Nexthop as the first element of the set
				pHop = pHopHead;				//mark the start point of the set
			}
			else{
				pHop->pNext = (struct NextHop*)malloc(NEXTHOPLEN);
				if (NULL==pHop->pNext)
				{
					printf("error 5..., exit(0)\n");
					exit(0);
				}
				pHop = pHop->pNext;
			}
			pHop->iVal=pRNextHop->iVal;
			pHop->pNext = NULL;

			pRNextHop = pRNextHop->pNext;
		}while(pRNextHop!=NULL);
		pTrie->intersection=false;
	}
	else
		pTrie->intersection=true;
}

bool Fib::ifcontainFunc(int inheritHop,NextHop *ptmp)
{
	bool ifcontain=false;
	while(ptmp!=NULL)
	{
		if (inheritHop==ptmp->iVal)
		{
			ifcontain=true;
			break;
		}
		ptmp=ptmp->pNext;
	}
	return ifcontain;
}

void Fib::freeNextHopSet(NextHop *ptmp)
{
	NextHop *p,*q;
	p=ptmp;
	while(p!=NULL)
	{
		q=p->pNext;
		free(p);
		p=q;
	}
}

void Fib::PassThree(FibTrie *pTrie,int inheritHop)
{
	bool clear=true;
	if (pTrie==NULL)
		return;

	if(pTrie->intersection)
	{
		if(!ifcontainFunc(inheritHop,pTrie->pNextHop))
		{
			pTrie->iNewPort=pTrie->pNextHop->iVal;
			inheritHop=pTrie->pNextHop->iVal;
			clear=false;
		}
	}
	if(clear)
		pTrie->iNewPort=EMPTYHOP;

	PassThree(pTrie->pLeftChild, inheritHop);
	PassThree(pTrie->pRightChild, inheritHop);
}

void Fib::Compress()
{
	if(m_pTrie->iNewPort==EMPTYHOP)
		m_pTrie->iNewPort=m_pTrie->pNextHop->iVal=DEFAULTHOP;
	
	PassOneTwo(m_pTrie);
	PassThree(m_pTrie,DEFAULTHOP-1);  //
}

void Fib::FreeSubTree(FibTrie *FreeNode)
{
	if (NULL==FreeNode)
		return;
	FreeSubTree(FreeNode->pLeftChild);
	FreeNode->pLeftChild=NULL;
	FreeSubTree(FreeNode->pRightChild);
	FreeNode->pRightChild=NULL;
	freeNextHopSet(FreeNode->pNextHop);
	FreeNode->pNextHop=NULL;
	free(FreeNode);
}

