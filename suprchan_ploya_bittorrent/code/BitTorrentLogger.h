#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <ctime>



using namespace std;

class BitTorrentLogger{

	const char* logFileName;
	int verboseState;
	static BitTorrentLogger* m_logger;

  public: 
	BitTorrentLogger();
	static BitTorrentLogger* Instance();
	int logMe(string logText,bool onlyLog=false);
	void logWithTimeStampToFile(string logText);

	string getLogFileName();
	void setLogFileName(const char* logFileName);
	int getVerboseState();
	void setVerboseState(int verboseState);
	string getTimeStamp();


};

