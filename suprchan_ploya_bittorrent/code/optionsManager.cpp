
#include "optionsManager.h"

using namespace std;

optionsManager* optionsManager::m_optManager = NULL;


/*
 *
 * a function to create and maintain the singleton object
 * of option manager
 */
optionsManager* optionsManager::Instance()
{
   if (!m_optManager)   // Only allow one instance of class to be generated.
	   m_optManager = new optionsManager();
   return m_optManager;
}


/*
 *
 *
 *
 */
void optionsManager::setPeerInfo(int numOfPeers, char * ptrToPeerString){
	listOfPeerIpPorts.push_back(string(ptrToPeerString));
}

/*
 * getpeerInfoList()
 *
 * getter method to return list of peers mentioned in the command line
 */
list<string> optionsManager::getpeerInfoList(){
	return listOfPeerIpPorts;	
}

/*
 * readOptions()
 * argc - No of arguments from the command line
 * argv - All command line arguments are stored as an array
 *
 * Reads the options of the command line and stores it in a dictionary
*/
void optionsManager::readOptions(int argc,char** argv) {

	char ch;
	int numOfPeers = 0;
	while ((ch = getopt(argc, argv, "hb:s:l:p:vI:")) != -1) {
		
	    switch (ch) {
		
	    case 'h': //help
		optionDict.insert(pair<string,string>{"help", GetStandardUsageOptionScreen()});
		return;
	      break;
	    case 's': //save the torrent to
		optionDict.insert(pair<string,string>{"torrentdir",optarg});
	      break;
	    case 'l'://output log file
	      	optionDict.insert(pair<string,string>{"log_file",optarg});
	      break;
	    case 'v'://verbose
	      	optionDict.insert(pair<string,string>{"verbose","true"}); 
	      break;
	    case 'I':
		//Assign dynamic memory if you can	
	      optionDict.insert(pair<string,string>{"nodeidentifier",optarg});
	      break;
	    case 'p'://peers
	    	numOfPeers++;
	    	setPeerInfo(numOfPeers, optarg);
	    	//optionDict.insert(pair<string,string>{"nodeidentifier",optarg});
		break;
		//check for max connections and exit program if it exceeds max connections
	//      break;
	    case 'b':
	    	optionDict.insert(pair<string,string>{"server",optarg});
	    	break;
	    default:
		fprintf(stderr,"ERROR: Unknown option '-%c'\n",ch);
		//fprintf(stderr,GetStandardUsageOptionScreen());
		exit(1);
	  }		
	}
}

/*
 * GetStandardUsageOptionScreen()
 *
 * Returns the HELP option screen
*/
string optionsManager::GetStandardUsageOptionScreen() {

return	"bt-client [OPTIONS] file.torrent \n \
	-h 		Print this help screen\n \
	-b ip 		Bind to this ip for incoming connections, ports\n \
			are selected automatically\n\
	-s save_file 	Save the torrent in directory save_dir (dflt: .) \n \
	-l log_file 	Save logs to log_file (dflt: bt-client.log) \n \
	-p ip:port 	Instead of contacting the tracker for a peer list, \n \
			use this peer instead, ip:port (ip or hostname) \n \
			(include multiple -p for more than 1 peer)\n \
	-I id 		Set the node identifier to id, a hex string \n \
			(dflt: computed based on ip and port) \n \
	-l 		Listen on port instead of connecting and write output to file \n \
			and dest_ip refers to which ip to bind to (dflt: localhost) \n \
	-v 		verbose, print additional verbose info\n";

}


/*
 * Method : getOptionDictionary
 *
 * Returns the option dictionary
*/
map<string,string> optionsManager::getOptionDictionary() {

	return optionDict;
}



