#pragma once
#include "common.h"
#include "AllNextHop.h"
using namespace std;

class Rib
{
public:
	Rib(void);
	~Rib(void);

	unsigned int BuildRibFromFile(string sFileName);
	int	withdrawLeafNode(RibTrie *pTrie);
	void CreateNewNode(RibTrie* &pTrie);
	unsigned int ConvertBinToIP(string sBinFile,string sIpFile);

	RibTrie* getRibTrie();
	RibTrieStatistic* getRibTrieStatistic();
	AllNextHop* getAllNextHopSet();

private:
	RibTrie* m_pTrie;				//RibTrie
	
	RibTrieStatistic* m_pRibTrieStat;
	AllNextHop* m_pAllNHS;
	
	
	void FreeSubTree(RibTrie *pTrie);
	bool updateAnnounce(int iNextHop,char *insert_C);
	bool updateWithdraw(char *insert_C);
	
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
	
	void prefixNumTravel(RibTrie *pTrie);
};

