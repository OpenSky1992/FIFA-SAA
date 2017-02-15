#pragma once

#define RIBLEN				sizeof(struct RibTrie)		//size of the node in Trie
#define HIGHTBIT				2147483648				//Binary: 10000000000000000000000000000000
#define EMPTYHOP			0							//next hop is 0
#define DEFAULTHOP			-1                          //

#include <string>
using namespace std;


struct RibTrie
{
	RibTrie*				pParent;				//point to father node
	RibTrie*				pLeftChild;				//point to left child
	RibTrie*				pRightChild;			//point to right child
	int						iNextHop;				//Nexthop number
};

class Rib
{
public:
	Rib(void);
	~Rib(void);
	unsigned int BuildRibFromFile(string sFileName);
	unsigned int ConvertBinToIP(string sBinFile,string sIpFile);
	RibTrie* Update(int iNextHop,char *insert_C,char operation_type,int &out,int &inheritHop);
	RibTrie* getRibTrie();


private:
	RibTrie* m_pTrie;				//RibTrie

	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
};

