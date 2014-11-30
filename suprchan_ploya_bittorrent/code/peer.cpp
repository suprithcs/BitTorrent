#include "peer.h"

FileManager fileManager;
using namespace std;


/*
 * Default constructor sets up a peer
 *
 */
Peer::Peer() {

	INFO info = MetainfoParser::Instance()->getInfoStruct();
	noOfpieces = info.pieces.size();
	pieceArray = new int[noOfpieces];
	int i;
	for (i = 0; i < noOfpieces; i ++)
	    pieceArray[i] = i;
	srand(time(0));
	random_shuffle(pieceArray, pieceArray + noOfpieces);
	recievedPieceArray = new int[noOfpieces];
	for (i = 0; i < noOfpieces; i ++)
		    recievedPieceArray[i] = 0;
	outputFileName = "moby_output.txt";
	noOfPiecesRecieved = 0;
}


/*
 * createASocket
 *
 * Returns the created socket file descriptor
 */
int Peer::createASocket(string peerType) {
	int serverOpsStatus = 1, sockDesc;
	sockDesc = socket(AF_INET, SOCK_STREAM, 0);

	if(peerType == string("SEEDER"))
		fcntl(sockDesc, F_SETFL, O_NONBLOCK);
	if (sockDesc == -1)
		cout << " ERROR IN OPENING SOCKET FOR SERVER OPERATIONS "
				<< strerror(errno);
	BitTorrentLogger::Instance()->logMe("SOCKET HAS BEEN CREATED");
	return sockDesc;

}

/*
 * networkConnectClient
 *
 * serverNameOrIP - seeder ip
 * portNumber - seeder port
 *
 * Returns the connected socket desriptor
 */
int Peer::networkConnectClient(const char* ipAddress, const char* portNumber) {
	struct addrinfo machineInfo, *ptrToMachineInfo;
	memset(&machineInfo, 0, sizeof machineInfo );
	machineInfo.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	machineInfo.ai_socktype = SOCK_STREAM;
	
	int sockFd = createASocket("");
	
	int conStatus;

	if ((conStatus = getaddrinfo(ipAddress, portNumber, &machineInfo, &ptrToMachineInfo)) != 0) {
			cout << " FAILED TO GET DESTINATION INFORMATION  : " << gai_strerror(errno) << "\n";
	} else {
		if ((conStatus = connect(sockFd, ptrToMachineInfo->ai_addr, ptrToMachineInfo->ai_addrlen))!= 0)
		{
			cout << " CONNECTION ERROR : " << strerror(errno) << "\n";
			close(sockFd);
			sockFd = -1;
		}
	}
	return sockFd;
}

/*
 * acceptAConnRequest()
 *
 * seederSocketDescriptor - listen socket descriptor of the seeder
 */
int Peer::acceptAConnRequest(int seederSocketDescriptor) {
	
	struct sockaddr_in clientAddressInfo;
	int newClientSocket;
	socklen_t sizeOfClientAddrStr;
	sizeOfClientAddrStr = sizeof(clientAddressInfo);
	
	if((newClientSocket = accept(seederSocketDescriptor,(sockaddr*) &clientAddressInfo,&sizeOfClientAddrStr)) == -1) {
		cout << "CLIENT CONNECTION FAILED" << strerror(errno) << endl;
	} else {
		string ipandPort = getPeerIPAndPort(clientAddressInfo);
		btHandshake.setIpAndPort(ipandPort);
		if(!isValidHandShake(newClientSocket)) {
			close(newClientSocket);
			newClientSocket = -1;
		}
		cout << "Client's connection request has been accepted and new socket has been assigned" << endl;
		cout << "newClientSocket:" << newClientSocket << endl;
	}
	return newClientSocket;

}

/*
 * compareHashes()
 * sockWriteBuffer - content of handshake verifiedS
 *
 * Returns the handshake status
 */
