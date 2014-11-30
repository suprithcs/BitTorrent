#pragma once
#include<iostream>
#include<string>
#include<map>
#include<list>
#include<sstream>
#include <stdio.h>
#include "fileOperationsManager.h"
#include "BitTorrentLogger.h"


typedef struct info {
	int pieceLength;
	long length;
	string name;

	list<string> pieces;
}INFO;

class MetainfoParser {

	string announce,info;
	const char* torrentFilename;
	int creationDate;
	map<string,string> infoDictionary;
	FileManager fileManager;
	INFO infoValue;
	char* rawContent;
	static MetainfoParser* m_metainfoParser;


	public :
		static MetainfoParser* Instance();
		int parseFile(const char* fileName);
		int getIntValueFromParser(stringstream &ss);
		string getStringValueFromParser(stringstream &ss);
		list<string> getPieceListFromInfo(stringstream &ss);
		int parseInfoDictionary(stringstream &ss);
		string getInfoString();
		INFO getInfoStruct();
		const char* getTorrentFile();
		const char* getContentFileName();
		int getFileLength();
		string getPieceHash(int pieceNo);
};
