#include<iostream>
#include<fstream>
#include<string>

using namespace std;

int main()
{
	string firstWord;
	string oneLine;
	string dataDirect="../Data/";
	string inFilename=dataDirect+"updates.20170301.0000_read";
	string updateType;

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
			if(updateType!="BGP4MP/MESSAGE/Update")
			{
				cout<<"type error"<<updateType<<endl;
				return 0;
			}
			else
				cout<<updateType<<endl;
		} 
		else
		{
		}
		getline(inFile,oneLine);
		//readNumber++;
		//cout<<readNumber<<endl;
	}
	inFile.close();
	return 0;
}
