#include "Fib.h"

#include <time.h>



Fib::Fib(void)
{
	CreateNewNode(m_pTrie);
	m_pUpdateStat=new UpdateFibStatistic();
	m_pFibTrieStat=new FibTrieStatistic();
}

Fib::~Fib(void)
{
	FreeSubTree(m_pTrie);
	delete m_pUpdateStat;
	delete m_pFibTrieStat;
}

FibTrie* Fib::getFibTrie()
{
	return m_pTrie;
}

UpdateFibStatistic* Fib::getUpdateStatistics()
{
	return m_pUpdateStat;
}

void Fib::prefixNumTravel(FibTrie *pTrie)
{
	if(pTrie==NULL)
		return ;
	m_pFibTrieStat->totalNodeNum++;
	if(pTrie->iNewPort!=EMPTYHOP)
	{
		m_pFibTrieStat->prefixNum++;
		if(pTrie->iNewPort==DEFAULTHOP)
			m_pFibTrieStat->nonRouteNum++;
	}
	NextHop *ptmp=pTrie->pNextHop;
	while(ptmp!=NULL)
	{
		m_pFibTrieStat->totalNextHopNum++;
		ptmp=ptmp->pNext;
	}
	prefixNumTravel(pTrie->pLeftChild);
	prefixNumTravel(pTrie->pRightChild);
}

FibTrieStatistic* Fib::getFibTrieStatistic()
{
	m_pFibTrieStat->reset();
	prefixNumTravel(m_pTrie);
	return m_pFibTrieStat;
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
	
	pTrie= new FibTrie();
	NextHop* pNHop =new NextHop();

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
					pHop->pNext =new NextHop();
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
				pHop->pNext = new NextHop();
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
				pHop->pNext =new NextHop();
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
		delete p;
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
	

	//LARGE_INTEGER frequence,privious,privious1;
	//if(!QueryPerformanceFrequency(&frequence))	return ;
	//QueryPerformanceCounter(&privious); 
	PassOneTwo(m_pTrie);
	//QueryPerformanceCounter(&privious1);
	//printf("pass one and two:             %d microsecond\n",1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart);

	PassThree(m_pTrie,DEFAULTHOP-1);  //
	//QueryPerformanceCounter(&privious);
	//printf("pass three:                   %d microsecond\n",1000000*(privious.QuadPart-privious1.QuadPart)/frequence.QuadPart);
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
	delete FreeNode;
}

