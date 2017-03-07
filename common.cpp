#pragma once
#include "common.h"
using namespace std;


UpdateStatistic::UpdateStatistic()
{
	reset();
}

void UpdateStatistic::reset()
{
	AnnounceNum=0;
	A_inherit=0;
	A_inValidNum=0;
	A_leaf_0=0;
	A_leaf_1=0;
	A_leaf_2=0;
	A_true_goDown=0;

	WithdrawNum=0;
	W_inherit=0;
	W_inValidNum=0;
	W_leaf_0=0;
	W_leaf_1=0;
	W_leaf_2=0;
	W_true_goDown=0;
}


void UpdateStatistic::printInfor()
{
	cout<<"Announce statistics:"<<endl;
	cout<<"total:       "<<AnnounceNum<<endl;
	cout<<"invalid:     "<<A_inValidNum<<endl;
	cout<<"leaf_0:      "<<A_leaf_0<<endl;
	cout<<"leaf_1:      "<<A_leaf_1<<endl;
	cout<<"leaf_2:      "<<A_leaf_2<<endl;
	cout<<"inherit:     "<<A_inherit<<endl;
	cout<<"trueGoDown:  "<<A_true_goDown<<endl;

	cout<<"Withdraw statistics:"<<endl;
	cout<<"total:       "<<WithdrawNum<<endl;
	cout<<"invalid:     "<<W_inValidNum<<endl;
	cout<<"leaf_0:      "<<W_leaf_0<<endl;
	cout<<"leaf_1:      "<<W_leaf_1<<endl;
	cout<<"leaf_2:      "<<W_leaf_2<<endl;
	cout<<"inherit:     "<<W_inherit<<endl;
	cout<<"trueGoDown:  "<<W_true_goDown<<endl;
}

RibTrieStatistic::RibTrieStatistic()
{
	reset();
}

void RibTrieStatistic::reset()
{
	prefixNum=0;
	totalNodeNum=0;
	diffNextHopNum=0;
}

void RibTrieStatistic::printInfor()
{
	cout<<"Rib:prefix number:            "<<prefixNum<<endl;
	cout<<"Rib:total node number:        "<<totalNodeNum<<endl;
	cout<<"Rib:different nexthop number: "<<diffNextHopNum<<endl;
}


FibTrieStatistic::FibTrieStatistic()
{
	reset();
}

void FibTrieStatistic::reset()
{
	prefixNum=0;
	totalNodeNum=0;
	nonRouteNum=0;
	totalNextHopNum=0;
}

void FibTrieStatistic::printInfor()
{
	cout<<"Fib:prefix number:            "<<prefixNum<<endl;
	cout<<"Fib:total node number:        "<<totalNodeNum<<endl;
	cout<<"Fib:non-route prefix number:  "<<nonRouteNum<<endl;
	cout<<"Fib:total nexthop number:     "<<totalNextHopNum<<endl;
}