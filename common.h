#pragma once

#define		HIGHTBIT				2147483648					//Binary: 10000000000000000000000000000000
#define		EMPTYHOP				-1							//empty next hop is -1
#define		DEFAULTHOP				0                           //default hop is 0
#define		UPDATE_WITHDRAW			'W'							//withdraw character
#define		UPDATE_ANNOUNCE			'A'							//announce character
#define		PREFIX_LEN				32							//the length of ip prefix
#define		STATISTICS_PERFORMANCE  0							//0:no statistical information about update,make the program better performance,fast run.
#define		BITMAPLENGTH			1							//this number n represent the size of bitmap,how many long long can represent this bitmap
//the BITMAPLENGTH more big,this program costumes more memeory and more time to run(more slow),so we must keep this number more small (restrict condition:BITMAPLENGTH>=1)
#define		LONGLONG_SIZE			(8*sizeof(long long))

					
#include <iostream>
#include <iomanip>
#include <string>
#include <hash_map>
#include <cstdlib>


typedef long long BitMap[BITMAPLENGTH];

//node in FibTrie
struct FibTrie
{
	FibTrie*			pParent;				//parent pointer
	FibTrie*			pLeftChild;				//point to left child
	FibTrie*			pRightChild;			//point to right child
	int					iNewPort;				//new port,the number of nexthop is small,so char may be better
	bool				intersection;			//record intersection or union
	bool				is_NNC_area;			//for update,NCC: Nexthop set No Change
	//NextHop*			pNextHop;				//Nexthop set
	BitMap				pNextHop;
};

/*
struct FibTrieKKKKK
{//this class test size of struct
	FibTrie*			pParent;				//parent pointer
	FibTrie*			pLeftChild;				//point to left child
	FibTrie*			pRightChild;			//point to right child
	int					iNewPort;				//new port,the number of nexthop is small,so char may be better
	//bool				intersection;			//record intersection or union
	//bool				is_NNC_area;			//for update,NCC: Nexthop set No Change
	//NextHop*			pNextHop;				//Nexthop set
	BitMap				pNextHop;
};*/

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

	unsigned int upwardStopNum;
	unsigned int itselfStopNum;
	unsigned int untilRootNum;
	unsigned int influenceRange[PREFIX_LEN]; 

	unsigned int		A_select;
	unsigned int		A_leaf_0;
	unsigned int		A_leaf_1;
	unsigned int		A_leaf_2;
	unsigned int		A_inherit;
	unsigned int		A_true_goDown;
	
	unsigned int		W_select;
	unsigned int		W_leaf_0;
	unsigned int		W_leaf_1;
	unsigned int		W_leaf_2;
	unsigned int		W_inherit;
	unsigned int		W_true_goDown;
};

class UpdateTotalStatistic:public StatisticModule
{
public:
	UpdateTotalStatistic();
	virtual void reset();
	virtual void printInfor();

	unsigned int		announceNum;
	unsigned int		A_invalid;
	unsigned int		withdrawNum;
	unsigned int		W_invalid;
};

class RibTrieStatistic:public StatisticModule
{
public:
	RibTrieStatistic();
	virtual void reset();
	virtual void printInfor();
	unsigned int		prefixNum;			//prefix number
	unsigned int		totalNodeNum;		//total Node number
	unsigned int		diffNextHopNum;		//different Next hop number
};

class FibTrieStatistic:public StatisticModule
{
public:
	FibTrieStatistic();
	virtual void reset();
	virtual void printInfor();
	unsigned int		prefixNum;			//prefix number
	unsigned int		totalNodeNum;		//total Node number
	unsigned int		nonRouteNum;		//non-route prefix number
	//unsigned int		totalNextHopNum;    //next hop number count
};