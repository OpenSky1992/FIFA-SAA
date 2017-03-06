#pragma once
#include "common.h"
using namespace std;


UpdateStatistic::UpdateStatistic()
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
	cout<<"total:		"<<AnnounceNum<<endl;
	cout<<"invalid:		"<<A_inValidNum<<endl;
	cout<<"leaf_0:		"<<A_leaf_0<<endl;
	cout<<"leaf_1:		"<<A_leaf_1<<endl;
	cout<<"leaf_2:		"<<A_leaf_2<<endl;
	cout<<"inherit:		"<<A_inherit<<endl;
	cout<<"trueGoDown:	"<<A_true_goDown<<endl;

	cout<<"Withdraw statistics:"<<endl;
	cout<<"total:		"<<WithdrawNum<<endl;
	cout<<"invalid:		"<<W_inValidNum<<endl;
	cout<<"leaf_0:		"<<W_leaf_0<<endl;
	cout<<"leaf_1:		"<<W_leaf_1<<endl;
	cout<<"leaf_2:		"<<W_leaf_2<<endl;
	cout<<"inherit:		"<<W_inherit<<endl;
	cout<<"trueGoDown:	"<<W_true_goDown<<endl;
}