bool Peer::compareHashes(vector<char> sockWriteBuffer) {
	bool handshakeStatus = false;

	char* clientInfoHash =  new char[20];
	char *peerIPAndPortHash =  new char[20];
	stringstream completeData(string(sockWriteBuffer.begin(), sockWriteBuffer.end()));
	completeData.seekg(28);
	completeData.read(clientInfoHash, 20);
	//completeData.seekg(20);
	completeData.read(peerIPAndPortHash,20);
	completeData.clear();
	clientInfoHash[20] = '\0';
	peerIPAndPortHash[20] = '\0';
	BitTorrentLogger::Instance()->logMe("COMPARING THE PEER'S INFO_HASH WITH MINE");
	string info = MetainfoParser::Instance()->getInfoString();
	btHandshake.setInfoString(info);
	string serverHash = btHandshake.getInfoHash();
	string myIPandPortHash = btHandshake.getpeerID();
		cout << "Hash1 :" << serverHash << endl;
		cout << "Hash2 :" << clientInfoHash << endl;
	if (serverHash.compare(string(clientInfoHash)) ==0 &&
			myIPandPortHash.compare(string(peerIPAndPortHash)) == 0) {
		cout << "INFO HASHES MATCHED" << endl;
		cout << "HANDSHAKE DONE" << endl;
		handshakeStatus  = true;		
	} 
	else
		cout << "MESSAGE INTEGRITY VERIFIED AND DATA DOES NOT MATCH " << endl;
	return handshakeStatus;
}


/*
 * bindMySocketAndListen()
 * socketDescriptor - binded socket
 *
 *
 */
int Peer::bindMySocketAndListen(const char* ipAddress, int socketDescriptor) {
	int serverOpsStatus = 1;
	int listenStatus;
	int newClientSocket;
	int recievedSize;
	char * sockReadBuffer = new char[100];
	vector<char> sockWriteBuffer;
	fd_set serverFDs;
	int serverFDMax;
	FD_ZERO(&serverFDs);
	fd_set superSet;
	FD_ZERO(&superSet);
	string serverIp = getSelfIP(ipAddress);
	cout << "My selfIP: " << serverIp << endl;
	string port = getSelfPort(socketDescriptor);
		
		listenStatus = listen(socketDescriptor, 10);
	
		if (listenStatus != 0)
			cout << "SERVER IS UNABLE TO LISTEN ON THE SELECTED PORT"
					<< strerror(errno) << endl;
		else {
			FD_SET(socketDescriptor, &superSet);
			serverFDMax = socketDescriptor;	
			BitTorrentLogger::Instance()->logMe(
					"SERVER IS LISTENING ON THE CHOSEN PORT AND CAN ACCEPT CONNECTION REQUESTS FROM 10 CLIENTS AT A TIME");
			cout << "SERVER IS READY TO ACCEPT CONNECTIONS" << endl;
			
			while (1) {

				serverFDs = superSet;
				if(select(serverFDMax+1, &serverFDs, NULL, NULL, NULL) == -1)
					cout << "ERROR SELECT" << endl;
				else{
					
					for(int i=0; i<= serverFDMax;i++) {
						BitTorrentLogger::Instance()->logMe("FD MAX : " + to_string(serverFDMax));
						if(FD_ISSET(i, &serverFDs)){
						
						 	recievedSize = 0;
							if(i == socketDescriptor)
							{
								newClientSocket = acceptAConnRequest(socketDescriptor);
								if(newClientSocket != -1){ 
									FD_SET(newClientSocket, &superSet);
									if(newClientSocket > serverFDMax)
										serverFDMax = newClientSocket;
									BitTorrentLogger::Instance()->logMe("SEEDER : CLIENT ADDED : " + to_string(newClientSocket));
									btHandshake.setIpAndPort(serverIp+":"+port);
									string serverhandshakeData = btHandshake.getTheHandshakeText();
									int bytes_sent = send(newClientSocket,serverhandshakeData.c_str() ,serverhandshakeData.length(), 0);
									BitTorrentLogger::Instance()->logMe("BYTES SENT : " + to_string(bytes_sent));
									string bitFieldMsg = createBitFieldMessage();
									bytes_sent = send(newClientSocket,bitFieldMsg.c_str(),bitFieldMsg.length(), 0);
									BitTorrentLogger::Instance()->logMe(" BYTES SENT BITFIELD : " + to_string(bytes_sent));
								}
						}
						else {

							recievedSize = recv(i, sockReadBuffer, 1, MSG_PEEK);
							while(recievedSize > 0 ) {
								recievedSize = recv(i, sockReadBuffer, 64, 0);
								sockWriteBuffer.insert(sockWriteBuffer.end(), sockReadBuffer,sockReadBuffer + recievedSize);
								BitTorrentLogger::Instance()->logMe("RECIEVED NUMBER OF BYTES: " + to_string(recievedSize));
								if(recievedSize < 64)	//Need to change this to the data we need every time from a recv
									break;
							}
							string data = string(sockWriteBuffer.begin(),sockWriteBuffer.end());
							sockWriteBuffer.clear();
							processMessageTypeSeeder(data,i);
							if(data.find("$$FILE TRANSFER COMPLETED$$") != string::npos) {
								cout << "CLOSING CLIENT CONNECTION" <<  endl;
								close(i);
								FD_CLR(i,&superSet);
							}
						}
					}
				}
			}
		}
	}

	return listenStatus;
}

