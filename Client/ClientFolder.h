#include "Crypto.h"
#include "ServerFolder.h"
#include "CryptreeOrganiser.h"

class ClientFolder {

public:

	std::string name;
	std::string parentName;
	bytes<Key> DK;
	bytes<Key> BK;
	bytes<Key> SK;
	bytes<Key> FK;

	//route folder
	ClientFolder(bytes<Key> key) {
		this->parentName = "no parent";
		this->name = "Cryptree";
		DK = AES::GenerateKey();
		BK = AES::GenerateKey();
		SK = key; 
		FK = AES::GenerateKey();
	}
	// not finished
	ClientFolder(std::string name, std::string parent) {
		this->parentName = parent;
		this->name = name;
		DK = AES::GenerateKey();
		BK = AES::GenerateKey();
		SK = AES::GenerateKey();
		FK = AES::GenerateKey();
	}

	//works for route may not work for sub folders
	ClientFolder(ServerFolder serverFolder, bytes<Key> key) {
		//route folder
		if (serverFolder.BK_BKp.size() == 1) {
			for (int i = 0; i < serverFolder.folderID.size(); i++) {
				this->name += serverFolder.folderID[i];
			}
			for (int i = 0; i < serverFolder.parentID.size(); i++) {
				this->parentName += serverFolder.parentID[i];
			}
			SK = key;
			FK = decryptKey(SK, serverFolder.SK_FK);
			BK = decryptKey(SK, serverFolder.SK_BK);
			DK = decryptKey(BK, serverFolder.BK_DK);
		}
		//all other folders
		else {
			for (int i = 0; i < serverFolder.folderID.size(); i++) {
				this->name += serverFolder.folderID[i];
			}
			for (int i = 0; i < serverFolder.parentID.size(); i++) {
				this->parentName += serverFolder.parentID[i];
			}
			SK = decryptKey(key, serverFolder.SKp_SK);
			BK = decryptKey(SK, serverFolder.SK_BK);
			DK = decryptKey(BK, serverFolder.BK_DK);
			FK = decryptKey(SK, serverFolder.SK_FK);
		}

	}

	ServerFolder convertRoute() {
		return ServerFolder(encryptKey(BK, DK), { 0x00 }, encryptKey(SK, BK), { 0x00 }, encryptKey(SK, FK), name, parentName);
	}

	ServerFolder convert(ClientFolder parent) {
		return ServerFolder(encryptKey(BK, DK), encryptKey(BK, parent.BK), encryptKey(SK, BK), encryptKey(parent.SK, SK), encryptKey(SK, FK), name, parentName);
	}

	

	bool equals(ClientFolder check) {
		if (DK == check.DK) {
			if (BK == check.BK) {
				if (SK == check.SK) {
					if (FK == check.FK) {
						if (parentName == check.parentName) {
							if (name == check.name) {
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

	
};