#include "metainfoParser.h"
#include "BitTorrentLogger.h"
#include "optionsManager.h"
#include "peer.h"
#include <fstream>
#include <iostream>




/*
 *		Method : processCommand
 * 		fileName : FileName either to read data from file in the client OR  write to the file in case of server.
 * 		opDict - option as key value pairs mentioned in command line
 *
 * 		Processes the commands from the option dictionary and takes appropriate actions
 *
*/

int processCommand(string torrentFilename,map<string,string> opDict) {

	Peer peer;
	BitTorrentLogger::Instance()->logMe(" IN PROCESS COMMAND");
	int returnVal = 0;

	auto value = opDict.find("help");
	if(value != opDict.end()) {
		cout << endl;
		cout << value->second;
		return 0;
	}

	value = opDict.find("verbose");
	if(value != opDict.end())
		BitTorrentLogger::Instance()->setVerboseState(1);

	value = opDict.find("torrentdir");
	if(value != opDict.end())
		peer.setOutputFileName(value->second.c_str());

	value = opDict.find("nodeidentifier");
	if(value != opDict.end())
		cout << "";

	value = opDict.find("server");
	if(value != opDict.end())  {
		string bindingIP = value->second;
		if(bindingIP == "localhost" || bindingIP == "127.0.0.1"){
			cout << "STARTING THE SEEDER USING LOOPBACK ADDRESS" << endl;
			peer.startSeeder(NULL,"15786");
		}
		else {
			cout << "STARTING THE SERVER USING THE SPECIFIED IP: " << bindingIP << endl;
			peer.startSeeder(bindingIP.c_str(),"15786");
		}
	}
	else {

			list<string> ipPortArray = optionsManager::Instance()->getpeerInfoList();
			peer.startLeecher(ipPortArray);
	}
	return returnVal;
}


/*
 * getTorrentFileName
 *
 * argc - count of arguments
 * inArguments - input arguments from command line
 *
 */
string getTorrentFileName(int argc, char* inArguments[]){

	string torrentFile = "";
	for(int i = 0; i < argc; i++) {

		if(strstr(inArguments[i],".torrent")) {
			torrentFile = string(inArguments[i]);
			break;
		}
	}
	return torrentFile;
}

int main(int argc, char* argv[]) {

	BitTorrentLogger::Instance()->logMe("STARTING THE PEER");
	if(argc < 2)
		cout << " For Usage type :  ./bt-client -h" << endl;
	else {
			optionsManager::Instance()->readOptions(argc,argv);
			map<string,string> opDict = optionsManager::Instance()->getOptionDictionary();
			auto value = opDict.find("help");
			if(value != opDict.end()) {
				cout << endl;
				cout << value->second;
				return 0;
			}
			value = opDict.find("log_file");
			if(value != opDict.end()) {
				BitTorrentLogger::Instance()->setLogFileName(value->second.c_str());
			}
			string torrentfile = getTorrentFileName(argc,argv);
			if(!(torrentfile.empty())) {

				if((MetainfoParser::Instance()->parseFile(torrentfile.c_str())) == 0) {
					processCommand(torrentfile,opDict);
				}
				else
					cout << "ERROR IN PARSING THE TORRENT FILE \n" ;
			}
			else
				cout << "TORRENT FILE IS NOT MENTIONED IN THE COMMAND LINE. \
					For Usage type :  ./bt-client -h ";
	}
}
