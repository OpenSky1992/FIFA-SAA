#include<iostream>
#include<fstream>
#include<string>
#include<stdlib.h>

using namespace std;
class DateTime
{
public:
	DateTime(){}
	~DateTime(){}
	void setDateTime(string sDate,string sTime)
	{
		month =atoi(sDate.substr(0,2).c_str());
		day   =atoi(sDate.substr(3,2).c_str());
		year  =atoi(sDate.substr(6,2).c_str());
		hour  =atoi(sTime.substr(0,2).c_str());
		minute=atoi(sTime.substr(3,2).c_str());
		second=atoi(sTime.substr(6,2).c_str());
	}
	void setDateTime(const DateTime &destdate)
	{
		year=destdate.year;
		month=destdate.month;
		day=destdate.day;
		hour=destdate.hour;
		minute=destdate.minute;
		second=destdate.second;
	}

	bool moreFresh(const DateTime &destdate)
	{
		if(destdate.year<year)
			return true;
		else if(destdate.year==year)
		{
			if(destdate.month<month)
				return true;
			else if(destdate.month==month)
			{
				if(destdate.day<day)
					return true;
				else if(destdate.day==day)
				{
					if(destdate.hour<hour)
						return true;
					else if(destdate.hour==hour)
					{
						if(destdate.minute<minute)
							return true;
						else if(destdate.minute==minute)
						{
							if(destdate.second<second)
								return true;
							else if(destdate.second==second)
								return false;
							else
								return false;
						}
						else
							return false;
					}
					else
						return false;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	void printTime()
	{
		cout<<year<<month<<day<<hour<<minute<<second<<endl;
	}

	int month;
	int day;
	int year;
	int hour;
	int minute;
	int second;
};

int main(int argc,char **argv)
{
	string firstWord;
	string oneLine;
	string parameter1=argv[1];
	string parameter2=argv[2];
	string inFilename="../SourceRibs/"+parameter1;

	string nexthopAS;
	string IP_prefix;
	string temp_prefix;
	string monthDayYear;
	string hourMinSec;
	char updateType='A';
	int totalLine=0;
	int sequenceNum=-1,oldSequence=-1;
	bool newSequen;
	bool moreFreshTime;
	DateTime oldTime,newTime;
/*	oldTime.setDateTime("03/01/17","11:23:34");
	newTime.setDateTime("03/01/17","11:23:33");
	oldTime.printTime();
	newTime.printTime();
	cout<<oldTime.moreFresh(newTime)<<endl;
	cout<<true<<endl;*/


	int readNumber=0;

	//inFilename=dataDirect+"rib_test";
	string outFilename="../Ribs/"+parameter2;
	ifstream inFile(inFilename.c_str());
	ofstream outFile(outFilename.c_str());
	if(!inFile.is_open())
	{
		cout<<"file open failed."<<endl;
	}
	while(!inFile.eof())
	{
		inFile>>firstWord;
		if(firstWord=="ASPATH:") 
		{
			if(moreFreshTime)
				inFile>>nexthopAS;
			
			/*outFile<<IP_prefix<<" "<<nexthopAS<<endl;
			totalLine++;
			if(totalLine%1000==0)
				cout<<totalLine/1000<<endl;*/
		}
		else if(firstWord=="SEQUENCE:") 
		{
			inFile>>sequenceNum;
//			cout<<"old sequence:"<<oldSequence<<endl;
//			cout<<"new sequence:"<<sequenceNum;
			if(sequenceNum==oldSequence)
			{
				newSequen=false;
			}
			else
			{
				oldSequence=sequenceNum;
				if(IP_prefix!="")
				{
					outFile<<IP_prefix<<" "<<nexthopAS<<endl;
					totalLine++;
					if(totalLine%1000==0)
						cout<<totalLine/1000<<endl;
				}
				IP_prefix=temp_prefix;
				newSequen=true;	
			}
		}
		else if(firstWord=="ORIGINATED:") 
		{
			inFile>>monthDayYear>>hourMinSec;	
			if(newSequen)
			{
				oldTime.setDateTime(monthDayYear,hourMinSec);
				moreFreshTime=true;
			}
			else
			{
				newTime.setDateTime(monthDayYear,hourMinSec);
				if(newTime.moreFresh(oldTime))
				{
					oldTime.setDateTime(newTime);
					moreFreshTime=true;
				}
				else
					moreFreshTime=false;
			}
		}
		else if(firstWord=="PREFIX:") 
		{
			inFile>>temp_prefix;
		}
		else
		{
		}
		getline(inFile,oneLine);
		//readNumber++;
		//cout<<readNumber<<endl;
	}
	outFile<<IP_prefix<<" "<<nexthopAS<<endl;
	inFile.close();
	outFile.close();
	return 0;
}
