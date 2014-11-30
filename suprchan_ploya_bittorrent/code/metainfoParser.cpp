#include "metainfoParser.h"

using namespace std;
MetainfoParser* MetainfoParser::m_metainfoParser = NULL;

MetainfoParser* MetainfoParser::Instance()
{
   if (!m_metainfoParser)
	   	   m_metainfoParser = new MetainfoParser();
   return m_metainfoParser;
}

/*
	parseFile

	Parses the whole bencoded file and stores all the values to the members

*/

int MetainfoParser::parseFile(const char* filename) {

	torrentFilename = filename;
	long totalBytes = fileManager.totalNumOfBytesInFile(filename);
	if(totalBytes == -1) {
		cout << " TORRENT FILE DOES NOT EXIST" << endl;
		exit(1);
	}
	rawContent = new char[totalBytes+1];
	rawContent = fileManager.ReadTorrentFile(filename);
	stringstream ss(rawContent);
	int position,offset;	
	string token,key;
	BitTorrentLogger::Instance()->logWithTimeStampToFile("PARSING TORRENT");
	if(ss.get() == 'd') {

		key = getStringValueFromParser(ss);	
		if(key == "announce") 
			announce = getStringValueFromParser(ss);

		key = getStringValueFromParser(ss);	
		if(key == "creation date") 
			creationDate = getIntValueFromParser(ss);		
				
		key = getStringValueFromParser(ss);					
		if(key == "info") {
			streampos pos = ss.tellg();
			getline(ss,info);
			info.erase(info.length()-1,1);
			BitTorrentLogger::Instance()->logWithTimeStampToFile("info : "+ info);
			ss.seekg(pos);
			parseInfoDictionary(ss);
		}
	}
	else 
		cout << " Error in Parsing the torrent " << endl; 
	return 0;
}

/*
	getIntValueFromParser

	To parse each integer value in the bencoded torrent file

*/
int MetainfoParser::getIntValueFromParser(stringstream &ss) {

	int value=0;
	string token;
	if(ss.get() == 'i') {		
		getline(ss,token,'e');	
 		value = stoi(token);		
		BitTorrentLogger::Instance()->logMe(" Parsed INt value : " + to_string(value) );
	}
	return value;
}


/*
	getStringValueFromParser	

	Method to parse each string value in the bencoded torrent file
*/
string MetainfoParser::getStringValueFromParser(stringstream &ss) {	
	char *value;
	int noOfChars;
	string token;	
	getline(ss,token,':');
	BitTorrentLogger::Instance()->logMe(" POSITION " +  to_string(ss.tellg()));
	BitTorrentLogger::Instance()->logMe(" TOKEN : " + token);
	noOfChars = stoi(token);
	value = new char[noOfChars+1];
	ss.read(value,noOfChars);
	value[noOfChars] = '\0';
	return string(value);
}


/*
	getPieceListFromInfo	

	Read the hash pieces and store it in a piece list

*/
list<string> MetainfoParser::getPieceListFromInfo(stringstream &ss) {
	list<string> pieceList;
	char hash[21],result[41];
	string token;
	getline(ss,token,':');
	int piecesToRead = stoi(token)/20;
	BitTorrentLogger::Instance()->logMe(" NO OF PIECES : " + to_string(piecesToRead));
	char* post = strstr(rawContent,"pieces");
	while(*post != ':')
		post++;
	post++;

	for(int j = 0 ; j < piecesToRead; j++) {
		memset(hash,'\0',21);
		memset(result,'\0',41);
		memcpy(hash,post,20);
		post += 20;
		for(int i = 0; i < 20; i++)
			sprintf(result+i*2, "%02x", (unsigned int) hash[i]);
		BitTorrentLogger::Instance()->logMe("PARSER : TORRENT FILE PIECE HASH: "+ string(result));
		pieceList.push_back(string(result));
	}	
	return pieceList;
}	


/*
	parseInfoDictionary	

	Method to parse the info field of the bencoded torrent file
*/

int MetainfoParser::parseInfoDictionary(stringstream &infoSS) {

	string key;
	int status = 1;
	if(infoSS.get() == 'd') {
		int count = 0;
		const char* keyChar;
		while(count < 4) { 	
			count++;
			key = getStringValueFromParser(infoSS);		

			if( key == "length")
				infoValue.length = getIntValueFromParser(infoSS);				
			else if( key == "piece length")
				infoValue.pieceLength = getIntValueFromParser(infoSS);	
			else if( key == "name")
				infoValue.name = getStringValueFromParser(infoSS);
			else if( key == "pieces") {
				infoValue.pieces = getPieceListFromInfo(infoSS);	
			}		
			else{
				
				cout << " ERROR : Unidentified element in the info Field";
				status = 1;
				//error
			}		
		}
	}

	return status;
}

string MetainfoParser::getInfoString() {
	return info;
}

INFO MetainfoParser::getInfoStruct() {
	return infoValue;
}

const char* MetainfoParser::getTorrentFile() {
	return torrentFilename;
}

const char* MetainfoParser::getContentFileName() {
	return infoValue.name.c_str();
}

int MetainfoParser::getFileLength() {
	return infoValue.length;
}

string MetainfoParser::getPieceHash(int pieceNo) {

	if(infoValue.pieces.size() > pieceNo && pieceNo >= 0){
		list<string>::iterator it = infoValue.pieces.begin();
		advance(it, pieceNo);
		return *it;
	}
	else
		return "";
}
