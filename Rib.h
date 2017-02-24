#pragma once
#include "common.h"
using namespace std;

class Rib
{
public:
	Rib(void);
	~Rib(void);
	void Update(UpdatePara *para);
	unsigned int BuildRibFromFile(string sFileName);
	unsigned int ConvertBinToIP(string sBinFile,string sIpFile);

	RibTrie* getRibTrie();
	UpdateRib* getUpdate();

private:
	RibTrie* m_pTrie;				//RibTrie
	UpdateRib* update;

	void FreeSubTree(RibTrie *pTrie);
	bool updateAnnounce(int iNextHop,char *insert_C);
	bool updateWithdraw(char *insert_C);
	int withdrawLeafNode(RibTrie *pTrie);
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
	void CreateNewNode(RibTrie* &pTrie);
};

