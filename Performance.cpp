#pragma once 
#include "Performance.h"
#include <windows.h>
#include <time.h>



Performance::Performance(Rib *pRib,Fib *pFib)
{
	pRibTrie=pRib;
	pFibTrie=pFib;
	updateIndex=0;
	updateTimeUsed=0;
}


Performance::~Performance(void)
{
}

void Performance::printUseTime()
{
	double usetime=updateTimeUsed/2.648437;
	cout<<"tick:"<<updateTimeUsed<<" time:"<<usetime<<endl;
	UpdateStatistic *stat=pFibTrie->getStatistics();
	cout<<"Announce statistics:"<<endl;
	cout<<"total:"<<stat->AnnounceNum<<endl;
	cout<<"invalid:"<<stat->A_inValidNum<<endl;
	cout<<"leaf_0:"<<stat->A_leaf_0<<endl;
	cout<<"leaf_1:"<<stat->A_leaf_1<<endl;
	cout<<"leaf_2:"<<stat->A_leaf_2<<endl;
	cout<<"inherit:"<<stat->A_inherit<<endl;
	cout<<"trueGoDown:"<<stat->A_true_goDown<<endl;

	cout<<"Withdraw statistics:"<<endl;
	cout<<"total:"<<stat->WithdrawNum<<endl;
	cout<<"invalid:"<<stat->W_inValidNum<<endl;
	cout<<"leaf_0:"<<stat->W_leaf_0<<endl;
	cout<<"leaf_1:"<<stat->W_leaf_1<<endl;
	cout<<"leaf_2:"<<stat->W_leaf_2<<endl;
	cout<<"inherit:"<<stat->W_inherit<<endl;
	cout<<"trueGoDown:"<<stat->W_true_goDown<<endl;
}


void Performance::updateParameter(UpdatePara *para)
{
	bufferSet[updateIndex].nextHop=para->nextHop;
	bufferSet[updateIndex].operate=para->operate;
	strcpy(bufferSet[updateIndex].path,para->path);
	updateIndex++;
	if(updateIndex==100)
		AccUpdate();
}

void Performance::AccUpdate()
{
	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))
		return ;
	QueryPerformanceCounter(&privious); 
	for(int i=0;i<updateIndex;i++)
	{
		pRibTrie->Update(bufferSet+i);
		pFibTrie->Update(bufferSet+i,pRibTrie->getUpdate());
	}
	QueryPerformanceCounter(&privious1);
	updateTimeUsed=updateTimeUsed+privious1.QuadPart-privious.QuadPart;
	//updatetimeused=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
	updateIndex=0;
}