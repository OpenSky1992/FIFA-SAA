#include "Fib.h"

/*
run speed compare:
first:		bitmapExist
second:		bitmapSelect,	bitmapEqual
third:		bitmapCopy,		bitmapInitial
forth:		bitMerge
*/
bool Fib::bitmapMerge(BitMap bm1,BitMap bm2,BitMap res)
{
	bool intersection=false;
	for(int i=0;i<BITMAPLENGTH;i++)
		res[i]=bm1[i] & bm2[i];
	for(int i=0;i<BITMAPLENGTH;i++)
		if(res[i]!=0)
			return true;
	for(int i=0;i<BITMAPLENGTH;i++)
		res[i]=bm1[i] | bm2[i];
	return false;
}

bool Fib::bitmapEqual(BitMap bm1,BitMap bm2)
{
	for(int i=0;i<BITMAPLENGTH;i++)
		if(bm1[i]!=bm2[i])
			return false;
	return true;
}

bool Fib::bitmapExist(unsigned int index,BitMap bm1)
{
	if(index>=BitmapCapacity)
		return false;
	int section=index/LONGLONG_SIZE;
	int seciont_index=index%LONGLONG_SIZE;
	long long standard=m_iStandardBitmap[seciont_index];
	if((bm1[section] & standard)==standard)
		return true;
	else
		return false;
}

void Fib::bitmapPrepare()
{
	BitmapCapacity=BITMAPLENGTH*LONGLONG_SIZE;
	long long numberOne=1,temp;
	for(int i=0;i<LONGLONG_SIZE;i++)
	{
		temp=numberOne<<i;
		m_iStandardBitmap[i]=temp;
		m_mStandardInverse[temp]=i;
	}
}

void Fib::bitmapInitial(BitMap bm1,unsigned int index)
{
	int section=index/LONGLONG_SIZE;
	int section_index=index%LONGLONG_SIZE;
	long long temp=m_iStandardBitmap[section_index];
	for(int i=0;i<BITMAPLENGTH;i++)
	{
		if(i==section)
			bm1[i]=temp;
		else
			bm1[i]=0;
	}
}

unsigned int Fib::bitmapSelect(BitMap bm1)
{
	int section=0;
	for(int i=0;i<BITMAPLENGTH;i++)
	{
		if(bm1[i]==0)
		{
			section++;
			continue;
		}
		else
		{
			std::hash_map<long long,int>::iterator it=m_mStandardInverse.find(bm1[i] & (-bm1[i]));
			return section*LONGLONG_SIZE+(it->second);
		}
	}
}

void Fib::bitmapCopy(BitMap dest,BitMap src)
{
	for(int i=0;i<BITMAPLENGTH;i++)
		dest[i]=src[i];
}