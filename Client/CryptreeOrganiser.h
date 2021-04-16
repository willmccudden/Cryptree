#pragma once

#include<iostream>
#include <vector>

std::string getEncFileName(std::string fileName, std::string folderName);

std::string getDecFileName(std::string fileName, std::string folderName);

std::string getParentFolder(std::string currentFolder);

std::string serverFileName(std::string encName);

std::string removeServerName(std::string serverName);

void copyStringToArray(std::string str, char array[]);