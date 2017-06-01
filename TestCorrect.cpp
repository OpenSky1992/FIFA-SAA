#include "TestCorrect.h"



void TestCorrect::updateParameter(UpdatePara *para)
{
	pUpdate->Update(para);
}

bool TestCorrect::forwardCorrect(RibTrie *pRib,int inheritRib,FibTrie *pFib,int inheritFib)
{
	//this function does not depend on nexthop set,this function only need information is that iNexPort in every FibTrie nodes
	//if we use nexthop set to compare,this function will lose its effectiveness
	int newInheritRib=inheritRib;
	int newInheritFib=inheritFib;
	if(pRib==NULL&&pFib==NULL)
		return true;
	else if(pRib==NULL&&pFib!=NULL)
	{
		if(pFib->iNewPort!=EMPTYHOP)
			newInheritFib=pFib->iNewPort;
		if(pFib->pLeftChild!=NULL||pFib->pRightChild!=NULL)
			return false;
		if(inheritRib==newInheritFib)
			return true;
		else
			return false;
	}
	else if(pRib!=NULL&&pFib==NULL)
		return false;
	else
	{
		if(pFib->iNewPort!=EMPTYHOP)
			newInheritFib=pFib->iNewPort;
		if(pRib->iNextHop!=EMPTYHOP)
			newInheritRib=pRib->iNextHop;
		if(pRib->pLeftChild==NULL&&pRib->pRightChild==NULL)
		{
			if(pFib->pLeftChild!=NULL||pFib->pRightChild!=NULL)
				return false;
			if(newInheritRib==newInheritFib)
				return true;
			else
				return false;
		}
		else
		{
			bool leftRes=forwardCorrect(pRib->pLeftChild,newInheritRib,pFib->pLeftChild,newInheritFib);
			bool rightRes=forwardCorrect(pRib->pRightChild,newInheritRib,pFib->pRightChild,newInheritFib);
			return leftRes&&rightRes;
		}
	}
}

bool TestCorrect::NHS_isCorrect(FibTrie *pFibSrc,FibTrie *pTarget)
{
	//pTarget is update one by one 
	//pFibSrc is compressed Fib trie from pRib that complete total update
	if(pTarget->is_NNC_area)
		return false;
	if(pFibSrc==NULL&&pTarget==NULL)
		return true;
	else if(pFibSrc==NULL&&pTarget!=NULL)
		return false;
	else if(pFibSrc!=NULL&&pTarget==NULL)
		return false;
	else
	{
		if(pFibSrc->pLeftChild!=NULL&&pFibSrc->pRightChild!=NULL)
		{
			bool NHS_result=pFibTrie->EqualNextHopSet(pFibSrc->pNextHop,pTarget->pNextHop);
			if(!NHS_result)
				return false;
			bool leftResult=NHS_isCorrect(pFibSrc->pLeftChild,pTarget->pLeftChild);
			bool RightResult=NHS_isCorrect(pFibSrc->pRightChild,pTarget->pRightChild);
			return leftResult&&RightResult;
		}
		else
		{
			if(pTarget->pLeftChild!=NULL||pTarget->pRightChild!=NULL)
				return false;
			return pFibSrc->pNextHop->iVal==pTarget->pNextHop->iVal;
		}
	}
}

bool TestCorrect::exammineOnebyOne()
{
	FibTrieStatistic *pFTS=pFibTrie->getFibTrieStatistic();
	int fibPrefixNum=pFTS->prefixNum;
	int fibTotalNodeNum=pFTS->totalNodeNum;

	Fib *fibTotal=new Fib();
	fibTotal->ConstructFromRib(pRibTrie->getRibTrie());
	fibTotal->Compress();
	
	pFTS=fibTotal->getFibTrieStatistic();
	int once_fibPrefixNum=pFTS->prefixNum;
	int once_fibTotalNodeNum=pFTS->totalNodeNum;
	if(fibPrefixNum!=once_fibPrefixNum)
	{
		cout<<"fib prefix number:"<<fibPrefixNum<<endl;
		cout<<"once compression fib prefix number:"<<once_fibPrefixNum<<endl;
		return false;
	}
	if(fibTotalNodeNum!=once_fibTotalNodeNum)
	{
		cout<<"fib total node number:"<<fibTotalNodeNum<<endl;
		cout<<"once compression fib total node number:"<<once_fibTotalNodeNum<<endl;
		return false;
	}
	bool NHS_correct=NHS_isCorrect(fibTotal->getFibTrie(),pFibTrie->getFibTrie());
	bool forward_correct=forwardCorrect(pRibTrie->getRibTrie(),DEFAULTHOP,pFibTrie->getFibTrie(),DEFAULTHOP);
	delete fibTotal;
	return NHS_correct&&forward_correct;
}

void TestCorrect::examineAlgorithm()
{
	cout<<endl<<"examine algorithm:"<<endl;
	cout<<"compressing one by one:"<<endl;
	pFibTrie->getFibTrieStatistic()->printInfor();

	Fib *fibTotal=new Fib();
	fibTotal->ConstructFromRib(pRibTrie->getRibTrie());
	fibTotal->Compress();
	cout<<"compression one time:"<<endl;
	fibTotal->getFibTrieStatistic()->printInfor();
	
	bool NHS_correct=NHS_isCorrect(fibTotal->getFibTrie(),pFibTrie->getFibTrie());
	delete fibTotal;
	if(NHS_correct)
		cout<<"NextHop set,is_NNC_area,intersection are correct"<<endl;
	else
		cout<<"NextHop set,is_NNC_area,intersection are wrong"<<endl;
	bool forward_correct=forwardCorrect(pRibTrie->getRibTrie(),DEFAULTHOP,pFibTrie->getFibTrie(),DEFAULTHOP);
	if(forward_correct)
		cout<<"forwarding is correct"<<endl;
	else
		cout<<"forwarding is wrong"<<endl;
}
