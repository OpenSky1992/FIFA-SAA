#include "Fib.h"
#include "Rib.h"
#include "TestCorrect.h"
#include "Performance.h"

#include <fstream>
#include<sys/time.h>

using namespace std;

unsigned int updateFromFile(string sFileName,Performance *test)
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
		cout<<"!!!error!!!!  no file named:"<<sFileName<<endl;
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
			
			/*if(readlines>2020)
			{
				cout<<"wrong line numbers:"<<readlines<<endl;
				cout<<parameter.path<<endl;
			}*/
			
			test->updateParameter(&parameter);

			if(readlines%1000000==0)
				cout<<"updates line numbers:"<<readlines<<endl;
			

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
	cout<<"update number:"<<readlines<<endl;
	fin.close();
	return readlines;
}

int main()
{
	bool ipFormat=true;
	string directory="../Data/";
	string ribFile;
	string ribFileIP=directory+"rib2_ip.txt";
	string updatefile;
	
	/*
	//cout<<"size of FibTrieKKKKK:"<<sizeof(FibTrieKKKKK)<<endl;
	cout<<"size of int:"<<sizeof(int)<<endl;
	cout<<"size of long:"<<sizeof(long)<<endl;
	cout<<"size of long long:"<<sizeof(long long)<<endl;
	cout<<"size of pointer:"<<sizeof(TestModule *)<<endl;
	cout<<"size of RibTrie:"<<sizeof(RibTrie )<<endl;
	cout<<"size of FibTrie:"<<sizeof(FibTrie )<<endl;
	cout<<"size of UpdateTotalStatistic:"<<sizeof(UpdateTotalStatistic)<<endl;*/
	
	cout<<"Please input original Rib file:";
	cin>>ribFile;
	ribFile=directory+ribFile+".txt";
	
	Rib *tRib=new Rib();
	Fib *tFib=new Fib();
	Performance *testCor=new Performance(tRib,tFib);

	struct  timeval  start,end;

	if(ipFormat)
	{
		cout<<"read from file:"<<ribFile<<endl;
		gettimeofday(&start,NULL); 
		tRib->BuildRibFromFile(ribFile);
		gettimeofday(&end,NULL);
		cout<<"read from file time:"<<(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec)<<" microsecond"<<endl;
	}
	else
	{
		tRib->ConvertBinToIP(ribFile,ribFileIP);
		tRib->BuildRibFromFile(ribFileIP);
	}
	//tRib->getRibTrieStatistic()->printInfor();
	cout<<"construct from rib..."<<endl;
	tFib->ConstructFromRib(tRib->getRibTrie());


	gettimeofday(&start,NULL); 
	tFib->Compress();
	gettimeofday(&end,NULL); 
	cout<<"compressing time:"<<(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec)<<" microsecond"<<endl;
		
	cout<<endl<<"Please input update file:";	
	cin>>updatefile;
	//tFib->getFibTrieStatistic()->printInfor();
	string updateFileName=directory+updatefile+".txt";
	updateFromFile(updateFileName,testCor);
	
	testCor->AccUpdate();
	testCor->printUseTime();
	//testCor->examineAlgorithm();
	
	cout<<endl;
	tRib->getRibTrieStatistic()->printInfor();
	tFib->getFibTrieStatistic()->printInfor();
	cout<<endl;
	#if STATISTICS_PERFORMANCE
		testCor->printInfor();
		tFib->getUpdateStatistics()->printInfor();
	#endif
	
	delete testCor;
	delete tRib;
	delete tFib;
	

	return 0;
}
