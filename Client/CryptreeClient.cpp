#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <string.h>
#include "Crypto.h"
#include "CryptreeOrganiser.h"
#include "ClientFolder.h"
#include <ctime>
#include <chrono>


#pragma comment(lib, "ws2_32.lib")

int main() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) {
		std::cerr << "Error starting winsock!" << std::endl;
		return -1;
	}

	SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSock == INVALID_SOCKET) {
		std::cerr << "Error creating socket!, " << WSAGetLastError() << std::endl;
		return -1;
	}

	char serverAddress[NI_MAXHOST];
	memset(serverAddress, 0, NI_MAXHOST);

	std::cout << "Enter server address: ";
	std::cin.getline(serverAddress, NI_MAXHOST);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(55000);
	inet_pton(AF_INET, serverAddress, &hint.sin_addr);

	char welcomeMsg[255];
	if (connect(clientSock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		std::cerr << "Error connect to server!, " << WSAGetLastError() << std::endl;
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	int recvInfo = recv(clientSock, welcomeMsg, 255, 0);
	if (recvInfo == 0 || recvInfo == -1) {
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	printf("%.30s\n", welcomeMsg);

	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];

	//send folder name
	char requestedFolderName[FILENAME_MAX];

	std::cout << "What folder do you require access to?: ";
	std::cin.getline(requestedFolderName, FILENAME_MAX);

	int sndInfo = send(clientSock, (char*)&requestedFolderName, FILENAME_MAX, 0);

	bool routeExists = false;
	bool clientClose = false;
	int recieveCode = 1;
	int sendCode = 2;
	int closeCode = 3;
	int newFolderCode = 4;
	int subfolderCode = 5;
	int changeFolderCode = 6;
	int folderCode = 7;
	int returnCode = 8;
	//master key
	bytes<Key> key = { 0x1f };
	ClientFolder currentFolder(key);

	// does route already exist?
	recv(clientSock, (char*)&routeExists, sizeof(bool), 0);

	//create new route
	if (routeExists == false) {
		std::cout << "Folder does not exists, press enter to close program." << std::endl;
		clientClose = true;
	}
	else if(routeExists == true) {
		//recieve data
		//std::cout << "route exists" << std::endl;
		char cryptLinksData[300];
		blockData cryptLinksData2;
		int info = recv(clientSock, (char*)&cryptLinksData, sizeof(cryptLinksData), 0);
		//std::cout << "receieved data: " << info<< std::endl;
		//std::cin.ignore();
		for (int i = 0; i < 212; i++) {
			cryptLinksData2.push_back(cryptLinksData[i]);
		}

		//recreate route folder from data
		ServerFolder serverFolder(cryptLinksData2);
		//std::cout << "serverFolder created: " << info << std::endl;
		//std::cin.ignore();
		currentFolder = ClientFolder(serverFolder, key);
		//std::cout << "clientFolder created: " << info << std::endl;
		//std::cin.ignore();

	}
	else {
		std::cout << "invalid response" << std::endl;
	}
	
	const int fileAvailable = 200;
	const int fileNotfound = 404;
	std::string recieve = "receive";
	std::string sending = "send";
	std::string close = "close";
	std::string folders = "folders";
	std::string folder = "folder";
	std::string changeFolder = "changefolder";
	std::string subFolders = "subfolders";
	std::string return1 = "return";
	std::string newFolder = "newfolder";
	std::vector<bytes<Key>> subFolderKeys;
	std::string choice = "";
	boolean folderFound;

	do {
		std::cout << "command: " << std::endl;
		std::cin >> choice;  // chose send or recieve

		std::cout <<"given command: "<< choice << std::endl;
		std::cin.ignore();

		if (choice == close) {
		    int closeInfo = send(clientSock, (char*)&closeCode, FILENAME_MAX, 0);
		    clientClose = true;
		    break;
		}

		else if (choice == folder) {
			std::cout << currentFolder.name << std::endl;
		}

		//move to sub folder
		else if (choice == changeFolder) {
			//send change folder code 
			int changeFolderCode = 6;
			int changeFolderInfo = send(clientSock, (char*)&changeFolderCode, sizeof(int), 0);
			//std::cout << "sent: " << changeFolderCode << std::endl;
			//std::cin.ignore();

			//get folder name
			std::string requestedFolderName;
			std::cout << "Folder Name: " << std::endl;
			std::cin >> requestedFolderName;

			//send requested folder name
			changeFolderInfo = send(clientSock, (char*)&requestedFolderName[0], sizeof(std::string), 0);
			//std::cout << "sent requested name: " << requestedFolderName << std::endl;
			//std::cin.ignore();

			//send current folder name
			std::string currentFolderName = currentFolder.name;
			changeFolderInfo = send(clientSock, (char*)&currentFolderName[0], sizeof(std::string), 0);
			//std::cout << "sent current name: " << currentFolderName << std::endl;
			//std::cin.ignore();

			//check for availability
			bool folderFound = false;
			changeFolderInfo = recv(clientSock, (char*)&folderFound, sizeof(bool), 0);
			//std::cout << "recieved folder found " << folderFound << std::endl;
			//std::cin.ignore();

			// folder found
			if (folderFound == true) {

				//recieve data size
				long dataSize = 0;
				changeFolderInfo = recv(clientSock, (char*)&dataSize, sizeof(long), 0);
				//std::cout << "recieved file size: " << dataSize << std::endl;
				//std::cin.ignore();

				//recieve data
				char folderData[500];
				changeFolderInfo = recv(clientSock, (char*)&folderData[0], dataSize, 0);
				//std::cout << "recieved data: " << changeFolderInfo << std::endl;
				//std::cin.ignore();

				//convert to blockData
				blockData folderData2;
				for (int i = 0; i < dataSize; i++) {
					folderData2.push_back(folderData[i]);
				}

				ServerFolder newFolderS(folderData2);
				ClientFolder newFolderC(newFolderS, currentFolder.SK);
				subFolderKeys.push_back(currentFolder.SK);
				currentFolder = newFolderC;
				std::cout << "current folder: " << currentFolder.name << std::endl;
			}
			else { 
				std::cout << "folder does not exist" << std::endl;
			}
		}

		else if (choice == return1) {

			if (currentFolder.name != "Cryptree") {
				//send return code
				int returnInfo = send(clientSock, (char*)&returnCode, sizeof(int), 0);

				//send folder name length
				long nameLength = currentFolder.parentName.length();
				returnInfo = send(clientSock, (char*)&nameLength, sizeof(long), 0);

				//send parent folder name
				char parentFolderName[FILENAME_MAX]; 
				copyStringToArray(currentFolder.parentName, parentFolderName);
				returnInfo = send(clientSock, (char*)&parentFolderName, nameLength, 0);

				//std::cout << "sent parent name: " << parentFolderName << std::endl;
				//std::cin.ignore();

				//recieve folder data size
				long cryptLinkDataSize = 0;
				returnInfo = recv(clientSock, (char*)&cryptLinkDataSize, sizeof(long), 0);

				//std::cout << "recieved folder data size: " << cryptLinkDataSize << std::endl;
				//std::cin.ignore();

				//recieve new folder data
				char folderCryptLinkData[500];
				returnInfo = recv(clientSock, (char*)&folderCryptLinkData, cryptLinkDataSize, 0);

				//std::cout << "recieved folderData " << returnInfo << std::endl;
				//std::cin.ignore();

				//std::cout << "printed" << std::endl;
				//std::cin.ignore();

				//find appropriate key and erase from subkey vector
				int keyPos = subFolderKeys.size() - 1;
				bytes<Key> currentKey = subFolderKeys[keyPos];
				subFolderKeys.erase(subFolderKeys.end() - 1);

				//std::cout << "retrieved key " << currentKey[0] << std::endl;
				//std::cin.ignore();

				//convert to clientfolder
				blockData folderCryptLinkData2;
				for (int i = 0; i < cryptLinkDataSize; i++) {
					folderCryptLinkData2.push_back(folderCryptLinkData[i]);
				}

				//std::cout << "converted block data"  << std::endl;
				//std::cin.ignore();

				ServerFolder serverFolder(folderCryptLinkData2);

				//std::cout << "created server folder "<< std::endl;
				//std::cin.ignore();

				ClientFolder newFolder(serverFolder, currentKey);

				//std::cout << "created client folder "<< std::endl;
				//std::cin.ignore();

				//set as new folder
				currentFolder = newFolder;

				//print out current folder name
				std::cout << "current folder: " << currentFolder.name << std::endl;
			}
			else {
				std::cout << "you are in route folder" << std::endl;
			}
		}

		else if (choice == subFolders) {
			// send client code
			int subfolderInfo = send(clientSock, (char*)&subfolderCode, sizeof(int), 0);

			// send folder name
			std::string folderName = currentFolder.name;
			subfolderInfo = send(clientSock, (char*)&folderName[0], sizeof(std::string), 0);

			//recieve number of subfolders
			int subfolderNum = 0;
			subfolderInfo = recv(clientSock, (char*)&subfolderNum, sizeof(int), 0);
			if (subfolderNum == 0) {
				std::cout << "no subfolders" << std::endl;
			}

			//recieve subfolders
			else {
				std::vector<std::string> subFolders;
				for (int i = 0; i < subfolderNum; i++) {
					std::string str;
					subfolderInfo = recv(clientSock, (char*)&str, sizeof(std::string), 0);
					std::cout << str << std::endl;
					//program randomly shuts down here 
				}
			}
		}


		//create new folder
		if (choice == newFolder) {
			int sendInfo = send(clientSock, (char*)&newFolderCode, sizeof(int), 0);

			//initialize
			std::string folderName;
			std::cout << "Folder Name: " << std::endl;
			std::cin >> folderName;

			ClientFolder newFolder(folderName, currentFolder.name);
			std::string folderDataFileNameEnc = folderName + "Data.enc";
			std::string folderDataFileNameTxt = folderName + "Data.txt";

			//create and encrypt data file
			std::ofstream dataFile(folderDataFileNameTxt, std::fstream::app);
			dataFile << folderName << std::endl;
			encryptFile(folderDataFileNameTxt, folderDataFileNameEnc, newFolder.DK);
			std::ifstream folderDataFile(folderDataFileNameEnc, std::ios::binary);
			if (folderDataFile.is_open()) {

			}

			//send foldername
			int folderInfo = send(clientSock, (char*)&folderDataFileNameEnc[0], sizeof(std::string), 0);
			//std::cout << "sent file name: " << folderDataFileNameEnc << std::endl;
			//std::cin.ignore();
			//std::cin.ignore();

			//get file size
			folderDataFile.seekg(0, std::ios::end);
			long folderDataFileSize = folderDataFile.tellg();
			folderDataFile.seekg(0, std::ios::beg);

			// send filesize to client
			folderInfo = send(clientSock, (char*)&folderDataFileSize, sizeof(long), 0);
			//std::cout << "sent file size: " << folderDataFileSize << std::endl;
			//std::cin.ignore();

			//send file data
			char fileData[100];
			folderDataFile.read((char*)&fileData, folderDataFileSize);
			folderInfo = send(clientSock, (char*)&fileData, folderDataFileSize, 0);
			//std::cout << "sent file: " << folderInfo << std::endl;
			//std::cin.ignore();

			//clean up
			folderDataFile.close();
			dataFile.close();
			remove((char*)&folderDataFileNameEnc[0]);
			remove((char*)&folderDataFileNameTxt[0]);

			//convert folder to send form
			ServerFolder newFolderS = newFolder.convert(currentFolder);
			blockData cryptLinksData = newFolderS.convertToSingleBlock();
			char fileBlockData[500];
			for (int i = 0; i < cryptLinksData.size(); i++) {
				fileBlockData[i] = cryptLinksData[i];
			}

			//send file name
			std::string cryptLinksFileName = folderName + "CryptLinks.enc";
			folderInfo = send(clientSock, (char*)&cryptLinksFileName[0], sizeof(std::string), 0);
			//std::cout << "sent file name: " << cryptLinksFileName << std::endl;
			//std::cin.ignore();

			//send file size
			long cryptLinkFileSize = cryptLinksData.size();
			folderInfo = send(clientSock, (char*)&cryptLinkFileSize, sizeof(long), 0);
			//std::cout << "sent file size: " << cryptLinkFileSize << std::endl;
			//std::cin.ignore();

			// send file data
			folderInfo = send(clientSock, (char*)&fileBlockData[0], cryptLinkFileSize, 0);
			//std::cout << "sent file: " << folderInfo << std::endl;
			//std::cin.ignore();

			//clean up
			remove((char*)&cryptLinksFileName[0]);
			std::cout << folderName << " created" << std::endl;
		}


		//recieve file
		else if (choice == recieve) {

			//send recieve code
			int recvInfo = send(clientSock, (char*)&recieveCode, sizeof(int), 0);

			int fileDownloaded = 0;

			//get file name
			char recievingFile[FILENAME_MAX];
			std::cout << "Enter file name: " << std::endl;
			std::cin >> recievingFile;

			//get encFileName
			std::string encFileName = getEncFileName(recievingFile, currentFolder.name);

			//send enc file name
			recvInfo = send(clientSock, (char*)&encFileName[0], FILENAME_MAX , 0);

			//recieve code available
			int codeAvailable;
			recvInfo = recv(clientSock, (char*)&codeAvailable, sizeof(int), 0);

			if (codeAvailable == fileAvailable) {
				//recieve filesize
				long fileRequestedSize = 0;
				recvInfo = recv(clientSock, (char*)&fileRequestedSize, sizeof(long), 0);

				std::ofstream file;
				file.open(encFileName, std::ios::binary | std::ios::trunc);

				//recieve and write to file
				do {
					memset(bufferFile, 0, BUFFER_SIZE);
					recvInfo = recv(clientSock, bufferFile, BUFFER_SIZE, 0);
					file.write(bufferFile, recvInfo);
					fileDownloaded += recvInfo;
				} while (fileDownloaded < fileRequestedSize);
				file.close();
				decryptFile(encFileName,recievingFile,currentFolder.SK);
				remove((char*)&encFileName[0]);
			}
			else if (codeAvailable == 404) {
				std::cout << "Can't open file or file not found!" << std::endl;
			}
		}
		//send file
		else if (choice == sending) {

		    //get filename
		    char sendingFile[FILENAME_MAX];
		    std::cout << "Enter file name: " << std::endl;
		    std::cin >> sendingFile;

			std::ifstream file(sendingFile, std::ios::binary);
			if (file.good()) {

				//send send code
				int sendInfo = send(clientSock, (char*)&sendCode, sizeof(int), 0);

				//get enc file name
				std::string encFileName = getEncFileName(sendingFile, currentFolder.name);

				//encrypt file
				encryptFile(sendingFile, encFileName, currentFolder.SK);

				//open file
				std::ifstream encFile(encFileName, std::ios::binary);

				// send enc filename 
				sendInfo = send(clientSock, (char*)&encFileName[0], sizeof(std::string), 0);


				if (encFile.is_open()) {
					encFile.seekg(0, std::ios::end);
					long fileSize = encFile.tellg();
					encFile.seekg(0, std::ios::beg);


					// send filesize to client
					sendInfo = send(clientSock, (char*)&fileSize, sizeof(long), 0);

					do {
						// read and send part file to client
						encFile.read(bufferFile, BUFFER_SIZE);
						if (encFile.gcount() > 0) {
							sendInfo = send(clientSock, bufferFile, encFile.gcount(), 0);
						}
					} while (encFile.gcount() > 0);
					encFile.close();
					std::cout << sendingFile << " sent to server." << std::endl;
					std::remove((char*)&encFileName[0]);
			    }

			}
			else {
				std::cout << "file not found" << std::endl;
			}
		}

		else {
        }
	} while (!clientClose);
	closesocket(clientSock);
	WSACleanup();
	return 0;
}
