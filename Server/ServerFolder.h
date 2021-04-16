
#include <iostream>
#include <vector>
//server side version
class ServerFolder {

	using byte_t = uint8_t;
	using blockData = std::vector<byte_t>;

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
		if (singleBlock.size() < 320) {
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
			while (singleBlock[j] != (byte_t)',') {
				parentID.push_back(singleBlock[j]);
				j++;
			}
			this->parentID = parentID;
			j++;
			blockData folderID;
			folderID.insert(folderID.begin(), singleBlock.begin() + j, singleBlock.end());
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
			this->folderID = { singleBlock[320] };
			for (int i = 0; i < size - 321; i++) {
				this->parentID[i] = singleBlock[i + 321];
			}
		}
	}

	blockData convertToSingleBlock() {
		blockData single;
		single.insert(single.end(), BK_DK.begin(), BK_DK.end());
		single.insert(single.end(), BK_BKp.begin(), BK_BKp.end());
		single.insert(single.end(), SK_BK.begin(), SK_BK.end());
		single.insert(single.end(), SKp_SK.begin(), SKp_SK.end());
		single.insert(single.end(), SK_FK.begin(), SK_FK.end());
		single.insert(single.end(), parentID.begin(), parentID.end());
		//inserts a coma between folderID and parentID
		single.insert(single.end(), (byte_t)',');
		single.insert(single.end(), folderID.begin(), folderID.end());
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
