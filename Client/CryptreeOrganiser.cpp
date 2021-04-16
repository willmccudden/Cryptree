#include "CryptreeOrganiser.h"

//get enc folder name
std::string getEncFileName(std::string fileName, std::string folderName) {
	std::string result = folderName + ",,";
	int i = 0;
	while (fileName[i] != '.') {
		result += fileName[i];
		i++;
	}
	result += "__";
	i++;
	while (i < fileName.length()) {
		result += fileName[i];
		i++;
	}
	result += ".enc";
	return result;
}

//get dec folder name
std::string getDecFileName(std::string fileName, std::string folderName) {
	std::string result;
	std::string str;
	int prefixLength = folderName.length()+2;
	for (int i = prefixLength - 1; i < fileName.length(); i++) {
		str += fileName;
	}
	int j = 0;
	while (str[j] != '_') {
		result += str[j];
		j++;
	}
	j = j + 2;
	result += '.';
	while (str[j] != '.') {
		result += str[j];
		j++;
	}
	return result;
}

//gets names of folders within a given folder
std::vector<std::string> getFolders(std::vector<std::string> availableFolders, std::string folderName) {
	int slashes=0;
	int slashes2 = 0;
	std::vector <std::string> folders;
	std::vector<std::string> availableFoldersFixed(availableFolders.size());
	for (int i = 0; i < availableFolders.size(); i++) {
		availableFoldersFixed[i] = "Cryptree/" + availableFolders[i];
	}
	for (int i = 0; i < folderName.length(); i++) {
		if (folderName[i] == '/') {
			slashes++;
		}
	}
	for (int i = 0; i < availableFoldersFixed.size(); i++) {
		for (int j = 0; j < availableFoldersFixed[i].length(); j++) {
			if (availableFoldersFixed[i][j] == '/') {
				slashes2++;
			}
		}
		if ((availableFoldersFixed[i].find(folderName) != std::string::npos)&&(slashes + 1 == slashes2)) {
			folders.push_back(availableFolders[i]);
		}
		slashes2 = 0;
	}
	return folders;
}

std::string getEncFileName(char fileName[FILENAME_MAX]) {
	std::string enc = ".enc";
	std::string encFileName;
	int i = 0;
	while (fileName[i] != '.') {
		encFileName += fileName[i];
		i++;
	}
	encFileName += "__";
	i++;
	while (fileName[i] != 0) {
		encFileName += fileName[i];
		i++;
	}
	encFileName += enc;
	return encFileName;
}

std::string convertToString(char* p, int size) {
	int i = 0;
	std::string s = "";
	for(int i = 0; i < size;i++){
		s = s += p[i];
	}
	return s;
}


//gets folder name without dash and parent folder
std::string getShortFolderName(std::string currentFolder, std::string subFolder) {
	std::string str;
	if (currentFolder == "Cryptree/") {
		for (int i = 0; i < subFolder.length() - 1; i++) {
			str += subFolder[i];
		}
	}
	else {
		for (int i = currentFolder.length()-9; i < subFolder.length() - 1; i++) {
			str += subFolder[i];
		}
	}
	
	return str;
}
// gets parent folder in / form
std::string getParentFolder(std::string currentFolder) {
    std::string str;
	int a=0;
	for (int b=0; b < currentFolder.length(); b++) {
		if (currentFolder[b] == '/') {
			a++;
		}
	}
	if (a == 1) {
		return "Cryptree/";
	}
	int i = currentFolder.length()-1;
	int j = 0;
	while (currentFolder[i-1] != '/') {
		j++;
		i--;
	}
	j++;
	for (int k = 0; k < currentFolder.length() - j; k++) {
		str += currentFolder[k];
	}
	return str;
}

std::string serverFileName(std::string encName) {
	std::string str;
	for (int i = 0; i < encName.size(); i++) {
		if (encName[i] == '/') {
			str += ",,";
		}
		else {
			str += encName[i];
		}
	}
	return str;
}

std::string removeServerName(std::string serverName) {
	std::string str;
	int a=0;
	for (int i = serverName.size()-1; i >0; i--) {
		if (serverName[i] == ',') {
			a = i;
			break;
		}
	}
	a = a + 1;
	for (int i = a; i < serverName.size(); i++) {
		str += serverName[i];
	}
	return str;
}

void copyStringToArray (std::string str, char array[]) {
	for (int i = 0; i < str.length(); i++) {
		array[i] = str[i];
	}
	return;
}
