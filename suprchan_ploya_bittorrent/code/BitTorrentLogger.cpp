#include "BitTorrentLogger.h"

using namespace std; 

BitTorrentLogger* BitTorrentLogger::m_logger = NULL;

/*****************************************************************************************************************************************************

	Method : netcatLogger()

	constructor to intialize the verbose state	
*****************************************************************************************************************************************************/
BitTorrentLogger::BitTorrentLogger(){
        verboseState = 0;
        logFileName = "log.txt";
}


BitTorrentLogger* BitTorrentLogger::Instance()
{
   if (!m_logger)   // Only allow one instance of class to be generated.
      m_logger = new BitTorrentLogger();
   return m_logger;
}



/*****************************************************************************************************************************************************

	Method : logMeOnVerbose
	
	Argument :
		logText : the text to be logged

	Displays the text on the screen if verbose option is enabled
*****************************************************************************************************************************************************/			
int BitTorrentLogger::logMe(string logText,bool onlyLog){

        if(verboseState == 1 && !logText.empty() && !onlyLog ){
        	cout << logText << endl;
        }
        logWithTimeStampToFile(logText);
        return 0;
}

/**
 * Method : logMeOnVerbose
 *
 * Logs the message with timestamp to the file
 */
void BitTorrentLogger::logWithTimeStampToFile(string logText) {

	string timeStamp = getTimeStamp();
	ofstream logFile;
	logFile.open(logFileName,ofstream::app);
	logFile << timeStamp;
	logFile << logText;
	logFile << endl;
	logFile.close();

}

/**
 * Method : getTimeStamp
 *
 * Returns the current timestamp
 */
string BitTorrentLogger::getTimeStamp() {

	int status = 0;
	char* timeInString = new char[19];
	time_t rtime;
	time(&rtime);
	struct tm* timeInfo = localtime(&rtime);
	strftime(timeInString,30,"[%m-%d-%Y %R]",timeInfo);
	timeInString[18] = '\0';
	string timeStamp = string(timeInString);
	delete[] timeInString;
	return timeStamp;
}

//All getters and setters of the logger
void BitTorrentLogger::setLogFileName(const char* logFilName) {
		logFileName = logFilName;
}

int BitTorrentLogger::getVerboseState() {
	return verboseState;
}

void BitTorrentLogger::setVerboseState(int verbState) {
	verboseState = verbState;
}

string BitTorrentLogger::getLogFileName() {
	return logFileName;
}


