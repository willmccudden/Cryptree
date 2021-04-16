#include "Crypto.h"

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include<conio.h>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include "Block.h"
#include "CryptreeOrganiser.h"

void AES::Setup()
{
	// Initialise OpenSSL
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_no_config();
}

void AES::Cleanup()
{
	EVP_cleanup();
	ERR_free_strings();
}

uint32_t AES::GetRandom(uint32_t range)
{
	uint8_t buffer[4];

	if (!(RAND_bytes((uint8_t*)buffer, sizeof(buffer)))) {
		std::cout << "needs more entropy" << std::endl;
	}
	uint32_t rand = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0]);
	return (abs((int)rand) % (uint32_t)range);
}


int AES::EncryptBytes(bytes<Key> key, bytes<IV> iv, byte_t* plaintext, int plen, byte_t* ciphertext)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	if (!ctx) {
		std::cout << "failed to create new cipher" << std::endl;
	}

	// Initialise the encryption operation
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1) {
		std::cout << "Failed to initialise encryption" << std::endl;
	}

	// Encrypt
	int len;
	if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plen) != 1) {
		std::cout << "Failed to complete EncryptUpdate" << std::endl;
	}

	int clen = len;

	if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
		std::cout << "Failed to complete EncryptFinal" << std::endl;
	}
	clen += len;

	EVP_CIPHER_CTX_free(ctx);

	return clen;
}

int AES::DecryptBytes(bytes<Key> key, bytes<IV> iv, byte_t* ciphertext, int clen, byte_t* plaintext)
{

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	if (!ctx) {
		std::cout << "Failed to create new cipher" << std::endl;
	}

	// Initialise the decryption operation
	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1) {
		std::cout << "Failed to initialise decryption" << std::endl;
	}

	// Decrypt
	int len;
	if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, clen) != 1) {
		std::cout << "Failed to complete DecryptUpdate" << std::endl;
		return -1;
	}

	int plen = len;

	if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
		return -1;
		std::cout << "failed" << std::endl;
	}
	plen += len;

	EVP_CIPHER_CTX_free(ctx);

	return plen;
}

blockData AES::EncryptBlock(bytes<Key> key, bytes<IV> iv, blockData plaintext)
{
	int clen = GetCiphertextLength(plaintext.size());
	blockData ciphertext(clen);

	int plen = plaintext.size();
	EncryptBytes(key, iv, plaintext.data(), plen, ciphertext.data());

	return ciphertext;
}

blockData AES::DecryptBlock(bytes<Key> key, bytes<IV> iv, blockData ciphertext, size_t blockSize)
{
	int clen = ciphertext.size();
	blockData plaintext(clen);

	int plen = DecryptBytes(key, iv, ciphertext.data(), clen, plaintext.data());

	// Trim plaintext to actual size
	if (plen == -1) {
		return Block(-1, 1, 0, blockData(blockSize)).toBytes();
		std::cout << "decryptBlock failed" << std::endl;
	}
	plaintext.resize(plen);

	return plaintext;
}

blockData AES::Encrypt(bytes<Key> key, blockData plaintext)
{
	bytes<IV> iv = AES::GenerateIV();

	blockData ciphertext = EncryptBlock(key, iv, plaintext);

	// Put randomised IV at the front of the ciphertext
	ciphertext.insert(ciphertext.begin(), iv.begin(), iv.end());

	return ciphertext;
}

blockData AES::Decrypt(bytes<Key> key, blockData ciphertext, size_t blockSize)
{
	// Extract the IV
	bytes<IV> iv;
	std::copy(ciphertext.begin(), ciphertext.begin() + IV, iv.begin());

	ciphertext.erase(ciphertext.begin(), ciphertext.begin() + IV);

	// Perform the decryption
	blockData plaintext = DecryptBlock(key, iv, ciphertext, blockSize);

	return plaintext;
}

int AES::GetCiphertextLength(int plen)
{
	// Round up to the next 16 bytes (due to padding)
	return (plen / 16 + 1) * 16;
}

bytes<IV> AES::GenerateIV()
{
	bytes<IV> iv;

	if (RAND_bytes(iv.data(), iv.size()) != 1) {
		// Bytes generated aren't cryptographically strong
		std::cout << "needs more entropy" << std::endl;
	}

	return iv;
}

bytes<Key> AES::GenerateKey()
{
	bytes<Key> key;

	if (RAND_bytes(key.data(), key.size()) != 1) {
		// Bytes generated aren't cryptographically strong
		std::cout << "needs more entropy" << std::endl;
	}

	return key;
}

void encryptFile(std::string decName ,std::string encName, bytes<Key> key) {
	std::ifstream file(decName, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Failed to open file: " << decName << std::endl;
		std::cin.ignore();
	    return;
	}
	std::ofstream encFileIn(encName, std::ios::binary);
	if (!encFileIn.is_open()) {
		std::cout << "Failed to open file: " << encName << std::endl;
		std::cin.ignore();
		return;
	}
	file.seekg(0, std::ios::end);
	long fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<unsigned char> fileData(fileSize);
	std::vector<unsigned char> cipherData(AES::GetCiphertextLength(fileSize));
	file.read((char*)&fileData[0], fileSize);
	cipherData = AES::Encrypt(key, fileData);
	std::copy(cipherData.begin(), cipherData.end(), std::ostream_iterator<unsigned char>(encFileIn));
	encFileIn.close();
	file.close();
}

void decryptFile(std::string encName  , std::string decName, bytes<Key> key) {
	std::ifstream encFile(encName, std::ios::binary);
	std::ofstream decFile(decName, std::ios::binary);
	encFile.seekg(0, std::ios::end);
	int encFileSize = encFile.tellg();
	encFile.seekg(0, std::ios::beg);
	std::vector<unsigned char> cipherData2(encFileSize);
	std::vector<unsigned char> decData(encFileSize);
	encFile.read((char*)&cipherData2[0], AES::GetCiphertextLength(encFileSize));
	decData = AES::Decrypt(key, cipherData2, encFileSize);
	std::copy(decData.begin(), decData.end(), std::ostream_iterator<unsigned char>(decFile));
}

blockData encryptKey(bytes<Key> encKey, bytes<Key> subjKey) {
	blockData subjKey2;
	for (int i = 0; i < subjKey.size(); i++) {
		subjKey2.push_back(subjKey[i]);
	}
	return AES::Encrypt(encKey, subjKey2);
}

bytes<Key> decryptKey(bytes<Key> decKey, blockData subjKey) {
	blockData result = AES::Decrypt(decKey, subjKey, Key);
	bytes<Key> result2{};
	for (int i = 0; i < Key; i++) {
		result2[i] = result[i];
	}
	return result2;

}


