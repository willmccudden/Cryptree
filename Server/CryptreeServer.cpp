#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <vector>
#include <string>
#include "ServerFunctions.h"
#include "ServerFolder.h"

#pragma comment(lib, "ws2_32.lib")
using byte_t = uint8_t;
using blockData = std::vector<byte_t>;

int main() {

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) {
		std::cerr << "Error starting winsock!" << std::endl;
		return -1;
	}

	SOCKET listenerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenerSock == INVALID_SOCKET) {
		std::cerr << "Error creating listener socket! " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}

	sockaddr_in listenerHint;
	listenerHint.sin_family = AF_INET;
	listenerHint.sin_port = htons(55000);
	listenerHint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listenerSock, (sockaddr*)&listenerHint, sizeof(listenerHint));
	listen(listenerSock, SOMAXCONN);

	sockaddr_in clientHint;
	int clientSize = sizeof(clientHint);

	SOCKET clientSock = accept(listenerSock, (sockaddr*)&clientHint, &clientSize);

	if (clientSock == SOCKET_ERROR) {
		std::cerr << "Error accept socket! " << WSAGetLastError() << std::endl;
		closesocket(listenerSock);
		WSACleanup();
		return -1;
	}

	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];

	if (getnameinfo((sockaddr*)&clientHint, sizeof(clientHint), host, NI_MAXHOST, serv, NI_MAXSERV, 0) == 0) {
		std::cout << "Host: " << host << " connected on port: " << serv << std::endl;
	}
	else {
		inet_ntop(AF_INET, &clientHint.sin_addr, host, NI_MAXHOST);
		std::cout << "Host: " << host << " connected on port: " << ntohs(clientHint.sin_port) << std::endl;
	}

	closesocket(listenerSock);

	const char* welcomeMsg = "Welcome to the Cryptree Server";
	bool clientClose = false;
	const int fileAvailable = 200;
	const int fileNotfound = 404;
	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];
	int clientCode = 0;


	// sending welcome message
	int bysendMsg = send(clientSock, welcomeMsg, strlen(welcomeMsg), 0);

	char folderName [FILENAME_MAX];
	int foldInf = recv(clientSock, (char*)&folderName, FILENAME_MAX, 0);

	bool routeExists = false;
	char cryptLinks[15]= "CryptLinks.enc";
	for (int i = 0; i < 14; i++) {
		folderName[i + 8] = cryptLinks[i];
	}
	std::string folderNameStr;
	for (int i = 0; i < 22; i++) {
		folderNameStr += folderName[i];
	}
	std::ifstream routeFolder(folderNameStr);

	//route already exists
	if (routeFolder.good()) {
		//send route exists
		routeExists = true;
		int routeInfo = send(clientSock, (char*)&routeExists, sizeof(bool), 0);

		//manage files
		routeFolder.close();
		std::ifstream cryptLinkFolder(folderNameStr, std::ios::binary);

		//read out data
		char cryptLinksData[1000];
		cryptLinkFolder.read((char*)&cryptLinksData, 212);

		//send data
		routeInfo = send(clientSock, (char*)&cryptLinksData,212, 0);

	}

	// route is created
	else if (folderNameStr == "CryptreeCryptLinks.enc") {
		//write route in foldernames file
		std::ofstream folderNames("FolderNames.txt", std::ios::binary|std::ios::trunc);
		folderNames << "Cryptree" << std::endl;
		folderNames.close();

		//send route exists
		int routeInfo = send(clientSock, (char*)&routeExists, sizeof(bool), 0);

		//create folder data file
		std::ofstream folderDataFile("CryptreeData.enc", std::ios::binary | std::ios::trunc);

		//recieve file size
		long folderDataFileSize;
		routeInfo = recv(clientSock, (char*)&folderDataFileSize, sizeof(long), 0);

		//recieve filedata
		char folderData[212];
		routeInfo = recv(clientSock, (char*)&folderData, folderDataFileSize, 0);
		
		//write to file
		folderDataFile.write((char*)&folderData, folderDataFileSize);

		//create crypt links file
		std::ofstream cryptLinksFolder("CryptreeCryptLinks.enc", std::ios::binary | std::ios::trunc);

		//recieve cryptLinks file size
		long cryptLinkFileSize;
		routeInfo = recv(clientSock, (char*)&cryptLinkFileSize, sizeof(long), 0);
		
		//recieve file data
		char cryptLinksData[300];
		routeInfo = recv(clientSock, (char*)&cryptLinksData, cryptLinkFileSize, 0);

		cryptLinksFolder.write((char*)&cryptLinksData,cryptLinkFileSize);
		cryptLinksFolder.close();
	}

	else {
		int routeInfo = send(clientSock, (char*)&routeExists, sizeof(bool), 0);
	}

	do {
		//recieve client code
		std::cout << "waiting to receive" << std::endl;
		int recvClientCode = recv(clientSock, (char*)&clientCode, sizeof(int), 0);
		std::cout << "recieved client code " << clientCode << std::endl;

		//send files
		if (clientCode == 1) {

			char fileRequested[FILENAME_MAX];
			int recvInfo = recv(clientSock, (char*)&fileRequested[0], FILENAME_MAX, 0);

			// open file
			std::ifstream file;
			file.open(fileRequested, std::ios::binary);

			if (file.is_open()) {
				// file is available
				int sendInfo = send(clientSock, (char*)&fileAvailable, sizeof(int), 0);

				// get file size
				file.seekg(0, std::ios::end);
				long fileSize = file.tellg();
				file.seekg(0, std::ios::beg);

				// send filesize to client
				sendInfo = send(clientSock, (char*)&fileSize, sizeof(long), 0);

				// read file with do-while loop
				do {
					// read and send part file to client
					file.read(bufferFile, BUFFER_SIZE);
					if (file.gcount() > 0) {
						sendInfo = send(clientSock, bufferFile, file.gcount(), 0);
					}
				} while (file.gcount() > 0);
				file.close();
			}
			else {
				int failInfo = send(clientSock, (char*)&fileNotfound, sizeof(int), 0);
			}
		}
		//recieve files
		else if (clientCode == 2) {

			int fileDownloaded = 0;
			//recieve filename
			char recievingFile[FILENAME_MAX];
			int recvInfo = recv(clientSock, (char*)&recievingFile, FILENAME_MAX, 0);

			// open new file
			std::ofstream recFile;
			recFile.open(recievingFile, std::ios::binary | std::ios::trunc);

			//recieve file size
			long recievingFileSize = 0;
			recvInfo = recv(clientSock, (char*)&recievingFileSize, sizeof(long), 0);

			//write to file
			do {
				memset(bufferFile, 0, BUFFER_SIZE);
				recvInfo = recv(clientSock, bufferFile, BUFFER_SIZE, 0);
				recFile.write(bufferFile, recvInfo);
				fileDownloaded += recvInfo;
			} while (fileDownloaded < recievingFileSize);
			recFile.close();
		}
		//close down
		else if (clientCode == 3) {
			clientClose = true;
			std::cout << host << " disconnected " << std::endl;
		}


		//create new folder
		else if (clientCode == 4) {
			//recieve folder data filename
			char folderDataFileName[FILENAME_MAX];
			int newFolderInfo = recv(clientSock, (char*)&folderDataFileName, FILENAME_MAX, 0);
			//std::cout << "recieved file name: " << folderDataFileName << std::endl;
			//std::cin.ignore();

			//create raw folder name
			std::string str;
			int i = 0;
			while (folderDataFileName[i] != '.') {
				str += folderDataFileName[i];
				i++;
			}
			std::string str2 = str.substr(0, str.length() - 4);
			//std::cout << "created string "<< std::endl;
			//std::cin.ignore();

			//add to folderNames file
			std::ofstream folderNames("FolderNames.txt", std::fstream::app);
			folderNames << str2 << std::endl;
			folderNames.close();
			//std::cout << "put string into folder names " << std::endl;
			//std::cin.ignore();

			//open data file
			std::ofstream folderDataFile(folderDataFileName, std::ios::binary | std::ios::trunc);
			//std::cout << "opened file " << std::endl;
			//std::cin.ignore();

			//recieve file size
			long folderDataFileSize;
			newFolderInfo = recv(clientSock, (char*)&folderDataFileSize, sizeof(long), 0);
			//std::cout << "recieved file size: " << folderDataFileSize << std::endl;
			//std::cin.ignore();

			//recieve file data
			char dataFile[100];
			newFolderInfo = recv(clientSock, (char*)&dataFile, folderDataFileSize, 0);
			//std::cout << "recieved file : " << newFolderInfo << std::endl;
			//std::cin.ignore();

			//write to file 
			folderDataFile.write((char*)&dataFile, folderDataFileSize);

			//recieve cryptLinks file name
			char cryptLinksFileName[FILENAME_MAX];
			newFolderInfo = recv(clientSock, (char*)&cryptLinksFileName, sizeof(std::string), 0);
			//std::cout << "recieved file name: " << cryptLinksFileName << std::endl;
			//std::cin.ignore();

			//open file
			std::ofstream cryptLinkFile(cryptLinksFileName, std::ios::binary | std::ios::trunc);

			//recieve file size
			long cryptLinkFileSize;
			newFolderInfo = recv(clientSock, (char*)&cryptLinkFileSize, sizeof(long), 0);
			//std::cout << "recieved file size: " << cryptLinkFileSize << std::endl;
			//std::cin.ignore();

			//recieve data
			char cryptLinkFileData[500];
			newFolderInfo = recv(clientSock, (char*)&cryptLinkFileData, cryptLinkFileSize, 0);
			//std::cout << "recieved file : " << newFolderInfo << std::endl;
			//std::cin.ignore();

			//write to file
			cryptLinkFile.write((char*)&cryptLinkFileData, cryptLinkFileSize);

			//cleanup
			cryptLinkFile.close();
			folderDataFile.close();

		}

		//subfolder
		else if (clientCode == 5) {

			//recieve current folder name
			std::string currentFolder;
			int subfolderInfo = recv(clientSock, (char*)&currentFolder[0], sizeof(std::string), 0);

			//open foldernames file and save names
			std::ifstream folderNames("FolderNames.txt");
			std::string str;
			std::vector<std::string> folders;
			while (std::getline(folderNames, str)) {
				folders.push_back(str);
			}

			// find subfolders of currentFolder
			std::ifstream thisFolder;
			char folderData[500];
			std::vector<std::string> subFolders;
			for (int i = 1; i < folders.size(); i++) {
				str = folders[i] + "CryptLinks.enc";
				thisFolder.open(str);
				thisFolder.seekg(0, std::ios::end);
				long thisFolderSize = thisFolder.tellg();
				thisFolder.seekg(0, std::ios::beg);
				thisFolder.read(folderData, thisFolderSize);
				int index = 320;
				str = "";
				std::string str2 = "";
				while (folderData[index] != ',') {
					str2 += folderData[index];
					index++;
				}
				index++;
				for (int j = index; j < thisFolderSize; j++) {
					str += folderData[index];
					index++;
				}
				if (str2 == currentFolder) {
					subFolders.push_back(str);
				}
				thisFolder.close();
			}


			for (int i = 0; i < subFolders.size(); i++) {
			}

			//send subfolder number
			int subfolderNum = subFolders.size();
			subfolderInfo = send(clientSock, (char*)&subfolderNum, sizeof(int), 0);

			//send folder name
			if (subfolderNum != 0) {
				for (int i = 0; i < subfolderNum; i++) {
					subfolderInfo = send(clientSock, (char*)&subFolders[i], sizeof(std::string), 0);
				}
			}
		}


		//return to previous folder
		else if (clientCode == 8) {
			
			//recieve folder name length
			long nameLength = 0;
			int returnCommandInfo = recv(clientSock, (char*)&nameLength, sizeof(long), 0);
			//recieve parent folder name
			char parentFolderName[FILENAME_MAX];
			returnCommandInfo = recv(clientSock, (char*)&parentFolderName[0], nameLength, 0);

			std::string requestedFolder;
			for (int i = 0; i < nameLength; i++) {
				requestedFolder += parentFolderName[i];
			}

			//find cryptlink folder
			std::string cryptoLinkFileName = requestedFolder + "CryptLinks.enc";
			std::ifstream cryptoLinkFile(cryptoLinkFileName, std::ios::binary);

			//get file size
			cryptoLinkFile.seekg(0, std::ios::end);
			long cryptoLinkFileSize = cryptoLinkFile.tellg();
			cryptoLinkFile.seekg(0, std::ios::beg);

			//write folder to blockdata
			char cryptoLinksData[500]; 
			cryptoLinkFile.read((char*)&cryptoLinksData, cryptoLinkFileSize);


			//send block data size
			returnCommandInfo = send(clientSock, (char*)&cryptoLinkFileSize, sizeof(long), 0);

			//send blockdata
			returnCommandInfo = send(clientSock, (char*)&cryptoLinksData, cryptoLinkFileSize, 0);

			//cleanup
			cryptoLinkFile.close();

			//std::cout << "sent folder data " << returnInfo << std::endl;
			//std::cin.ignore();
	    }

		// change folder
	    else if (clientCode == 6) {

		    //recieve folderName
		    std::string requestedFolder;
		    int changeFolderInfo = recv(clientSock, (char*)&requestedFolder[0], sizeof(std::string), 0);
		    //std::cout << "recieved requested folder name: " << requestedFolder << std::endl;
		    //std::cin.ignore();

		    //recieve current folder
		    std::string currentFolder;
		    changeFolderInfo = recv(clientSock, (char*)&currentFolder[0], sizeof(std::string), 0);
		    //std::cout << "recieved current folder name: " << currentFolder << std::endl;
		    //std::cin.ignore();

		    //check through foldernames
		    std::ifstream folderNames("FolderNames.txt");
		    //std::cout << "opened foldernames " << std::endl;
		    //std::cin.ignore();
		    std::string str;
		    bool folderFound = false;
		    while (std::getline(folderNames, str)) {
			    if (str == requestedFolder) {
				    folderFound = true;
			    }
		    }

		    bool folderFound2 = false;

		    //folder exists
		    if (folderFound) {
			    std::string cryptLinksFolderName = requestedFolder + "CryptLinks.enc";
			    std::ifstream cryptLinksFile(cryptLinksFolderName, std::ios::binary);
			    //std::cout << "opened file: " <<cryptLinksFolderName<< std::endl;
			    //std::cin.ignore();
			    char fileData[500];
			    cryptLinksFile.seekg(0, std::ios::end);
			    long fileSize = cryptLinksFile.tellg();
			    cryptLinksFile.seekg(0, std::ios::beg);
			    cryptLinksFile.read((char*)&fileData, fileSize);
			    //std::cout << "read file " << std::endl;
			    //std::cin.ignore();
			    std::string match;
			    int j = 320;
			    while (fileData[j] != ',') {
			    	match += fileData[j];
				    j++;
			    }
			    //std::cout << "parentID: " << match << std::endl;
			    //std::cin.ignore();
			    if (match == currentFolder) {
			    	folderFound2 = true;
			    }
			    cryptLinksFile.close();
		    }

		    //folder is a subfolder of current folder
			if (folderFound2) {
				//send folderFound
				changeFolderInfo = send(clientSock, (char*)&folderFound2, sizeof(bool), 0);
				//std::cout << "sent folder found: " << requestedFolder << std::endl;
				//std::cin.ignore();

				//open file
				std::string cryptLinksFolderName = requestedFolder + "CryptLinks.enc";
				std::ifstream cryptLinksFile(cryptLinksFolderName, std::ios::binary);

				//get file size
				cryptLinksFile.seekg(0, std::ios::end);
				long fileSize = cryptLinksFile.tellg();
				cryptLinksFile.seekg(0, std::ios::beg);

				//send file size
				changeFolderInfo = send(clientSock, (char*)&fileSize, sizeof(long), 0);
				//std::cout << "sent file size: " << fileSize << std::endl;

				// send file data
				char fileData[500];
				cryptLinksFile.read((char*)&fileData, fileSize);
				changeFolderInfo = send(clientSock, (char*)&fileData[0], fileSize, 0);
				//std::cout << "sent file: " << changeFolderInfo << std::endl;
				//std::cin.ignore();
			}
			else {
				//folder not found
				changeFolderInfo = send(clientSock, (char*)&folderFound2, sizeof(bool), 0);
			}
		}
		else {
			std::cout << "invalid client input" << std::endl;
			std::cin.ignore();
		}
	} while (!clientClose);

	return 0;
}
