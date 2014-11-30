#pragma once
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <functional>
#include <sstream>


using namespace std;

class bitTorrentHandshake{

		string infoString;
		string ipAndPort;

  public: 
		string getTheHandshakeText();
		string getInfoHash();
		string getpeerID();
		void setInfoString(string info);
		void setIpAndPort(string ipAndPortAddress);
};


