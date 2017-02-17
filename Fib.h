#pragma once

#define		FIBLEN			sizeof(struct FibTrie)		//size of each node in FibTrie
#define		NEXTHOPLEN		sizeof(struct NextHop)		//size of struct Nexthop
#define     PREFIX_LEN		32							//the length of ip prefix

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
	bool		intersection;				//record intersection or union
	bool		is_NNC_area;				//for update,NCC: Nexthop set No Change
	struct NextHop* pNextHop;				//Nexthop set
};

struct UpdatePara
{
	int nextHop;
	char path[PREFIX_LEN];
	char operate;
	FibTrie *pLastFib;
};


class Fib
{
public:
	Fib(void);
	~Fib(void);
	void Update(UpdatePara *para,UpdateRib *info);	
	void ConstructFromRib(RibTrie* pRibTrie);
	void Compress();



private:
	//Data fib trie
	FibTrie* m_pTrie;	

	
	void CopyTrieFromRib(RibTrie* pSrcTrie,FibTrie* pDesTrie);
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
	int priority_select(int oldSelect,int oldInherit,NextHop *ptmp);

	//I sperate a old version big function into those little function
	void update_process(FibTrie *pLastFib,NextHop *oldNHS);
	bool update_NextHopSet(UpdatePara *para,UpdateRib *info);
	void update_select(FibTrie *pFib,int oldHop,int newHop);
	
	//update function
	NextHop *LastVisitNode(UpdatePara *para,UpdateRib *info);
	bool updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop);
	void NsNoChange_common_select(FibTrie *pFib,int oldHop,int newHop);  //the partition NNC are that their nexthop set don't change, this is select processing for partition NNC
	void NsNoChange_standard_select(FibTrie *pFib,int oldHop,int newHop);  //the standard model for partition NNC
	void NNC_SS_Double_search(FibTrie *pFib,int oldHop,int newHop);
	FibTrie* NNC_SS_search(FibTrie *pFib,int iNextHop);

	//withdraw delete
	FibTrie* withdrawLeaf(FibTrie *pFib,int upLevel);
};

/*	three important recursive function 
NsNoChange_standard_select		
updateGoDown_Merge:				post-order travel
update_select:					pre-order travel
*/

