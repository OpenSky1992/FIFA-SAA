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

	RibTrie* m_pTrie;				//RibTrie
	//int update_oldport;	
	
	unsigned int BuildRibFromFile(string sFileName);
	RibTrie* Update(int iNextHop,char *insert_C,char operation_type,int &out,int &inheritHop);
private:
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
};

