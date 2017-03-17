#include<iostream>
#include<fstream>
#include<string>

using namespace std;

int main()
{
	string firstWord;
	string oneLine;
	string dataDirect="../Data/";
	string inFilename=dataDirect+"rib.20170301.0000_read";
	string updateType;
	int readNumber=0;

	ifstream inFile(inFilename.c_str());
	
	if(!inFile.is_open())
	{
		cout<<"file open failed."<<endl;
	}
	while(!inFile.eof())
	{
		inFile>>firstWord;
		if(firstWord=="TYPE:")
		{
			inFile>>updateType;
			if(updateType!="TABLE_DUMP_V2/IPV4_UNICAST")
			{
				cout<<"type error"<<updateType<<endl;
				return 0;
			}
			else
			{
				readNumber++;
				if(readNumber%1000==0)
					cout<<readNumber/1000<<endl;
			}
		} 
		else
		{
		}
		getline(inFile,oneLine);
		
		//cout<<readNumber<<endl;
	}
	inFile.close();
	return 0;
}
