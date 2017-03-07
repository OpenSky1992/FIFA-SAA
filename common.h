#pragma once

#define		HIGHTBIT			2147483648					//Binary: 10000000000000000000000000000000
#define		EMPTYHOP			0							//empty next hop is 0
#define		DEFAULTHOP			-1                          //default hop is -1
#define		UPDATE_WITHDRAW		'W'							//withdraw character
#define		UPDATE_ANNOUNCE     'A'							//announce character
#define		PREFIX_LEN			32							//the length of ip prefix
					
#include <iostream>
#include <string>

//The defination of linklist Nexhop
struct NextHop{
	NextHop*			pNext;					
	int					iVal;					//the address of Nexthop£¬0 means empty
};

//node in FibTrie
struct FibTrie
{
	FibTrie*			pParent;				//parent pointer
	FibTrie*			pLeftChild;				//point to left child
	FibTrie*			pRightChild;			//point to right child
	int					iNewPort;				//new port
	bool				intersection;			//record intersection or union
	bool				is_NNC_area;			//for update,NCC: Nexthop set No Change
	NextHop*			pNextHop;				//Nexthop set
};

//node in RibTrie
struct RibTrie
{
	RibTrie*			pParent;				//point to father node
	RibTrie*			pLeftChild;				//point to left child
	RibTrie*			pRightChild;			//point to right child
	int					iNextHop;				//Nexthop number
};


//the information of update parameter
struct UpdatePara
{
	int					nextHop;				//update parameter nexthop 
	char				path[PREFIX_LEN];		//update para
	char				operate;				
};


//return result from Rib
class UpdateRib
{
public:
	bool				valid;					//if rib change
	bool				isLeaf;					//is leaf node
	bool				isEmpty;				//is empty node
	int					inheritHop;				//inherit hop
	int					w_OldHop;				//only for withdraw leaf 
	int					w_outNumber;			//when update node is leaf node,this variable represent 
	RibTrie*			pLastRib;
};

class StatisticModule
{
public:
	virtual void reset()=0;
	virtual void printInfor()=0;
};

//the information of statistics update 
class UpdateStatistic:public StatisticModule
{
public:
	UpdateStatistic();
	virtual void reset();
	virtual void printInfor();

	int		AnnounceNum;
	int		A_inValidNum;
	int		A_leaf_0;
	int		A_leaf_1;
	int		A_leaf_2;
	int		A_inherit;
	int		A_true_goDown;
	
	int		WithdrawNum;
	int		W_inValidNum;
	int		W_leaf_0;
	int		W_leaf_1;
	int		W_leaf_2;
	int		W_inherit;
	int		W_true_goDown;
};

class RibTrieStatistic:public StatisticModule
{
public:
	RibTrieStatistic();
	virtual void reset();
	virtual void printInfor();
	int		prefixNum;			//prefix number
	int		totalNodeNum;		//total Node number
	int		diffNextHopNum;		//different Next hop number
};

class FibTrieStatistic:public StatisticModule
{
public:
	FibTrieStatistic();
	virtual void reset();
	virtual void printInfor();
	int		prefixNum;			//prefix number
	int		totalNodeNum;		//total Node number
	int		nonRouteNum;		//non-route prefix number
	int		totalNextHopNum;    //next hop number count
};