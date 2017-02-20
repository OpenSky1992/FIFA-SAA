#include "Rib.h"
#include <iostream>
#include <fstream>

Rib::Rib(void)
{
	update=(UpdateRib*)malloc(sizeof(UpdateRib));
	update->isLeaf=false;
	update->outNumber=0;
	update->inheritHop=DEFAULTHOP;
	update->pLastRib=NULL;
	CreateNewNode(m_pTrie);
}

Rib::~Rib(void)
{
}

void Rib::CreateNewNode(RibTrie* &pTrie)
{
	pTrie=(struct RibTrie*)malloc(RIBLEN);
	if (NULL==m_pTrie)
	{
		printf("error 10..., exit(0)\n");
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

bool Rib::updateAnnounce(int iNextHop,char *insert_C)
{
	RibTrie *insertNode=m_pTrie;
	int default_oldport=DEFAULTHOP;
	int outDeep=0;

	for (int i=0;i<(int)strlen(insert_C);i++)
	{
		if ('0'==insert_C[i])
		{
			if (NULL==insertNode->pLeftChild)
			{//turn left, if left child is empty, create new node
				RibTrie* pNewNode;
				CreateNewNode(pNewNode);
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
				RibTrie* pNewNode;
				CreateNewNode(pNewNode);
				pNewNode->pParent=insertNode;
				insertNode->pRightChild=pNewNode;
				outDeep++;
			}
			insertNode=insertNode->pRightChild;
		}
		if (insertNode->pParent!=NULL)
			if (insertNode->pParent->iNextHop!=0)
				default_oldport=insertNode->pParent->iNextHop;
	}

	update->inheritHop=default_oldport;
	update->isLeaf=false;
	if(outDeep>0)
	{
		update->a_isNewCreate=true;
		update->isLeaf=true;
	}
	else
		update->a_isNewCreate=false;
	
	if (insertNode->iNextHop==iNextHop)
		return false;
	insertNode->iNextHop=iNextHop;
	if(insertNode->pLeftChild==NULL&&insertNode->pRightChild==NULL)
	{
		update->isLeaf=true;
		update->pLastRib=NULL;//Leaf node don't need this parameter
	}
	else
		update->pLastRib=insertNode;
	return true;
}

bool Rib::updateWithdraw(char *insert_C)
{
	RibTrie *insertNode=m_pTrie;
	int default_oldport=DEFAULTHOP;
	for (int i=0;i<(int)strlen(insert_C);i++)
	{
		if ('0'==insert_C[i])
		{
			if (NULL==insertNode->pLeftChild)
				return false;
			insertNode=insertNode->pLeftChild;
		}
		else
		{
			if (NULL==insertNode->pRightChild)
				return false;
			insertNode=insertNode->pRightChild;
		}
		if (insertNode->pParent!=NULL)
			if (insertNode->pParent->iNextHop!=0)
				default_oldport=insertNode->pParent->iNextHop;
	}

	update->inheritHop=default_oldport;
	update->isLeaf=false;

	if (EMPTYHOP==insertNode->iNextHop)//invalid delete operation
		return false;
	if (insertNode->pLeftChild==NULL&&insertNode->pRightChild==NULL)
	{
		update->w_LeafOldHop=insertNode->iNextHop;
		update->outNumber=withdrawLeafNode(insertNode);
		update->isLeaf=true;
		update->pLastRib=NULL;  //Leaf node don't need this parameter
	}
	else
		update->pLastRib=insertNode;
	return true;
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
			//free(pTrie);
			break;
		}
		if(temp->iNextHop!=EMPTYHOP)
			breakwhile=2;
		if(NULL!=temp->pLeftChild&&NULL!=temp->pRightChild)
			breakwhile=1;
		if(temp->pLeftChild==pTrie)
			temp->pLeftChild=NULL;
		else
			temp->pRightChild=NULL;
		free(pTrie);
		if(breakwhile>=1)
			break;
		pTrie=temp;
		upLevel++;
		temp=temp->pParent;
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
	pTrie->iNextHop = iNextHop;
}