/*
 * getRequestedData()
 * message - request message to get the data
 *
 * Returns the piece message to be sent to the client
 */
string Peer::getRequestedData(string message) {
	int index;
	long length;
	long begin;
	stringstream ss(message);
	string temp;
	string token;
	getline(ss,token,'|');
	//ignore type
	getline(ss,token,'|');
	temp = token.substr(token.find(':')+1,token.length() - 1);
	index = stoi(temp);
	getline(ss,token,'|');
	temp = token.substr(token.find(':')+1,token.length() - 1);
	begin = stoi(temp);
	getline(ss,token);
	temp = token.substr(token.find(':')+1,token.length() - 1);
	length = stoi(temp);
	const char* filename = MetainfoParser::Instance()->getContentFileName();
	FileManager fileManager;
	INFO info = MetainfoParser::Instance()->getInfoStruct();
	long fileOffset = fileManager.translatePieceNumberIntoFileOffset(filename,index,info.pieceLength);
	BitTorrentLogger::Instance()->logMe("FILE OFFSET: " + to_string(fileOffset));
	string content = fileManager.readFromFile(filename,fileOffset+begin,length);
	string resp = createPieceMessage(index,begin,content);
	return resp;
}


/*
 * isValidHandShake
 * socketFd - socket file descriptor
 *
 * Returns the status of the handshake
 */
bool Peer::isValidHandShake(int socketFd) {

	char * sockReadBuffer = new char[100];
	vector<char> sockWriteBuffer;
	int recievedSize = recv(socketFd, sockReadBuffer, 1, MSG_PEEK);
	while(recievedSize > 0 ) {
		recievedSize = recv(socketFd, sockReadBuffer, 68, 0);
		sockWriteBuffer.insert(sockWriteBuffer.end(), sockReadBuffer,sockReadBuffer + recievedSize);
		BitTorrentLogger::Instance()->logMe("RECIEVED NUMBER OF BYTES: " + to_string(recievedSize));
		if(recievedSize <= 68)	//Need to change this to the data we need every time from a recv
			break;
	}
	BitTorrentLogger::Instance()->logMe(" WRITE BUFFER " + string(sockWriteBuffer.begin(),sockWriteBuffer.end()));
	return compareHashes(sockWriteBuffer);
}


/*
 * createANetworkingHost()
 *
 * serverNameOrIP - binding IP
 * portNumber - binding port number
 *
 * Returns the socket descriptor of the listen port of binding seeder
 */
