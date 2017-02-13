#include "Fib.h"
#include "Rib.h"
#include<iostream>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include<fstream>

using namespace std;


unsigned int updateFromFile(string sFileName,Rib *tRib,Fib *tFib)
{
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX  
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	char			operate_type_read;
	int 			operate_type;
	int				readlines=0;
	long			updatetimeused=0;

	long			yearmonthday=0;		//an integer to record year, month, day
	long			hourminsec=0;		//an integer to record hour, minute, second


	ifstream fin(sFileName);
	if (!fin)
	{
		printf("!!!error!!!!  no file named:%s\n",sFileName);
	}
	printf("parsing %s\n",sFileName);

	while (!fin.eof()) 
	{
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = -9;

		memset(sPrefix,0,sizeof(sPrefix));
		fin >> yearmonthday >> hourminsec >> operate_type_read >> sPrefix;//>> iNextHop;

		if('W'!=operate_type_read&&'A'!=operate_type_read)
		{
			printf("Format of update file Error, quit....\n");
			getchar();
			return 0;
		}
		int iStart=0;				//the end point of IP
		int iEnd=0;					//the end point of IP
		int iFieldIndex = 3;		
		int iLen=strlen(sPrefix);	//the length of Prefix
		int update_outsideOfRib;
		int update_inheritHop;

		
		if(iLen>0)
		{
			readlines++;
			for ( int i=0; i<iLen; i++ ){
				//extract the first 3 sub-part
				if ( sPrefix[i] == '.' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); //向左移位到高位
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' ){
					//extract the 4th sub-part
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					//extract the length of prefix
					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}

			char insert_C[50];
			memset(insert_C,0,sizeof(insert_C));
		    //insert the current node into Trie tree
			for (unsigned int yi=0; yi<iPrefixLen; yi++)
			{
				if(((lPrefix<<yi) & HIGHTBIT)==HIGHTBIT)insert_C[yi]='1';
				else insert_C[yi]='0';
			}


			LARGE_INTEGER frequence,privious,privious1;
			if(!QueryPerformanceFrequency(&frequence))return 0;
			QueryPerformanceCounter(&privious); 

			RibTrie *updateRibNode=tRib->Update(iNextHop,insert_C,operate_type_read,update_outsideOfRib,update_inheritHop);
			if (NULL!=updateRibNode)
				tFib->Update(iNextHop,insert_C,operate_type_read,updateRibNode,update_outsideOfRib,update_inheritHop);
			QueryPerformanceCounter(&privious1);
			updatetimeused+=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
		}

	}
	fin.close();
	return readlines;
}


int main()
{
	int testA;
	Rib tRib=Rib();
	Fib tFib=Fib();

	tRib.BuildRibFromFile("rib2.txt");
	tFib.CopyTrieFromRib(tRib.m_pTrie,tFib.m_pTrie);
	tFib.Compress();
	updateFromFile("updates2.txt",&tRib,&tFib);
	cin>>testA;
	return 0;
}