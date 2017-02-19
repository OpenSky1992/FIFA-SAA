#pragma once

#define RIBLEN				sizeof(struct RibTrie)		//size of the node in Trie
#define HIGHTBIT			2147483648				//Binary: 10000000000000000000000000000000
#define EMPTYHOP			0							//next hop is 0
#define DEFAULTHOP			-1                          //
#define UPDATE_WITHDRAW		'W'
#define UPDATE_ANNOUNCE     'A'

#include <string>
using namespace std;


struct RibTrie
{
	RibTrie*				pParent;				//point to father node
	RibTrie*				pLeftChild;				//point to left child
	RibTrie*				pRightChild;			//point to right child
	int						iNextHop;				//Nexthop number
};

struct UpdateRib
{
	bool isLeaf;
	bool isNewCreate;
	int outNumber;    //when update node is leaf node,this variable represent 
	int inheritHop;
	int withdrawLeafoldHop;//only for withdraw leaf 
	RibTrie* pLastRib;
};

class Rib
{
public:
	Rib(void);
	~Rib(void);
	unsigned int BuildRibFromFile(string sFileName);
	unsigned int ConvertBinToIP(string sBinFile,string sIpFile);
	bool updateAnnounce(int iNextHop,char *insert_C);
	bool updateWithdraw(char *insert_C);
	RibTrie* getRibTrie();
	UpdateRib* getUpdate();

private:
	RibTrie* m_pTrie;				//RibTrie
	UpdateRib* update;



	RibTrie* withdrawLeafNode(RibTrie *pTrie,int &goUp);
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
	void CreateNewNode(RibTrie* &pTrie);
};

