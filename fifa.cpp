#pragma once
#include "Fib.h"
#include "Rib.h"
#include "TestCorrect.h"
#include "Performance.h"

#include <windows.h>
#include <time.h>
#include <conio.h>
#include <fstream>

using namespace std;

unsigned int updateFromFile(string sFileName,TestModule *test)
{
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX  
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	char			operate_type;
	int				readlines=0;

	string			yearmonthday;		//an integer to record year, month, day
	string			hourminsec;		//an integer to record hour, minute, second


	ifstream fin(sFileName);
	if (!fin)
	{
		printf("!!!error!!!!  no file named:%s\n",sFileName);
	}
	cout<<"parsing file:"<<sFileName<<endl;
	UpdatePara parameter;

	while (!fin.eof()) 
	{
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = -9;

		memset(sPrefix,0,sizeof(sPrefix));
		fin >> yearmonthday >> hourminsec >> operate_type>> sPrefix;//>> iNextHop;

		if(UPDATE_ANNOUNCE==operate_type)
			fin>>iNextHop;
		if(UPDATE_WITHDRAW!=operate_type&&UPDATE_ANNOUNCE!=operate_type)
		{
			cout<<"Format of update file Error:"<<readlines<<endl;
			return 0;
		}
		parameter.nextHop=iNextHop;
		parameter.operate=operate_type;

		int iStart=0;				//the end point of IP
		int iEnd=0;					//the end point of IP
		int iFieldIndex = 3;		
		int iLen=strlen(sPrefix);	//the length of Prefix

		if(iLen>0)
		{
			readlines++;
			for ( int i=0; i<iLen; i++ ){
				//extract the first 3 sub-part
				if ( sPrefix[i] == '.' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); 
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
			memset(parameter.path,0,sizeof(parameter.path));
		    //insert the current node into Trie tree
			for (unsigned int yi=0; yi<iPrefixLen; yi++)
			{
				if(((lPrefix<<yi) & HIGHTBIT)==HIGHTBIT)
					parameter.path[yi]='1';
				else 
					parameter.path[yi]='0';
			}
			test->updateParameter(&parameter);

			if(readlines%1000000==0)
				cout<<readlines/1000000<<endl;

			//if(readlines==9664)
			//{
			//	if(!test->exammineOnebyOne())
			//	{
			//		cout<<readlines<<":wrong"<<endl;
			//		return readlines;
			//	}
			//	else
			//	{
			//		cout<<readlines<<":correct"<<endl;	
			//	}
			//}
			//cout<<readlines<<endl;
		}
	}
	//double updatetimeusedtime=updatetimeused/2.648437;
	cout<<"update number:"<<readlines<<endl;
	fin.close();
	return readlines;
}

int main()
{
	int pause;
	bool ipFormat=true;
	string ribFile="getFromRib.txt";
	string ribFileIP="rib2_ip.txt";
	string updatefile="update_big";
	
	

	Rib *tRib=new Rib();
	Fib *tFib=new Fib();
	Performance *testCor=new Performance(tRib,tFib);

	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))return 0;

	if(ipFormat)
	{
		cout<<"read from file:"<<ribFile<<endl;
		QueryPerformanceCounter(&privious); 
		tRib->BuildRibFromFile(ribFile);
		QueryPerformanceCounter(&privious1);
		printf("read from file time:          %d microsecond\n",1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart);
	}
	else
	{
		tRib->ConvertBinToIP(ribFile,ribFileIP);
		tRib->BuildRibFromFile(ribFileIP);
	}
	
	tRib->getRibTrieStatistic()->printInfor();
	cout<<"construct from rib..."<<endl;
	tFib->ConstructFromRib(tRib->getRibTrie());


	QueryPerformanceCounter(&privious); 
	tFib->Compress();
	QueryPerformanceCounter(&privious1);
	printf("Compress Time Consumption:    %d microsecond\n",1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart);
		
		

	string updateFileName=updatefile+".txt";
	updateFromFile(updateFileName,testCor);
	
	testCor->AccUpdate();
	testCor->printUseTime();
	//testCor->examineAlgorithm();
	
	cout<<endl;
	tRib->getRibTrieStatistic()->printInfor();
	tFib->getFibTrieStatistic()->printInfor();
	cout<<endl;
	tFib->getUpdateStatistics()->printInfor();

	delete tRib;
	delete tFib;
	delete testCor;
	

	return 0;
}