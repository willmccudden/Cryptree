#include "Crypto.h"

#include <iostream>

class ServerFolder {
public:
	blockData BK_DK;
	blockData BK_BKp;
	blockData SK_BK;
	blockData SKp_SK;
	blockData SK_FK;
	blockData parentID;
	blockData folderID;
	// route folder
	ServerFolder(blockData BK_DK, blockData BK_BKp, blockData SK_BK, blockData SKp_SK, blockData SK_FK, std::string name, std::string parentName) {
		this->BK_DK = BK_DK;
		this->BK_BKp = BK_BKp;
		this->SK_BK = SK_BK;
		this->SKp_SK = SKp_SK;
		this->SK_FK = SK_FK;
		blockData vector(parentName.begin(), parentName.end());
		this->parentID = vector;
		blockData vector2(name.begin(), name.end());
		this->folderID = vector2;
	}

	ServerFolder(blockData singleBlock) {
		if (singleBlock.size() ==212) {
			blockData::const_iterator first = singleBlock.begin();
			blockData::const_iterator last = singleBlock.begin() + 64;
			blockData data(first, last);
			this->BK_DK = data;
			first = singleBlock.begin() + 65;
			last = singleBlock.begin() + 129;
			blockData data1(first, last);
			this->SK_BK = data1;
			first = singleBlock.begin() + 130;
			last = singleBlock.begin() + 194;
			blockData data2(first, last);
			this->SK_FK = data2;
			this->SKp_SK = { 0x00 };
			this->BK_BKp = { 0x00 };
			blockData parentID;
			int j = 194;
			while (singleBlock[j] != ',') {
				parentID.push_back(singleBlock[j]);
				j++;
			}
			this->parentID = parentID;

			j++;
			blockData folderID;
			for (int i = j; i < singleBlock.size(); i++) {
				folderID.push_back(singleBlock[i]);
			}
			this->folderID = folderID;
		}
		else {
			int size = singleBlock.size();
			blockData::const_iterator first = singleBlock.begin();
			blockData::const_iterator last = singleBlock.begin() + 64;
			blockData data(first, last);
			this->BK_DK = data;
			first = singleBlock.begin() + 64;
			last = singleBlock.begin() + 128;
			blockData data1(first, last);
			this->BK_BKp = data1;
			first = singleBlock.begin() + 128;
			last = singleBlock.begin() + 192;
			blockData data2(first, last);
			this->SK_BK = data2;
			first = singleBlock.begin() + 192;
			last = singleBlock.begin() + 256;
			blockData data3(first, last);
			this->SKp_SK = data3;
			first = singleBlock.begin() + 256;
			last = singleBlock.begin() + 319;
			blockData data4(first, last);
			this->SK_FK = data4;
			blockData parentID;
			int j = 320;
			while (singleBlock[j] != ',') {
				parentID.push_back(singleBlock[j]);
				j++;
			}
			this->parentID = parentID;
			j++;
			blockData folderID;
			for (int i = j; i < singleBlock.size(); i++) {
				folderID.push_back(singleBlock[i]);
			}
			this->folderID = folderID;
		}
	}

	blockData convertToSingleBlock(){
		blockData single;
		//std::cout << BK_DK.size() << std::endl;
		for (int i = 0; i < BK_DK.size(); i++) {
			single.push_back(BK_DK[i]);
		}
		//std::cout << BK_BKp.size() << std::endl;
		for (int i = 0; i < BK_BKp.size(); i++) {
			single.push_back(BK_BKp[i]);
		}
		//std::cout << SK_BK.size() << std::endl;
		for (int i = 0; i < SK_BK.size(); i++) {
			single.push_back(SK_BK[i]);
		}
		//std::cout << SKp_SK.size() << std::endl;
		for (int i = 0; i < SKp_SK.size(); i++) {
			single.push_back(SKp_SK[i]);
		}
		//std::cout << SK_FK.size() << std::endl;
		for (int i = 0; i < SK_FK.size(); i++) {
			single.push_back(SK_FK[i]);
		}
		//std::cout << parentID.size() << std::endl;
		for (int i = 0; i < parentID.size(); i++) {
			single.push_back(parentID[i]);
		}
		single.push_back(',');
		//std::cout << folderID.size() << std::endl;
		for (int i = 0; i < folderID.size(); i++) {
			single.push_back(folderID[i]);
		}
		return single;
	}

	bool equals(ServerFolder check) {
		if (BK_DK == check.BK_DK) {
			if (BK_BKp == check.BK_BKp) {
				if (SK_BK == check.SK_BK) {
					if (SKp_SK == check.SKp_SK) {
						if (SK_FK == check.SK_FK) {
							if (folderID == check.folderID) {
								if (parentID == check.parentID) {
									return true;
								}
								else {
									return false;
								}
							}
							else {
								return false;
							}
						}
						else {
							return false;
						}
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
};
