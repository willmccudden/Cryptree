#pragma once
#include<iostream>

void copyStringToArray(std::string str, char array[]) {
	for (int i = 0; i < str.length(); i++) {
		array[i] = str[i];
	}
	return;
}

std::string convertToString(char* p, int size) {
	int i = 0;
	std::string s = "";
	for (int i = 0; i < size; i++) {
		s = s += p[i];
	}
	return s;
}