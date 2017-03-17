#include<iostream>
#include<fstream>
#include<string>

using namespace std;

int main(int argc,char **argv)
{
	string firstWord;
	string oneLine;
	string parameter1=argv[1];
	string parameter2=argv[2];
	string inFilename=parameter1;
	string monthDayYear;
	string hourMinSecond;
	string nexthopAS;
	char updateType='A';
	int totalLine=0;
	int readNumber=0;

	//inFilename=dataDirect+"update_test";
	string outFilename=parameter2;
	cout<<inFilename<<" "<<outFilename<<endl;
	
	ifstream inFile(inFilename.c_str());
	ofstream outFile(outFilename.c_str(),std::ofstream::app);
	if(!inFile.is_open())
	{
		cout<<"file open failed."<<endl;
	}
	while(!inFile.eof())
	{
		inFile>>firstWord;
		if(firstWord=="TIME:")
		{
			inFile>>monthDayYear>>hourMinSecond;
		} 
		else if(firstWord=="ASPATH:") 
		{
			inFile>>nexthopAS;
		}
		else if(firstWord=="ANNOUNCE")
		{
			updateType='A';
		}
		else if(firstWord=="WITHDRAW")
		{
			updateType='W';
		}
		else if(firstWord.find('.',0)!=-1)
		{
			outFile<<monthDayYear<<" "<<hourMinSecond<<" "<<updateType;
			outFile<<" "<<firstWord;
			if(updateType=='A')
				outFile<<" "<<nexthopAS;
			outFile<<endl;
			totalLine++;
			if(totalLine%100000==0)
				cout<<inFilename<<":"<<totalLine/100000<<endl;
		}
		else
		{
		}
		getline(inFile,oneLine);
		//readNumber++;
		//cout<<readNumber<<endl;
	}
	inFile.close();
	outFile.close();
	return 0;
}
