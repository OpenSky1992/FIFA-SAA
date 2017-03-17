#include "common.h"
using namespace std;



UpdateFibStatistic::UpdateFibStatistic()
{
	reset();
}

void UpdateFibStatistic::reset()
{
	A_inherit=0;
	A_select=0;
	A_leaf_0=0;
	A_leaf_1=0;
	A_leaf_2=0;
	A_true_goDown=0;

	W_inherit=0;
	W_select=0;
	W_leaf_0=0;
	W_leaf_1=0;
	W_leaf_2=0;
	W_true_goDown=0;

	upwardStopNum=0;
	itselfStopNum=0;
	untilRootNum=0;
	for(int i=0;i<PREFIX_LEN;i++)
		influenceRange[i]=0;
}

void UpdateFibStatistic::printInfor()
{
	cout<<"Fib announce statistics:"<<endl;
	cout<<"select:      "<<A_select<<endl;
	cout<<"leaf_0:      "<<A_leaf_0<<endl;
	cout<<"leaf_1:      "<<A_leaf_1<<endl;
	cout<<"leaf_2:      "<<A_leaf_2<<endl;
	cout<<"inherit:     "<<A_inherit<<endl;
	cout<<"trueGoDown:  "<<A_true_goDown<<endl;

	cout<<"Fib withdraw statistics:"<<endl;
	cout<<"select:      "<<W_select<<endl;
	cout<<"leaf_0:      "<<W_leaf_0<<endl;
	cout<<"leaf_1:      "<<W_leaf_1<<endl;
	cout<<"leaf_2:      "<<W_leaf_2<<endl;
	cout<<"inherit:     "<<W_inherit<<endl;
	cout<<"trueGoDown:  "<<W_true_goDown<<endl;

	cout<<"upwardstop:  "<<upwardStopNum<<endl;
	cout<<"itselfstop:  "<<itselfStopNum<<endl;
	cout<<"untilRoot:   "<<untilRootNum<<endl;

	for(int i=0;i<PREFIX_LEN;i++)
		if(influenceRange[i]!=0)
			cout<<"influence deep "<<setw(2)<<i<<":  "<<influenceRange[i]<<endl;
}

UpdateTotalStatistic::UpdateTotalStatistic()
{
	reset();
}

void UpdateTotalStatistic::reset()
{
	announceNum=0;
	A_invalid=0;
	withdrawNum=0;
	W_invalid=0;
}

void UpdateTotalStatistic::printInfor()
{
	cout<<"announce number:   "<<announceNum<<endl;
	cout<<"announce invalid:  "<<A_invalid<<endl;
	cout<<"withdraw number:   "<<withdrawNum<<endl;
	cout<<"withdraw invalid:  "<<W_invalid<<endl;
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
	//totalNextHopNum=0;
}

void FibTrieStatistic::printInfor()
{
	cout<<"Fib:prefix number:            "<<prefixNum<<endl;
	cout<<"Fib:total node number:        "<<totalNodeNum<<endl;
	cout<<"Fib:non-route prefix number:  "<<nonRouteNum<<endl;
	//cout<<"Fib:total nexthop number:     "<<totalNextHopNum<<endl;
}