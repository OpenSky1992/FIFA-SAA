#pragma once
#include "common.h"

class Fib
{
public:
	Fib(void);
	~Fib(void);

	void ConstructFromRib(RibTrie* pRibTrie);
	void Compress();
	//void Update(UpdatePara *para,UpdateRib *info);

	void updateAnnounce(AnnounceInfo *info);
	void updateWithdraw(WithdrawInfo *info);
	void CreateNewNode(FibTrie* &pTrie);

	//bool EqualNextHopSet(NextHop *pNextA,NextHop *pNextB);
	bool bitmapEqual(BitMap bm1,BitMap bm2);
	void bitmapInitial(BitMap bm1,unsigned int index);
	unsigned int bitmapSelect(BitMap bm1);
	unsigned int BitmapCapacity;
	
	FibTrie* getFibTrie();
	UpdateFibStatistic* getUpdateStatistics();
	FibTrieStatistic* getFibTrieStatistic();

private:
	//Data fib trie
	FibTrie* m_pTrie;	
	UpdateFibStatistic* m_pUpdateStat;
	FibTrieStatistic* m_pFibTrieStat;
	
	void CopyTrieFromRib(RibTrie* pSrcTrie,FibTrie* pDesTrie);
	int GetAncestorHop(FibTrie* pTrie);  //this function only be called by function compress,it is obselete for update processing

	void PassOneTwo(FibTrie *pTrie);    //this function only be called by function compress
	void PassThree(FibTrie *pTrie,unsigned int inheritHop);

	//auxilary function
	void FreeSubTree(FibTrie *FreeNode);
	//bool ifcontainFunc(int inheritHop,NextHop *ptmp);
	//void freeNextHopSet(NextHop *ptmp);
	//NextHop* CopyNextHopSet(NextHop *ptmp);
	void NextHopMerge(FibTrie *pTrie);
	unsigned int priority_select(unsigned int oldSelect,unsigned int oldInherit,BitMap ptmp);
	void prefixNumTravel(FibTrie *pTrie);

	//I sperate a old version big function into those little function
	void update_process(FibTrie *pLastFib,BitMap oldNHS);
	void update_select(FibTrie *pFib,unsigned int oldHop,unsigned int newHop);
	FibTrie* lastVisitAnnounce(char *travel,FibTrie* &insertNode,int &deep);
	FibTrie* lastVisitWithdraw(char *travel);

	
	
	//update function
	bool updateGoDown_Merge(RibTrie *pRib,FibTrie *pFib,int inheritHop);
	
	//least change select
	void NsNoChange_common_select(FibTrie *pFib,int oldHop,int newHop);  //the partition NNC are that their nexthop set don't change, this is select processing for partition NNC
	void NsNoChange_standard_select(FibTrie *pFib,int oldHop,int newHop);  //the standard model for partition NNC
	void NNC_SS_Double_search(FibTrie *pFib,int oldHop,int newHop);
	FibTrie* NNC_SS_search(FibTrie *pFib,int iNextHop);

	//withdraw delete
	FibTrie* withdrawLeaf(FibTrie *pFib,int upLevel);




	//BitMap function
	long long m_iStandardBitmap[LONGLONG_SIZE];
	std::hash_map<long long,int> m_mStandardInverse;
	
	void bitmapPrepare();
	
	bool bitmapMerge(BitMap bm1,BitMap bm2,BitMap res);
	bool bitmapExist(unsigned int index,BitMap bm1);
	void bitmapCopy(BitMap dest,BitMap src);
	
};

/*	three important recursive function 
NsNoChange_standard_select		
updateGoDown_Merge:				post-order travel
update_select:					pre-order travel
*/

