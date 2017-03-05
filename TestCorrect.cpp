#include "TestCorrect.h"

TestCorrect::TestCorrect(Rib *pRib,Fib *pFib)
{
	pRibTrie=pRib;
	pFibTrie=pFib;
}

TestCorrect::~TestCorrect(void)
{
}

void TestCorrect::updateParameter(UpdatePara *para)
{
	pRibTrie->Update(para);
	pFibTrie->Update(para,pRibTrie->getUpdate());
}

bool TestCorrect::isCorrect(FibTrie *pFibSrc,FibTrie *pTarget)
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
			bool inter_result=(pFibSrc->intersection==pTarget->intersection);
			bool is_continue=NHS_result&&inter_result;
			if(!is_continue)
				return false;
			bool leftResult=isCorrect(pFibSrc->pLeftChild,pTarget->pLeftChild);
			bool RightResult=isCorrect(pFibSrc->pRightChild,pTarget->pRightChild);
			return leftResult&&RightResult;
		}
		else
		{
			if(pTarget->pLeftChild!=NULL||pTarget->pRightChild!=NULL)
				return false;
			bool NHS_reuslt=(pFibSrc->pNextHop->iVal==pTarget->pNextHop->iVal);
			return NHS_reuslt&&pTarget->intersection;
		}
	}
}

bool TestCorrect::exammineOnebyOne()
{
	int fibPrefixNum=pFibTrie->getPrefixNum();
	int fibNonRouNum=pFibTrie->getNonRouteNum();

	Fib *fibTotal=new Fib();
	fibTotal->ConstructFromRib(pRibTrie->getRibTrie());
	fibTotal->Compress();
	
	int once_fibPrefixNum=fibTotal->getPrefixNum();
	int once_fibNonRouNum=fibTotal->getNonRouteNum();
	if(fibPrefixNum!=once_fibPrefixNum)
	{
		cout<<"fib prefix number:"<<fibPrefixNum<<endl;
		cout<<"once compression fib prefix number:"<<once_fibPrefixNum<<endl;
		return false;
	}
	if(fibNonRouNum!=once_fibNonRouNum)
	{
		cout<<"fib non-route prefix number:"<<fibNonRouNum<<endl;
		cout<<"once compression fib non-route prefix number:"<<once_fibNonRouNum<<endl;
		return false;
	}
	bool correct=isCorrect(fibTotal->getFibRoot(),pFibTrie->getFibRoot());
	delete fibTotal;
	if(correct)
		return true;
	else
		return false;
}

void TestCorrect::examineAlogrithm()
{
	int ribPrefixNum=pRibTrie->getPrefixNum();
	int fibPrefixNum=pFibTrie->getPrefixNum();
	int fibNonRouNum=pFibTrie->getNonRouteNum();
	cout<<"rib prefix number:"<<ribPrefixNum<<endl;
	cout<<"fib prefix number:"<<fibPrefixNum<<endl;
	cout<<"fib non-route prefix number:"<<fibNonRouNum<<endl;
	Fib *fibTotal=new Fib();
	fibTotal->ConstructFromRib(pRibTrie->getRibTrie());
	fibTotal->Compress();
	
	int once_fibPrefixNum=fibTotal->getPrefixNum();
	int once_fibNonRouNum=fibTotal->getNonRouteNum();
	cout<<"once compression fib prefix number:"<<once_fibPrefixNum<<endl;
	cout<<"once compression fib non-route prefix number:"<<once_fibNonRouNum<<endl;
	
	bool correct=isCorrect(fibTotal->getFibRoot(),pFibTrie->getFibRoot());
	delete fibTotal;
	if(correct)
		cout<<"algorithm(NextHop set and is_NNC_area) is correct"<<endl;
	else
		cout<<"algorithm(NextHop set and is_NNC_area) is wrong"<<endl;
}