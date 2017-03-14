#pragma once 
#include "Rib.h"
#include <iostream>
#include <fstream>

Rib::Rib(void)
{
	m_pRibTrieStat=new RibTrieStatistic();
	m_pAllNHS=new AllNextHop();
	CreateNewNode(m_pTrie);
}

Rib::~Rib(void)
{
	FreeSubTree(m_pTrie);
	delete m_pRibTrieStat;
	delete m_pAllNHS;
}

AllNextHop* Rib::getAllNextHopSet()
{
	return m_pAllNHS;
}

unsigned int Rib::ConvertBinToIP(string sBinFile,string sIpFile)
{
	char			sBinPrefix[32];		//PREFIX in binary format
	string			strIpPrefix;		//PREFIX in binary format
	unsigned int	iPrefixLen;			//the length of PREFIX
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file
	unsigned int	iEntryCount=0;		//the number of items that is transformed sucessfully
	unsigned int	if_root;

	//open the output file 
	//and prepare store routing information in IP format
	
	ofstream fout(sIpFile.c_str());

	
	ifstream fin(sBinFile.c_str());
	while (!fin.eof()) {
		iNextHop = 0;
		
		memset(sBinPrefix,0,sizeof(sBinPrefix));
		fin >>if_root;

		//empty lines are ignored
		if(if_root == 0){
			fin>>sBinPrefix>>iNextHop;
			if(iNextHop==0)
				continue;
			string strBin(sBinPrefix);
			iPrefixLen=strBin.length();
			strBin.append(32-iPrefixLen,'0');

			//transform routing infomation from binary format into IP format
			strIpPrefix="";
			for(int i=0; i<32; i+=8){				//include 4 sub parts
				int iVal=0;
				string strVal=strBin.substr(i,8);

				//turn into integer
				for(int j=7;j>=0;j--){
					if(strVal.substr(j,1)=="1"){
						iVal+=(1<<(7-j));
					}
				}

				//turn into decimal
				char buffer[5];
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"%d",iVal);
				//itoa(iVal,buffer,10);
				strVal=string(buffer);


				//IP format
				strIpPrefix += strVal;
				if(i<24){
					strIpPrefix += ".";
				}
				strVal="";
			}
			fout<<strIpPrefix<<"/"<<iPrefixLen<<" "<<iNextHop<<endl;
		}
		else if(if_root==1)
		{
			fin>>iNextHop;
			fout<<"0.0.0.0/0  "<<iNextHop<<endl;
		}
	}

	//close BinFile
	fin.close();

	//close IpFile
	fout<<flush;
	fout.close();

	return iEntryCount;
}

void Rib::CreateNewNode(RibTrie* &pTrie)
{
	pTrie=new RibTrie();
	if (NULL==m_pTrie)
	{
		std::cout<<"new object fail, exit!"<<std::endl;
		exit(0);
	}
	pTrie->pParent=NULL;
	pTrie->pLeftChild = NULL;
	pTrie->pRightChild = NULL;
	pTrie->iNextHop = EMPTYHOP;
}

RibTrie* Rib::getRibTrie()
{
	return m_pTrie;
}

RibTrieStatistic* Rib::getRibTrieStatistic()
{
	m_pRibTrieStat->reset();
	prefixNumTravel(m_pTrie);
	m_pRibTrieStat->diffNextHopNum=m_pAllNHS->getSizeOfNHS();
	return m_pRibTrieStat;
}

void Rib::prefixNumTravel(RibTrie *pTrie)
{
	if(pTrie==NULL)
		return ;
	m_pRibTrieStat->totalNodeNum++;
	if(pTrie->iNextHop!=EMPTYHOP)
	{
		m_pRibTrieStat->prefixNum++;
	}
	prefixNumTravel(pTrie->pLeftChild);
	prefixNumTravel(pTrie->pRightChild);
}

int Rib::withdrawLeafNode(RibTrie *pLeaf)
{
	int upLevel,breakwhile;
	RibTrie *pTrie=pLeaf;
	RibTrie *temp=pTrie->pParent;
	breakwhile=0;
	upLevel=0;
	while(true)
	{
		if(temp==NULL)
		{
			breakwhile=3;
			//delete pTrie;
			break;
		}
		if(NULL!=temp->pLeftChild&&NULL!=temp->pRightChild)
		{
			if(temp->pLeftChild==pTrie)
				temp->pLeftChild=NULL;
			else
				temp->pRightChild=NULL;
			delete pTrie;
			breakwhile=1;
			break;
		}
		else if(temp->iNextHop!=EMPTYHOP)
		{
			if(temp->pLeftChild==pTrie)
				temp->pLeftChild=NULL;
			else
				temp->pRightChild=NULL;
			delete pTrie;
			upLevel++;
			breakwhile=2;
			break;
		}
		else
		{
			pTrie=temp;
			upLevel++;
			temp=temp->pParent;
		}
		//this order is solid,first breakwhile=1,then breakwhile=2
		//can not change the order,because two child is more import than it has label
	}
	return upLevel;
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
	RibTrie* pTrie=m_pTrie;
	RibTrie* pTChild;
	int indexOfNextHop=m_pAllNHS->existNextHop(iNextHop);
	if(indexOfNextHop<0)
		indexOfNextHop=indexOfNextHop*-1;
	for(unsigned int i=0; i<iPrefixLen; i++){
		//turn right
		if(((lPrefix<<i) & HIGHTBIT)==HIGHTBIT){
				//creat new node
			if(pTrie->pRightChild == NULL){
				CreateNewNode(pTChild);
				pTChild->pParent = pTrie;
				pTrie->pRightChild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->pRightChild;

		}
		//turn left
		else{
			//if left node is empty, creat a new node
			if(pTrie->pLeftChild == NULL){
				CreateNewNode(pTChild);
				pTChild->pParent = pTrie;
				pTrie->pLeftChild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->pLeftChild;
		}
	}
	pTrie->iNextHop = indexOfNextHop;
}

void Rib::FreeSubTree(RibTrie *freeNode)
{
	if(NULL==freeNode)
		return ;
	FreeSubTree(freeNode->pLeftChild);
	freeNode->pLeftChild=NULL;
	FreeSubTree(freeNode->pRightChild);
	freeNode->pRightChild=NULL;
	free(freeNode);
}