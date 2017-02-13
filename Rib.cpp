#include "Rib.h"
#include <iostream>
#include <fstream>

Rib::Rib(void)
{
	m_pTrie = (struct RibTrie*)malloc(RIBLEN);
	if (NULL==m_pTrie)
	{
		printf("error 10..., exit(0)\n");
		exit(0);
	}
	m_pTrie->pLeftChild = NULL;
	m_pTrie->pRightChild = NULL;
	m_pTrie->iNextHop = EMPTYHOP;
}


Rib::~Rib(void)
{
}


RibTrie * Rib::Update(int iNextHop,char *insert_C,char operation_type,int &outsideOfRib,int &inheritHop)
{
	//ifWithDrawLeaf=false;
	//update_oldport=-1;
	RibTrie *ret=NULL;
	int operation;
	//locate the node that will be updated
	RibTrie *insertNode=m_pTrie;
	int default_oldport=-1;
	int default_newport=-1;
	int outDeep=0;

	bool IfNewBornNode=false;
	//look up the location of the current node
	//Attention : the update node may not exist in FIB
	for (int i=0;i<(int)strlen(insert_C);i++)
	{
		if ('0'==insert_C[i])
		{
			if (NULL==insertNode->pLeftChild)
			{//turn left, if left child is empty, create new node
				if('W'==operation_type)	
				{
					return NULL;
				}
				IfNewBornNode=true;
				RibTrie* pNewNode=(RibTrie*)malloc(sizeof(RibTrie));
				if (NULL==pNewNode)
				{
					printf("error 13..., exit(0)\n");
					exit(0);
				}
				pNewNode->iNextHop=0;
				pNewNode->pLeftChild=NULL;
				pNewNode->pRightChild=NULL;
				pNewNode->pParent=insertNode;
				insertNode->pLeftChild=pNewNode;
				outDeep++;
			}

			insertNode=insertNode->pLeftChild;
		}
		else
		{//turn right, if right child is empty, create new node
			if (NULL==insertNode->pRightChild)
			{
				if('W'==operation_type)	
				{
					return NULL;
				}
				IfNewBornNode=true;
				RibTrie* pNewNode=(RibTrie*)malloc(sizeof(RibTrie));
				if (NULL==pNewNode)
				{
					printf("error 14..., exit(0)\n");
					exit(0);
				}
				pNewNode->iNextHop=0;
				pNewNode->pLeftChild=NULL;
				pNewNode->pRightChild=NULL;
				pNewNode->pParent=insertNode;
				insertNode->pRightChild=pNewNode;
				outDeep++;
			}
			insertNode=insertNode->pRightChild;
		}
		if (insertNode->iNextHop!=0)
			default_newport=insertNode->iNextHop;
		if (insertNode->pParent!=NULL)
		{
			if (insertNode->pParent->iNextHop!=0)default_oldport=insertNode->pParent->iNextHop;
		}
	}

	outsideOfRib=outDeep;
	inheritHop=default_oldport;
	if('A'==operation_type) 
	{
		//invalid update
		if (insertNode->iNextHop==iNextHop)return NULL;
		
		//insert 
		insertNode->iNextHop=iNextHop;
		ret=insertNode;
	}

	//invalid delete operation
	else if (0==insertNode->iNextHop)
	{
		ret=NULL;
	}
	else //Withdraw
	{
		//update_oldport=default_oldport;
		if (insertNode->pLeftChild==NULL&&insertNode->pRightChild==NULL)
		{
			insertNode->iNextHop=0;
			ret=insertNode;
		}
		else
		{
			return insertNode;
		}
	}
	return ret;
}

unsigned int Rib::BuildRibFromFile(string sFileName)
{
	unsigned int	iEntryCount=0;		//the number of items from file

	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	
	ifstream fin(sFileName.c_str());
	while (!fin.eof()) {

		
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix,0,sizeof(sPrefix));
		//get prefix and corresponding nexhop
		fin >> sPrefix>> iNextHop;

		int iStart=0;				//the start point of PREFIX
		int iEnd=0;					//the start point of PREFIX
		int iFieldIndex = 3;		
		int iLen=strlen(sPrefix);	//The length of PREFIX

		
		if(iLen>0){
			iEntryCount++;
			for ( int i=0; i<iLen; i++ ){
				//get the first three sub-items
				if ( sPrefix[i] == '.' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); 
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' ){
					//get the prefix length
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					//the length of prefix
					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}
		
			AddNode(lPrefix,iPrefixLen,iNextHop);
		}
	}

	fin.close();
	return iEntryCount;
}

void Rib::AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop)
{
	//get the root of rib
	RibTrie* pTrie = m_pTrie;
	//locate every prefix in the rib tree
	for (unsigned int i=0; i<iPrefixLen; i++){
		//turn right
		if(((lPrefix<<i) & HIGHTBIT)==HIGHTBIT){
				//creat new node
			if(pTrie->pRightChild == NULL){
				RibTrie* pTChild = (struct RibTrie*)malloc(RIBLEN);
				if (NULL==pTChild)
				{
					printf("error 11..., exit(0)\n");
					exit(0);
				}

				//insert new node
				pTChild->pParent = pTrie;
				pTChild->pLeftChild = NULL;
				pTChild->pRightChild = NULL;
				pTChild->iNextHop= EMPTYHOP;
				pTrie->pRightChild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->pRightChild;

		}
		//turn left
		else{
			//if left node is empty, creat a new node
			if(pTrie->pLeftChild == NULL){
				RibTrie* pTChild = (struct RibTrie*)malloc(RIBLEN);
				if (NULL==pTChild)
				{
					printf("error 12..., exit(0)\n");
					exit(0);
				}
				//insert new node
				pTChild->pParent = pTrie;
				pTChild->pLeftChild = NULL;
				pTChild->pRightChild = NULL;
				pTChild->iNextHop= EMPTYHOP;
				pTrie->pLeftChild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->pLeftChild;
		}
	}
	pTrie->iNextHop = iNextHop;
}
