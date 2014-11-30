#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <sstream>

#include "BitTorrentLogger.h"


class FileManager{

  public: 

	long totalNumOfBytesInFile(const char * fileToBeRead);
	long translatePieceNumberIntoFileOffset(const char * fileToBeRead, int pieceNumber, int pieceLengthInBytes);
	string readFromFile(const char *netcatPartInputFile,long seekToOffset, int readThisMuch);
	int createAnEmptyFileWithSize(const char* filename, int size);
	char* readFromBinaryFile(const char *netcatPartInputFile, int seekToOffset, int readThisMuch);
	int writeAPiece(const char* filename,const char* content,long begin,int length);
	char* ReadTorrentFile(const char* filename);
};
