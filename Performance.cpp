#include "Performance.h"


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
	struct timeval start,end;
	gettimeofday(&start,NULL);
	for(int i=0;i<updateIndex;i++)
	{
		pUpdate->Update(bufferSet+i);
	}
	gettimeofday(&end,NULL);
	
	updateTimeUsed=updateTimeUsed+(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
	updateIndex=0;
}