int Peer::createANetworkingHost(const char* serverNameOrIP, const char* portNumber,string peerType) {
	struct addrinfo machineInfo, *ptrToMachineInfo;
	memset(&machineInfo, 0, sizeof(machineInfo));
	machineInfo.ai_flags = AI_PASSIVE;
	int serverOpsStatus = 1;
	int socketDescriptor;
	const char* freePort = giveMeAFreePort(portNumber, serverNameOrIP);
	serverOpsStatus = getaddrinfo(serverNameOrIP, freePort, &machineInfo,&ptrToMachineInfo);
	if (serverOpsStatus == 0) {
		BitTorrentLogger::Instance()->logMe("Required machine information has been stored");
		socketDescriptor = createASocket(peerType);
		int s=1;
		setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &s, sizeof(int));
		if(peerType =="SEEDER"){
			serverOpsStatus = bind(socketDescriptor, ptrToMachineInfo->ai_addr,ptrToMachineInfo->ai_addrlen);
			cout << "serverOpsStatus: " << serverOpsStatus << endl;
			if (serverOpsStatus == 0){
				BitTorrentLogger::Instance()->logMe("SOCKED BINDING HAS BEEN DONE");
				cout << "SERVER ON PORT:" << freePort << endl;}
			else{
				cout << "FAILED TO BIND PROCESS ON PORT" << gai_strerror(errno) << endl;
			}
		}
	}
	else
		cout << " FAILED TO GET HOST MACHINE'S INFORMATION NEEDED FOR SERVER CREATION  : " << gai_strerror(errno) << "\n";
	return socketDescriptor;
}

/*
 Method : startSeeder

 Arguments :
 ipAddress : IP of the seeder
 fileName : File name where the content needs to be written
 portNumber : port number on which the application starts

 Starts the seeder listen.

 */
int Peer::startSeeder(const char* ipAddress,const char* portNumber) {

	cout << "PORT NO: " << portNumber << endl;

	int serverOpsStatus, newClientSocket, serverSocketDescriptor, clientCount =	1;
	BitTorrentLogger::Instance()->logMe("ENTERING SERVER CODE IMPLEMENTATION");
	string hash = MetainfoParser::Instance()->getInfoString();
	btHandshake.setInfoString(hash);
	if ((serverSocketDescriptor = createANetworkingHost(ipAddress, portNumber,"SEEDER")) !=  0) {
		cout << "SERVER READY TO LISTEN :  ";
		bindMySocketAndListen(ipAddress, serverSocketDescriptor);
	}
	return 0;
}


/*
 * startLeecher
 *
 *
 */
int Peer::startLeecher(list<string> ipPortArray) {

	vector<char> sockWriteBuffer;
	int leechFDMax;
	int seederSockDescriptor;
	char* sockReadBuffer = new char[65];
	BitTorrentLogger::Instance()->logMe("CREATE LEECHER CLIENT CONNECTION");
	list<string>::iterator it;
	int i = 0;
	int recievedSize;
	vector<int> seederList;
	string ip, port;
	for(it = ipPortArray.begin(); it != ipPortArray.end(); ++it) {
		ip = it->substr(0, it->find(":"));
		port = it->substr(it->find(":")+1, it->length());
		seederSockDescriptor = verifySeederAndReturnSocketFD(ip,port);
		if(seederSockDescriptor != -1) {

			seederList.push_back(seederSockDescriptor);
			cout << "WAITING FOR BITFIELDS FROM THE SEEDERS POOL" << endl;
			recievedSize = recv(seederSockDescriptor, sockReadBuffer, 1, MSG_PEEK);
			while(recievedSize > 0 ) {
				memset(sockReadBuffer,'\0',64);
				recievedSize = recv(seederSockDescriptor, sockReadBuffer, 64, 0);
				sockWriteBuffer.insert(sockWriteBuffer.end(), sockReadBuffer,sockReadBuffer + recievedSize);
				BitTorrentLogger::Instance()->logMe("LEECHER : RECIEVED NUMBER OF BYTES: " + to_string(recievedSize));
				if(recievedSize < 64 ) {
					break;
				}
			}
			string data =  string(sockWriteBuffer.begin(), sockWriteBuffer.end());
			sockWriteBuffer.clear();
			processMessageTypeLeecher(data,seederSockDescriptor);
		}
	}
	if(seederList.size() > 0) {
		getDataFromPeers(seederList);
	}
	return 0;
}


