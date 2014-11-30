#include <stdio.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <fstream>
#include <string.h>

#include "bitTorrentHandshake.h"


using namespace std;

/*
 * getTheHandshakeText()
 *
 *	Returns the handshake message.
 */
string bitTorrentHandshake::getTheHandshakeText(){

	//printf(" string : %s" , handshakeMsg);
	stringstream initiationMsg;
	initiationMsg << (unsigned char)19 << "BitTorrent Protocol" << "00000000";
	initiationMsg.seekp(0, ios::end);
	stringstream::pos_type offset = initiationMsg.tellp();
	cout << "offset:" << offset;
	string infoHash = getInfoHash();
	string peerID = getpeerID();
	initiationMsg << infoHash << peerID;
	initiationMsg.seekp(0, ios::end);
	offset = initiationMsg.tellp();
	string handshakeText = initiationMsg.str();
 	cout << handshakeText << '\n';
 	cout << "offset:" << offset;
 	cout << " exit handshake : " << endl;
  	return handshakeText;
}

/*
 * getInfoHash()
 *
 * Returns the info hash
 */
string bitTorrentHandshake::getInfoHash(){
	unsigned char hash[21];
	cout << "info : " << infoString << endl;
	SHA1((unsigned char*) infoString.c_str(),infoString.length(),hash);
	hash[20] = '\0';
	return string((char *)hash);
}

/*
 * getpeerID()
 *
 * Returns the hash of peer id
 */
string bitTorrentHandshake::getpeerID(){
	unsigned char hash[21];
	SHA1((unsigned char*)ipAndPort.c_str() ,ipAndPort.length(),hash);
	hash[20] = '\0';
	cout << " PEER ID HASH :" << hash << endl;
	return string((char *)hash);
}

/*
 * setInfoString()
 * info - info string from the torrent file
 *
 * Setter method for the info string
 */
void bitTorrentHandshake::setInfoString(string info) {
	infoString = info;
}

void bitTorrentHandshake::setIpAndPort(string ipAndPortAddress) {
	ipAndPort = ipAndPortAddress;
}
