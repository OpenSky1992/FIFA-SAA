#pragma once

#define		HIGHTBIT				2147483648					//Binary: 10000000000000000000000000000000
#define		EMPTYHOP				0							//empty next hop is 0
#define		DEFAULTHOP				-1                          //default hop is -1
#define		UPDATE_WITHDRAW			'W'							//withdraw character
#define		UPDATE_ANNOUNCE			'A'							//announce character
#define		PREFIX_LEN				32							//the length of ip prefix
#define		STATISTICS_PERFORMANCE  1							//0:no statist information about update
																//make the program better performance,fast run.
#define     PERFORMANCE_BUFFER_SIZE 100
					
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>

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
	char				path[PREFIX_LEN+1];		//update para
	char				operate;				
};


//return result from Rib
class WithdrawInfo
{
public:
	bool				isLeaf;					//is leaf node
	bool				isEmpty;				//only for non-terminal node
	int					inheritHop;				//inherit hop
	int					oldHop;					//old hop
	int					outNumber;				//only for terminal node
	RibTrie*			pLastRib;				//only for non-terminal node
	FibTrie*			pLastFib;
};

class AnnounceInfo
{
public:
	bool				isLeaf;					//is leaf node
	bool				isEmpty;				//only for non-terminal node
	int					inheritHop;				//inherit hop
	int					outNumber;				//
	int					iNextHop;				//
	RibTrie*			pLastRib;				//only for non-terminal node
	FibTrie*			pLastFib;
	FibTrie*			pInsertFib;
};

class StatisticModule
{
public:
	virtual void reset()=0;
	virtual void printInfor()=0;
};

//the information of statistics update 
class UpdateFibStatistic:public StatisticModule
{
public:
	UpdateFibStatistic();
	virtual void reset();
	virtual void printInfor();

	int		A_select;
	int		A_leaf_0;
	int		A_leaf_1;
	int		A_leaf_2;
	int		A_inherit;
	int		A_true_goDown;
	
	int		W_select;
	int		W_leaf_0;
	int		W_leaf_1;
	int		W_leaf_2;
	int		W_inherit;
	int		W_true_goDown;
};

class UpdateTotalStatistic:public StatisticModule
{
public:
	UpdateTotalStatistic();
	virtual void reset();
	virtual void printInfor();

	int		announceNum;
	int		A_invalid;
	int		withdrawNum;
	int		W_invalid;
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