/*
 * getDataFromPeers()
 *
 * seeder List - list of seeders with successful handshake
 *
 */
void Peer::getDataFromPeers(vector<int> seederList) {

	for(;;) {
		int x;
		for(int j = 0; j < noOfpieces; j++){
			if(recievedPieceArray[pieceArray[j]] != 1) {
				x = x % seederList.size();
				cout <<"REQUESTING PIECE NO: "<< pieceArray[j] << ", FROM THE SEEDER: " << x << endl;
				requestDataFromSeedersAndWrite(seederList.at(x),pieceArray[j]);
				x++;
			}
		}
		if(isAllDataRecieved()) {
			for(int k = 0; k < seederList.size() ; k++) {
				send(seederList.at(k), "$$FILE TRANSFER COMPLETED$$",27,0);
				close(seederList.at(k));
			}// All pieces done. Lets exit
			break;
		}
	}

}



/**
 * createBitFieldMessage()
 *
 * Creates bitfield for the available pieces of data
 *
 */
string Peer::createBitFieldMessage() {
	INFO info = MetainfoParser::Instance()->getInfoStruct();
	int noOfPieces = (int)ceil((double)info.length/info.pieceLength);
	cout << " NO Of PIECES : " << noOfPieces << endl;
	string bitFieldString(noOfPieces,'1');
	stringstream ss;
	ss << "type:BITFIELD"<< "|" << bitFieldString;
	ss.seekp(0, ss.end);
	string returnString = ss.str();
	return returnString;
}


/*
 * processMessageTypeSeeder
 *
 * data - The data or the request messages from the leecher(peer)
 * sockDescriptor - socket descriptor of the connected peer at seeder
 */
void Peer::processMessageTypeSeeder(string data, int sockDescriptor) {


	int bytesSent;
	if(data.find("type:INTERESTED") != string::npos) {
		cout <<" THE CLIENT IS INTERESTED";
	}
	if(data.find("type:HAVE") != string::npos) {
		cout << "PEER CONFIRMS THAT THE PIECE NUMBER HAS BEEN COMPLETELY DOWNLOADED" << endl;
		cout << data << endl;
		//for multiple clients can be used to track
	}
	else if(data.find("type:REQUEST") != string::npos) {
		//from leecher
		string message = getRequestedData(data);
		bytesSent = send(sockDescriptor,message.c_str(),message.length(),0);
		BitTorrentLogger::Instance()->logMe("SEEDER : SENT BYTES: " + to_string(bytesSent));
	}
}

/*
 * processMessageTypeSeeder
 *
 * data - The data or the messages from the seeder(peer)
 * sockDescriptor - socket descriptor of the connected peer(leecher) to seeder
 */
void Peer::processMessageTypeLeecher(string data,int sockDesc) {

	string value;
	string haveMessage;
	string intrstMsg;
	cout <<"PROCESSING RECIEVED BITFIELDS" << endl;
	if(data.find("type:BITFIELD") != string::npos) {
		value = getBitFieldMessage(data);
		int status;
		intrstMsg = createInterestedMessage();
		send(sockDesc,intrstMsg.c_str(),intrstMsg.length(),0);
		
	}
	cout <<"SENT INTERESTED MESSAGE TO SEEDERS" << endl;

}	

/*
 * requestDataFromSeedersAndWrite()
 * seederSock - socket connected to seeder from leecher.
 * currPiece - current piece number to be requested to seeder
 */
void Peer::requestDataFromSeedersAndWrite(int seederSock, int currPiece){	
	int status;
	status  = repeatSendAndRecieve(seederSock,currPiece);
	if(status ==0) {
		string haveMessage;
		recievedPieceArray[currPiece] = 1;
		noOfPiecesRecieved++;
		int percentageCompleted = ((double)noOfPiecesRecieved/noOfpieces) * 100;
		cout << "TRANSFER PROGRESS : " << percentageCompleted << "%" << endl;
		haveMessage = createHaveMessage(currPiece);
		send(seederSock, haveMessage.c_str(), haveMessage.length(),0);
	}
}


