#include "TestCorrect.h"

TestCorrect::TestCorrect(Rib *pRib,Fib *pFib)
{
	pRibTrie=pRib;
	pFibTrie=pFib;
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
			bool NHS_reuslt=(pFibSrc->pNextHop->iVal==pTarget->pNextHop->iVal);
			return NHS_reuslt&&pTarget->intersection;
		}
	}
}

bool TestCorrect::examineAlogrithm()
{
	Fib *fibTotal=new Fib();
	fibTotal->ConstructFromRib(pRibTrie->getRibTrie());
	fibTotal->Compress();
	bool correct=isCorrect(fibTotal->getFibRoot(),pFibTrie->getFibRoot());
	delete fibTotal;
	return correct;
}