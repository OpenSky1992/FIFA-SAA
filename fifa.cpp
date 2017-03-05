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

bool isSubPrefix(char *path)
{
	char target[PREFIX_LEN]="1100101001111111111";
	int charlen=strlen(target);
	for(int i=0;i<charlen;i++)
		if(path[i]!=target[i])
			return false;
	return true;
}


void getBugPrefix(string sFileName)
{
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX  
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	char			operate_type;
	int				readlines=0;
	long			updatetimeused=0;

	long			yearmonthday=0;		//an integer to record year, month, day
	long			hourminsec=0;		//an integer to record hour, minute, second
	ifstream fin(sFileName+".txt");
	ofstream fout(sFileName+"_filter.txt");
	if (!fin)
	{
		printf("!!!error!!!!  no file named:%s\n",sFileName);
	}
	cout<<"get bug file:"<<sFileName<<endl;
	UpdatePara parameter;
	while(!fin.eof())
	{
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = -9;

		memset(sPrefix,0,sizeof(sPrefix));
		fin >> yearmonthday >> hourminsec >> operate_type>> sPrefix;//>> iNextHop;
		if(UPDATE_ANNOUNCE==operate_type)
			fin>>iNextHop;

		int iStart=0;				//the end point of IP
		int iEnd=0;					//the end point of IP
		int iFieldIndex = 3;		
		int iLen=strlen(sPrefix);	//the length of Prefix

		parameter.nextHop=iNextHop;
		parameter.operate=operate_type;


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

			memset(parameter.path,0,sizeof(parameter.path));
		    //insert the current node into Trie tree
			for (unsigned int yi=0; yi<iPrefixLen; yi++)
			{
				if(((lPrefix<<yi) & HIGHTBIT)==HIGHTBIT)
					parameter.path[yi]='1';
				else 
					parameter.path[yi]='0';
			}
			if(isSubPrefix(parameter.path))
			{
				fout<<yearmonthday<<" "<<hourminsec<<" "<<operate_type<<" "<<sPrefix;
				if(UPDATE_ANNOUNCE==operate_type)
					fout<<" "<<iNextHop;
				fout<<endl;
			}
		}
		
	}
	fin.close();
	fout<<flush;
	fout.close();
}

unsigned int updateFromFile(string sFileName,TestModule *test)
{
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX  
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	char			operate_type;
	int				readlines=0;

	long			yearmonthday=0;		//an integer to record year, month, day
	long			hourminsec=0;		//an integer to record hour, minute, second


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
			printf("Format of update file Error, quit....\n");
			getchar();
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
			//if(readlines%1000==0)
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
	cout<<readlines<<":"<<endl;
	fin.close();
	return readlines;
}

int main()
{
	int pause;
	bool ipFormat=true;
	bool getBugfile=false;

	string ribFile="rib.txt";
	string ribFileIP="rib2_ip.txt";
	string updatefile="updates2";
	
	if(!getBugfile)
	{
		Rib *tRib=new Rib();
		Fib *tFib=new Fib();
		TestCorrect *testCor=new TestCorrect(tRib,tFib);

		if(ipFormat)
			tRib->BuildRibFromFile(ribFile);
		else
		{
			tRib->ConvertBinToIP(ribFile,ribFileIP);
			tRib->BuildRibFromFile(ribFileIP);
		}
		tFib->ConstructFromRib(tRib->getRibTrie());
		tFib->Compress();
		string updateFileName=updatefile+".txt";
		updateFromFile(updateFileName,testCor);
		testCor->examineAlogrithm();
		//testCor->AccUpdate();
		//testCor->printUseTime();

		delete tRib;
		delete tFib;
		delete testCor;
		cin>>pause;
	}
	else
		getBugPrefix(updatefile);
	return 0;
}