/*
 * createHaveMessage
 *
 * create have message for the complete current piece recieved at the leecher
 *
 */
string Peer::createHaveMessage(int pieceNumber){
	stringstream ss;
	ss << "type:HAVE"<< "|piece:" << pieceNumber;
	ss.seekp(0, ss.end);
	string returnString = ss.str();
	return returnString;
}



/*
 * createInterestedMessage
 *
 * create interest message to be sent to server
 */
string Peer::createInterestedMessage() {
	stringstream ss;
	ss << "type:INTERESTED";
	ss.seekp(0, ss.end);
	string returnString = ss.str();
	return returnString;
}


/*
 * getBitFieldMessage()
 *
 * create and return bitfield message sent from seeder to leecher
 *
 */
string Peer::getBitFieldMessage(string message) {

	INFO info = MetainfoParser::Instance()->getInfoStruct();
	int noOfPieces = (int)ceil((double)info.length/info.pieceLength);
	stringstream ss(message);
	string token;
	getline(ss,token,'|');
	getline(ss,token);
	return ss.str();
}

/**
 * createRequest
 *
 *
 * create request message for a particular block with offset as being and length of requested block
 */
string Peer::createRequest(int index, long begin, int length) {

	INFO info = MetainfoParser::Instance()->getInfoStruct();
	stringstream ss;
	ss<< "type:REQUEST";
	ss<< "|index:" << index ;
	ss<< "|begin:" << begin;
	ss<< "|length:" << length;
	return ss.str();
}

/**
 * repeatSendAndRecieve()
 *
 * send request and recieve a piece(index) from a seeder
 */
int Peer::repeatSendAndRecieve(int socketDesc,int index){
	int i = 0;
	int blockLength = 1024;
	char *sockReadBuffer;
	vector<char> sockWriteBuffer;
	int recvBytes = 0;
	sockReadBuffer = (char *) malloc(blockLength+1);
	string reqMsg;
	int sent;
	INFO info = MetainfoParser::Instance()->getInfoStruct();
	int status = -1;
	string data;
	int totalRecievedBytes = 0;
	string completePieceData;
	long maxIteration = info.pieceLength;
	long fileLength = info.length;

	if(fileLength < maxIteration)	//for file shorter than a piece
			maxIteration = fileLength;

	int bytesInABlock = 0;
	string temp;
	BitTorrentLogger::Instance()->logMe("LEECHER : MAX ITERATION: " + to_string(maxIteration),true);
	for(long i = 0; i < maxIteration; i+=blockLength)
	{

		sockWriteBuffer.clear();
		reqMsg = createRequest(index, i, blockLength);
		bytesInABlock = 0;
		BitTorrentLogger::Instance()->logMe("LEECHER : REQUEST MSG : " + reqMsg,true);
		sent = send(socketDesc, reqMsg.c_str(), reqMsg.length(),0);
		recvBytes = recv(socketDesc, sockReadBuffer, 1, MSG_PEEK);
		while(recvBytes > 0 ) {
			memset(sockReadBuffer,'\0',blockLength);
			recvBytes = recv(socketDesc, sockReadBuffer, blockLength, 0);
			sockWriteBuffer.insert(sockWriteBuffer.end(), sockReadBuffer,sockReadBuffer + recvBytes);
			BitTorrentLogger::Instance()->logMe("LEECHER : RECIEVED NUMBER OF BYTES: " + to_string(recvBytes),true);
			bytesInABlock += recvBytes;
			if(recvBytes < blockLength ) {
				break;
			}

		}
		BitTorrentLogger::Instance()->logMe("LEECHER : BYTES IN A BLOCK: " + to_string(bytesInABlock),true);
		//totalRecievedBytes += bytesInABlock;
		if(bytesInABlock > 0) {
			data =  string(sockWriteBuffer.begin(), sockWriteBuffer.end());
			temp = data.substr(data.find("data:")+5,data.length()-1);
			if(temp.length() > 0) {
				totalRecievedBytes += temp.length();
				status = getPieceMessage(data);
				if(status ==  0) {
					BitTorrentLogger::Instance()->logMe("LEECHER : TEMP DATA SIZE : " + to_string(temp.length()),true);
					completePieceData.append(temp);
				}
			}
		}
	}
	BitTorrentLogger::Instance()->logMe("LEECHER : PIECE NUMBER: " + to_string(index));
	BitTorrentLogger::Instance()->logMe("LEECHER : READ FROM: " + to_string(index*info.pieceLength),true);

	BitTorrentLogger::Instance()->logMe("LEECHER : TOTAL PIECE BYTES: " + to_string(totalRecievedBytes),true);
	status = comparePieceHash(completePieceData,index);
	return status;	
}

