#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <map>
#include <vector>
#include <algorithm> // for copy
#include <iterator>
#include <string.h>
#include <unistd.h>
#include<list>

using namespace std;

class optionsManager {
	
	map<string,string> optionDict;
	static optionsManager* m_optManager;
	//vector<char> listOfPeerIpPorts;
	list<string> listOfPeerIpPorts;
	
	public : 
		
		void readOptions(int argc,char* argv[]);
		static optionsManager* Instance();
		string GetStandardUsageOptionScreen();
		map<string,string> getOptionDictionary();
		void setPeerInfo(int numOfPeers, char * ptrToPeerString);
		//vector<char> getpeerInfoList();
		list<string> getpeerInfoList();
};
