#pragma once 
#include "Fib.h"

#include <windows.h>
#include <time.h>



Fib::Fib(void)
{
	CreateNewNode(m_pTrie);
	m_pUpdateStat=new UpdateFibStatistic();
	m_pFibTrieStat=new FibTrieStatistic();
	bitmapPrepare();
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

	if(pSrcTrie->iNextHop!=EMPTYHOP)
		bitmapInitial(pTrie->pNextHop,pSrcTrie->iNextHop);
	pTrie->iNewPort=pSrcTrie->iNextHop;
}

//creat a new FIB trie node
void Fib::CreateNewNode(FibTrie* &pTrie)
{
	
	pTrie= new FibTrie();
	if (NULL==pTrie)
	{
		std::cout<<"new object fail, exit!"<<std::endl;
		exit(0);
	}

	pTrie->pParent = NULL;
	pTrie->pLeftChild = NULL;
	pTrie->pRightChild = NULL;
	pTrie->iNewPort = EMPTYHOP;
	pTrie->intersection=false;
	pTrie->is_NNC_area=false;

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
		if(pTrie->iNewPort==EMPTYHOP)
			bitmapInitial(pNTrie->pNextHop,GetAncestorHop(pTrie));
		else	
			bitmapInitial(pNTrie->pNextHop,pTrie->iNewPort);
		pNTrie->intersection=true;
		pTrie->pLeftChild=pNTrie;

		NextHopMerge(pTrie);
	}
	else if(pTrie->pLeftChild!=NULL && pTrie->pRightChild==NULL)
	{
		PassOneTwo(pTrie->pLeftChild);
		
		CreateNewNode(pNTrie);
		pNTrie->pParent=pTrie;
		if(pTrie->iNewPort==EMPTYHOP)
			bitmapInitial(pNTrie->pNextHop,GetAncestorHop(pTrie));
		else		
			bitmapInitial(pNTrie->pNextHop,pTrie->iNewPort);
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
	pTrie->intersection=bitmapMerge(pTrie->pLeftChild->pNextHop,pTrie->pRightChild->pNextHop,pTrie->pNextHop);
}

void Fib::PassThree(FibTrie *pTrie,unsigned int inheritHop)
{
	unsigned int newInherit=inheritHop;
	bool clear=true;
	if (pTrie==NULL)
		return;

	if(pTrie->intersection)
	{
		if(!bitmapExist(inheritHop,pTrie->pNextHop))
		{
			newInherit=bitmapSelect(pTrie->pNextHop);
			pTrie->iNewPort=newInherit;
			clear=false;
		}
	}
	if(clear)
		pTrie->iNewPort=EMPTYHOP;

	PassThree(pTrie->pLeftChild, newInherit);
	PassThree(pTrie->pRightChild, newInherit);
}

void Fib::Compress()
{
	if(m_pTrie->iNewPort==EMPTYHOP)
		m_pTrie->iNewPort=DEFAULTHOP;
	
	//LARGE_INTEGER frequence,privious,privious1;
	//if(!QueryPerformanceFrequency(&frequence))	return ;
	//QueryPerformanceCounter(&privious); 
	PassOneTwo(m_pTrie);
	//QueryPerformanceCounter(&privious1);
	//printf("pass one and two:             %d microsecond\n",1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart);

	PassThree(m_pTrie,BitmapCapacity);  //
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
	delete FreeNode;
}