/*
 * getHash()
 *
 *
 * Returns hash for a text
 */
string Peer::getHash(string text) {

	char hash[21];
	char result[41];
	memset(hash,'\0',21);
	memset(result,'\0',41);
	SHA1((unsigned char*) text.c_str(),text.length(),(unsigned char*)hash);
	for( int i = 0; i < 20; i++)
			sprintf(result+i*2, "%02x", (unsigned int) hash[i]);
	BitTorrentLogger::Instance()->logMe("LEECHER : COMPLETED PIECE HASH: " + string(result));
	return string(result);
}

/**
 * createPieceMessage()
 *
 * creates a piece message to be sent to leecher from seeder
 */
string Peer::createPieceMessage(int pieceNo,long begin,string data) {

	stringstream ss;
	ss<< "type:PIECE";
	ss<< "|index:" << pieceNo ;
	ss<< "|begin:" << begin;
	ss<< "|data:" << data;
	return ss.str();

}

/**
 * getPieceMessage()
 *
 * Gets the piece message from seeder and writes to the file
 */
int Peer::getPieceMessage(string message) {

	int status = 1;
	string temp;
	int index;
	long begin;
	stringstream ss(message);
	string token;
	getline(ss,token,'|');
	getline(ss,token,'|');
	temp = token.substr(token.find(':')+1,token.length() - 1);
	index = stoi(temp);
	getline(ss,token,'|');
	temp = token.substr(token.find(':')+1,token.length() - 1);
	begin = stoi(temp);
	token = ss.str();
	temp = token.substr(token.find("data:")+5,token.length() - 1);
	FileManager fileManager;
	INFO info = MetainfoParser::Instance()->getInfoStruct();
	status = fileManager.writeAPiece(outputFileName,temp.c_str(),info.pieceLength*index+begin,temp.length());
	return status;
}

/**
 * comparePieceHash()
 *
 * Compares hash for a piece obtained from seeder to the torrent piece hash
 */
int Peer::comparePieceHash(string pieceData,int pieceNumber) {

	string pieceHash = getHash(pieceData);
	string torrPieceHash = MetainfoParser::Instance()->getPieceHash(pieceNumber);
	int status = pieceHash.compare(torrPieceHash);
	if(status == 0)
		cout << "PIECE INTEGRITY VERIFIED AND MATCHED" <<endl;
	else
		cout << "PIECE DIGEST DOESN'T MATCH" << endl;

	return status;
}

/*
 * getPeerIPAndPort()
 *
 * Returns the IP and port of the connected leecher to the seeder
 * */
string Peer::getPeerIPAndPort(struct sockaddr_in &clientAddressInfo) {

	char *peerIP = inet_ntoa(clientAddressInfo.sin_addr);
	int port = ntohs(clientAddressInfo.sin_port);
	BitTorrentLogger::Instance()->logMe("IP Address of the client:" + string(peerIP) + "\n Connected on PORT:" + to_string(port));
	return string(peerIP) +":"+to_string(port);
}

