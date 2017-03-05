#pragma once
#include "common.h"

class Fib
{
public:
	Fib(void);
	~Fib(void);

	void ConstructFromRib(RibTrie* pRibTrie);
	void Compress();
	void Update(UpdatePara *para,UpdateRib *info);

	bool EqualNextHopSet(NextHop *pNextA,NextHop *pNextB);
	FibTrie* getFibRoot();
	UpdateStatistic* getStatistics();

private:
	//Data fib trie
	FibTrie* m_pTrie;	
	UpdateStatistic* m_pStatics;
	
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
	void NextHopMerge(FibTrie *pTrie);
	int priority_select(int oldSelect,int oldInherit,NextHop *ptmp);

	//I sperate a old version big function into those little function
	void update_process(FibTrie *pLastFib,NextHop *oldNHS);
	void update_select(FibTrie *pFib,int oldHop,int newHop);
	FibTrie* lastVisitAnnounce(char *travel,FibTrie* &insertNode,int &deep);
	FibTrie* lastVisitWithdraw(char *travel);
	void updateAnnounce(int intNextHop,char *travel,UpdateRib *info);
	void updateWithdraw(char *travel,UpdateRib *info);
	
	
	//update function
	bool updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop);
	
	//least change select
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

