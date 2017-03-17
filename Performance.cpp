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
	pUpdate=new UpdateTravel(pRib,pFib);
}


Performance::~Performance(void)
{
	delete pUpdate;
}

void Performance::printUseTime()
{
	cout<<"totoal update time:"<<updateTimeUsed<<endl;
}


void Performance::updateParameter(UpdatePara *para)
{
	if(updateIndex==PERFORMANCE_BUFFER_SIZE)
		AccUpdate();
	bufferSet[updateIndex].nextHop=para->nextHop;
	bufferSet[updateIndex].operate=para->operate;
	strcpy(bufferSet[updateIndex].path,para->path);
	updateIndex++;
}

void Performance::AccUpdate()
{
	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))
		return ;
	QueryPerformanceCounter(&privious); 
	for(int i=0;i<updateIndex;i++)
	{
		pUpdate->Update(bufferSet+i);
	}
	QueryPerformanceCounter(&privious1);
	updateTimeUsed=updateTimeUsed+1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
	updateIndex=0;
}


