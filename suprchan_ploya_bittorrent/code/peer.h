#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <map>
#include <vector>
#include <openssl/sha.h>
#include <math.h>
#include <iterator>
#include <sstream>
#include <fcntl.h>
#include <algorithm>
#include <ifaddrs.h>
#include "optionsManager.h"
#include "fileOperationsManager.h"
#include "metainfoParser.h"
#include "bitTorrentHandshake.h"
#include "BitTorrentLogger.h"


class Peer {
	bitTorrentHandshake btHandshake;
	int noOfpieces;
	int *pieceArray;
	int *recievedPieceArray;
	const char* outputFileName;
	int currPiece;
	const char* portArray[5] = {"15786", "15886", "15996", "16096", "16196"};
	int noOfPiecesRecieved;

	public :
	Peer();
	int createASocket(string peerType);
	int networkConnectClient(const char* serverNameOrIP, const char* portNumber);
	int acceptAConnRequest(int serverSocketDescriptor);
	bool compareHashes(vector<char> sockWriteBuffer);
	int bindMySocketAndListen(const char* ipAddress, int serverSocketDescriptor);
	int createANetworkingHost(const char* serverNameOrIP,const char* portNumber,string peerType);
	int startSeeder(const char* ipAddress,const char* portNumber);
	int clientConnect(const char* serverNameOrIP,char* fileName,const char* portNo);
	int startLeecher(list<string> ipPortArray);
	bool isValidHandShake(int socketFd);
	string getHash(string text);
	string createBitFieldMessage();
	string getBitFieldMessage(string message);
	void writePieceMessage(string sockWriteBuffer);
	void processMessageTypeSeeder(string data, int sockDescriptor);
	void processMessageTypeLeecher(string data,int sockDesc);
	string createInterestedMessage();
	string checkPieceExistence(string bitField);
	string createHaveMessage(int pieceNumber);
	int digestCompare(string recvPieceDigest, string torrFileDigest);
	string createRequest(int index, long begin, int length);
	string getRequestedData(string message);
	string createPieceMessage(int pieceNo,long begin,string data);
	int getPieceMessage(string message);
	int comparePieceHash(string pieceData,int pieceNumber);
	int repeatSendAndRecieve(int socketDesc, int index);
	string getPeerIPAndPort(struct sockaddr_in &clientAddressInfo);
	void setOutputFileName(const char* outputFileName);
	bool isAllDataRecieved();
	string getSelfIP(const char* ipAddress);
	string getSelfPort(int sockDesc);
	int verifySeederAndReturnSocketFD(string ip,string port);
	const char* giveMeAFreePort(const char* port, const char* serverNameOrIP);
	void requestDataFromSeedersAndWrite(int seederSock, int currPiece);
	void getDataFromPeers(vector<int> seederList);
};

