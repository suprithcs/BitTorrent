#include "fileOperationsManager.h"


using namespace std;

		
/*******************************************************************************************************************************************************
	METHOD: READFROMFILE
	ARGUMENTS: POINTER TO THE FILE THAT HAS TO BE READ, OFFSET AND NUMBER OF BYTES TO BE READ
	FUNCTION: READS A GIVEN FILE BASED ON THE GIVEN OFFSET AND NUMBER OF BYTES. DEFAULTS TO READ ENTIRE FILE IF BOTH OFFSET AND NUMBER OF BYTES IS NOT MENTIONED.
	RETURNS A POINTER TO BUFFER CONTAINING THE READ CONTENT.
*******************************************************************************************************************************************************/	

/*
 * translatePieceNumberIntoFileOffset()
 * fileToBeRead - file name to validate the total number of bytes with file offset
 * pieceNumber - piece number to decide the read offset  in the file
 * pieceLengthInBytes -  length of piece requested
 *
 *	Translate the user request piece number and pieceLengthInBytes into file offset
 */
long FileManager::translatePieceNumberIntoFileOffset(const char * fileToBeRead, int pieceNumber, int pieceLengthInBytes){
	int bytesInAFile;
	long positionInFile;
	bytesInAFile = FileManager::totalNumOfBytesInFile(fileToBeRead);
	positionInFile = pieceNumber*pieceLengthInBytes;
	if( positionInFile > bytesInAFile)
		cout << "INCORRECT PIECE NUMBER. FILE OFFSET EXCEEDS NUMBER OF BYTES EXISTING IN A FILE" << endl;
	return positionInFile;
}

/*
 * totalNumOfBytesInFile()
 * fileToBeRead - file name to get the total number of bytes
 *
 * Returns the file length in bytes
 */
long FileManager::totalNumOfBytesInFile(const char * fileToBeRead){
		
	FILE * ptrToRead = NULL;
	long totalNumberOfBytes = 0;
	ptrToRead = fopen (fileToBeRead,"r");
	if (ptrToRead==NULL) {
		//CONDITION TO CHECK FILE OPEN FOR READ FAILURE
			cout << "ERROR IN OPENING FILE FOR READ OPERATION!\n ERROR: " << strerror(errno) << endl;
			cout << "ERRNO is:" << errno << "\n"; 
			return -1;
	}
	if((fseek (ptrToRead , 0 , SEEK_END))==0) {
		// FIND THE TOTAL NO OF BYTES AVAILABLE IN THE FILE
						BitTorrentLogger::Instance()->logWithTimeStampToFile("SEEK TO END OF FILE SUCCEEDED\n");
						totalNumberOfBytes = ftell (ptrToRead);
						BitTorrentLogger::Instance()->logWithTimeStampToFile("TOTAL NO. OF BYTES IN THE FILE IS: " + to_string(totalNumberOfBytes));
						rewind (ptrToRead);
	}
	else
		cout << "FAILED IN FINDING THE TOTAL NO OF BYTES IN THE FILE!";
	fclose(ptrToRead);
	return totalNumberOfBytes;
}

/*
 * createAnEmptyFileWithSize()
 *
 * filename - file name of the empty file to be created
 * size - size of the empty file to be created
 *
 * Returns the status that the empty gets created or not
 */
int FileManager::createAnEmptyFileWithSize(const char* filename, int size) {
	BitTorrentLogger::Instance()->logWithTimeStampToFile(" CREATING AN EMPTY FILE :  "+ string(filename) );
	int status = 0;
	string junkArray = string(size,'0');
	ofstream writeToThisFile;
	writeToThisFile.open(filename);
	if(ios::failbit )
			status = 1;
	writeToThisFile << junkArray;
	writeToThisFile.close();
	return status;
}


/*
 * readFromFile()
 *
 * netcatPartInputFile - file name of the name to be read
 * seekToOffset - seek within the file
 * readThisMuch - read number of characters in the file
 *
 *  Reads the file with the given offset and requested number of characters
 */
string FileManager::readFromFile(const char *netcatPartInputFile, long seekToOffset, int readThisMuch) {

	BitTorrentLogger::Instance()->logWithTimeStampToFile(" READING THE FILE "+ string(netcatPartInputFile));
	int totalNumberOfBytes = totalNumOfBytesInFile(netcatPartInputFile);
	if(readThisMuch == 0)
		readThisMuch = 0;
	if(seekToOffset + readThisMuch >= totalNumberOfBytes && seekToOffset < totalNumberOfBytes)
		readThisMuch = totalNumberOfBytes - seekToOffset;
	else if(seekToOffset > totalNumberOfBytes)
		return "";	//if the offset is not present in file, then return no data
	char* content = new char[readThisMuch+1];
	memset(content,'\0',readThisMuch+1);
	int status = 0;
	fstream readFile;
	readFile.open(netcatPartInputFile,ios::in);
	if(!(readFile.is_open())) {
		cout << "File could not be opened!\n";
		cout << "Error code: " << strerror(errno);
	}
	else {
		readFile.seekg(seekToOffset);
		readFile.read(content,readThisMuch);
		if(readFile.is_open())
			readFile.close();
	}
	return content;	
}

/*
 * writeAPiece
 * begin - seek within the file
 * length - read number of characters in the file
 *
 * Write from a specific position within the file
 */
int FileManager::writeAPiece(const char* filename, const char* content, long begin, int length) {
	
	BitTorrentLogger::Instance()->logWithTimeStampToFile(" WRITING A BLOCK IN FILE : "+ string(filename) );
	int status = 0;
	fstream writingFile;
	writingFile.open(filename,ios::in|ios::out);
	//if file exists do this
	if(!writingFile.good()) {
		writingFile.open(filename,ios::out);	
	}
	writingFile.seekg(begin);
	writingFile.write(content,length);
	if(writingFile.is_open())
		writingFile.close();
	return status;
}

/*
 * ReadTorrentFile()
 * filename - torrent file name to be read
 *
 * Returns the complete content of the torrent file
 */
char* FileManager::ReadTorrentFile(const char* filename) {

	int fileBytes = totalNumOfBytesInFile(filename);
	fstream readFile;
	char *content = new char[fileBytes+1];
	readFile.open(filename,ios::in);
	if(!(readFile.is_open())) {
		cerr << "File could not be opened!\n";
		cerr << "Error code: " << strerror(errno);
	}
	else {
		readFile.read(content,fileBytes);
		if(readFile.is_open())
			readFile.close();
	}

	return content;
}

