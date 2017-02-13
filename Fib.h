#pragma once

#define		FIBLEN			sizeof(struct FibTrie)		//size of each node in FibTrie
#define		NEXTHOPLEN		sizeof(struct NextHop)		//size of struct Nexthop

#include "Rib.h"
using namespace std;

//The defination of linklist Nexhop
struct NextHop{
	NextHop*			pNext;				
	int		iVal;							//the address of Nexthop£¬0 means empty
};

//node in FibTrie
struct FibTrie
{
	FibTrie*	pParent;					//parent pointer
	FibTrie*	pLeftChild;					//point to left child
	FibTrie*	pRightChild;				//point to right child
	int			iNewPort;					//new port
	bool		intersection;				//intersection and union
	struct NextHop* pNextHop;							//Nexthop set
};

class Fib
{
public:
	FibTrie* m_pTrie;	

	Fib(void);
	~Fib(void);
	void Update(int iNextHop,char *insert_C,char operation_type,RibTrie* pLastRib,int outRibDeep,int inheritHop);	
	void CopyTrieFromRib(RibTrie* pSrcTrie,FibTrie* pDesTrie);
	void Compress();

private:
	
	int GetAncestorHop(FibTrie* pTrie);  //this function only be called by function compress,it is obselete for update processing
	void CreateNewNode(FibTrie* &pTrie);
	void PassOneTwo(FibTrie *pTrie);    //this function only be called by function compress
	void PassThree(FibTrie *pTrie,int inheritHop);

	//auxilary function
	void FreeSubTree(FibTrie *FreeNode);
	bool ifcontainFunc(int inheritHop,NextHop *ptmp);
	void freeNextHopSet(NextHop *ptmp);
	NextHop* CopyNextHopSet(NextHop *ptmp);
	bool EqualNextHopSet(NextHop *pNextA,NextHop *pNextB);
	void NextHopMerge(FibTrie *pTrie);

	FibTrie* LastVisitNode(int iNextHop,char *insert_C,int &outNumber);
	void UpdateGoUp();
	void updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop);
	void NsNoChange_common_select(FibTrie *pFib,int oldHop,int newHop);  //the partition NNC are that their nexthop set don't change, this is select processing for partition NNC
	void NsNoChange_standard_select(FibTrie *pFib,int oldHop,int newHop);  //the standard model for partition NNC
	void NNC_SS_Double_search(FibTrie *pFib,int oldHop,int newHop);
	FibTrie* NNC_SS_search(FibTrie *pFib,int iNextHop);
};

