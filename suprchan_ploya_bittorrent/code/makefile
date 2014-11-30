
bt-client : fileManager.o BitTorrentLogger.o metainfoParser.o bitTorrentHandshake.o peer.o optionsManager.o BitTorrent.cpp
		g++ -std=c++0x fileManager.o BitTorrentLogger.o metainfoParser.o bitTorrentHandshake.o peer.o optionsManager.o BitTorrent.cpp -lssl -lcrypto -o bt-client

peer.o : peer.cpp
		g++ -std=c++0x -c peer.cpp -o peer.o
		
optionsManager.o : optionsManager.cpp
		g++ -std=c++0x -c optionsManager.cpp -o optionsManager.o

bitTorrentHandshake.o : bitTorrentHandshake.cpp 
		g++ -std=c++0x -c bitTorrentHandshake.cpp -o bitTorrentHandshake.o
		
metainfoParser.o :	fileManager.o BitTorrentLogger.o
		g++ -std=c++0x -c metainfoParser.cpp -o metainfoParser.o

fileManager.o:	fileOperationsManager.cpp 
		g++ -std=c++0x -c fileOperationsManager.cpp -o fileManager.o
		
BitTorrentLogger.o: BitTorrentLogger.cpp		
		g++ -std=c++0x -c BitTorrentLogger.cpp -o BitTorrentLogger.o
clean :
	rm *.o bt-client

