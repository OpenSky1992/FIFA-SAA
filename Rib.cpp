#include "Rib.h"
#include <iostream>
#include <fstream>

Rib::Rib(void)
{
	update=(UpdateRib*)malloc(sizeof(UpdateRib));
	m_pTrie = (struct RibTrie*)malloc(RIBLEN);
	if (NULL==m_pTrie)
	{
		printf("error 10..., exit(0)\n");
		exit(0);
	}
	m_pTrie->pLeftChild = NULL;
	m_pTrie->pRightChild = NULL;
	m_pTrie->iNextHop = EMPTYHOP;
	update->isLeaf=false;
	update->outNumber=0;
	update->inheritHop=DEFAULTHOP;
	update->pLastRib=NULL;
}


Rib::~Rib(void)
{
}

RibTrie* Rib::getRibTrie()
{
	return m_pTrie;
}

UpdateRib* Rib::getUpdate()
{
	return update;
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

RibTrie * Rib::Update(int iNextHop,char *insert_C,char operation_type)
{
	RibTrie *insertNode=m_pTrie;
	int default_oldport=-1;
	int default_newport=-1;

	//look up the location of the current node
	//Attention : the update node may not exist in FIB
	for (int i=0;i<(int)strlen(insert_C);i++)
	{
		if ('0'==insert_C[i])
		{
			if (NULL==insertNode->pLeftChild)
			{//turn left, if left child is empty, create new node
				if(UPDATE_WITHDRAW==operation_type)	
					return NULL;
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
			}
			insertNode=insertNode->pLeftChild;
		}
		else
		{//turn right, if right child is empty, create new node
			if (NULL==insertNode->pRightChild)
			{
				if(UPDATE_WITHDRAW==operation_type)	
					return NULL;
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

	update->isLeaf=false;
	update->inheritHop=default_oldport;
	update->withdrawLeafoldHop=EMPTYHOP;
	if(UPDATE_ANNOUNCE==operation_type) 
	{
		if (insertNode->iNextHop==iNextHop)//invalid update
			return NULL;
		insertNode->iNextHop=iNextHop;//insert 
		if(insertNode->pLeftChild==NULL&&insertNode->pRightChild==NULL)
			update->isLeaf=true;
		return insertNode;
	}
	else if (EMPTYHOP==insertNode->iNextHop)//invalid delete operation
	{
		return NULL;
	}
	else //Withdraw
	{
		if (insertNode->pLeftChild==NULL&&insertNode->pRightChild==NULL)
		{
			int outDeep;
			update->withdrawLeafoldHop=insertNode->iNextHop;
			insertNode=withdrawLeafNode(insertNode,outDeep);
			update->isLeaf=true;
			update->outNumber=outDeep;
			return insertNode;
		}
		else
		{
			return insertNode;
		}
	}
}

RibTrie* Rib::withdrawLeafNode(RibTrie *pLeaf,int &goUp)
{
	int upLevel,breakwhile;
	RibTrie *pTrie=pLeaf;
	RibTrie *temp=pTrie->pParent;
	breakwhile=1;
	upLevel=0;
	if(temp!=NULL)
	{
		while(temp->pLeftChild==NULL||temp->pRightChild==NULL)
		{
			if(temp->iNextHop!=EMPTYHOP)
			{
				breakwhile=2;
				break;
			}
			if(temp->pLeftChild==NULL)
				temp->pRightChild=NULL;
			else
				temp->pLeftChild=NULL;
			free(pTrie);
			pTrie=temp;
			upLevel++;
			temp=temp->pParent;
			if(temp==NULL)
			{
				breakwhile=3;
				break;
			}
		}
	}
	else
		breakwhile=4;
	goUp=upLevel;
	return temp;
	//switch(breakwhile)
	//{
	//case 1:
	//case 2:
	//case 3:goUp=upLevel;return temp;break;
	//case 4:break;
	//default:goUp=0;return NULL;break;
	//}
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
