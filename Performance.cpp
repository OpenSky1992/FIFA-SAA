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
	cout<<"totoal update time:"<<updateTimeUsed<<endl;
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
		//pFibTrie->Update(bufferSet+i,pRibTrie->getUpdate());
	}
	QueryPerformanceCounter(&privious1);
	updateTimeUsed=updateTimeUsed+1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
	updateIndex=0;
}