/*
 * setOutputFileName()
 *
 *
 * sets the output content file where the piece data is written
 */
void Peer::setOutputFileName(const char* outputFileName) {
		this->outputFileName = outputFileName;
}

/**
 * isAllDataRecieved()
 *
 * returns if all pieces are obtained from the seeders
 */
bool Peer::isAllDataRecieved() {

	for(int i = 0; i < noOfpieces ; i++ ) {
		if(recievedPieceArray[i] != 1)
			return false;
	}
	return true;
}

/**
 * getSelfIP()
 *
 * Returns the IP of the current system
 */
string Peer::getSelfIP(const char* ipAddress) {

	string selfIp;
	//cout << "got the ip address: " << ipAddress << endl;
	if( ipAddress == NULL || strcmp(ipAddress, "localhost") == 0 ){
		cout << "inside"<< endl;
		selfIp = "127.0.0.1";
	}
	else
		selfIp = ipAddress;
	return selfIp;
}

/**
 * getSelfPort()
 *
 * Returns the port on which client is connected to the seeder or leecher
 */
string Peer::getSelfPort(int sockDesc) {

	int portNo=-1;
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(sin);
	if(getsockname(sockDesc, (struct sockaddr *)&sin, &addrlen) == 0 && sin.sin_family == AF_INET &&
	   addrlen == sizeof(sin)) {
	   portNo = ntohs(sin.sin_port);
	}
	else
		cout << "ERROR " << endl;

	return to_string(portNo);
}

/**
 * verifySeederAndReturnSocketFD()
 *
 * Returns the socket descriptor if the handshake is completed for a seeder ip and port
 */
int Peer::verifySeederAndReturnSocketFD(string ip,string port) {

	string hash = MetainfoParser::Instance()->getInfoString();
	btHandshake.setInfoString(hash);
	int sockFd = networkConnectClient( ip.c_str(), port.c_str());
	if(sockFd != -1) {
		cout << "pssing ip:" << ip.c_str() << endl;
		string myipAddress = getSelfIP(ip.c_str());
		cout << "My selfIP: " << myipAddress << endl;
		string myPort = getSelfPort(sockFd);
		btHandshake.setIpAndPort(myipAddress+":"+myPort);
		string handshakeData = btHandshake.getTheHandshakeText();
		int recievedSize;
		if(sockFd > 0) {
			int bytes_sent = send(sockFd,handshakeData.c_str() ,handshakeData.length(), 0);
			BitTorrentLogger::Instance()->logWithTimeStampToFile("Leecher : bytes_sent: " + to_string(bytes_sent));
			//set ip and port of connecting peer to check handshake
			if(ip.compare("localhost") == 0)
				ip = "127.0.0.1";
			btHandshake.setIpAndPort(ip+":"+port);
			if(!isValidHandShake(sockFd))
				sockFd = -1;
		}
	}
	return sockFd;
}

/**
 *
 * giveMeAFreePort()
 *
 * Returns a non-busy port number which seeder can listen on
 */
const char* Peer::giveMeAFreePort(const char* port, const char* serverNameOrIP)
{
    struct addrinfo backupInfo, *ptrbackupInfo;
	memset(&backupInfo, 0, sizeof(backupInfo));
	backupInfo.ai_flags = AI_PASSIVE;
	//int sock = socket(AF_INET, SOCK_STREAM, 0);
	int sock = createASocket("");
	int status = -1;
	int serverOpsStatus;
	cout << "CHECKING PORT STATUS" << endl;
	for(int i = 0; i <= 4; i++){

		serverOpsStatus = getaddrinfo(serverNameOrIP, portArray[i], &backupInfo, &ptrbackupInfo);
		if (serverOpsStatus == 0) {
			status = bind(sock, ptrbackupInfo->ai_addr,ptrbackupInfo->ai_addrlen);
			if(status == 0){
				cout << "FREE PORT FOUND" << portArray[i] << endl;
				close(sock);
				return portArray[i];
			}
		}

	}
	close(sock);
	return "";
}


