#pragma once

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <array>
#include <cstdint>
#include<vector>
#include<iostream>

using byte_t = uint8_t;
using blockData = std::vector<byte_t>;
template <size_t N>
using bytes = std::array<byte_t, N>;


// Encryption constants (in bytes)
constexpr int IV = 16;
constexpr int Key = 32;

/*
 * Performs encryption using AES-256-CBC
 * and provides various helper functions
 */
class AES {

	// Encrypts/decrypts a block of data
	static blockData EncryptBlock(bytes<Key> key, bytes<IV> iv, blockData plaintext);
	static blockData DecryptBlock(bytes<Key> key, bytes<IV> iv, blockData ciphertext, size_t blockSize);
	// Encrypts a plaintext into a given ciphertext buffer
	static int EncryptBytes(bytes<Key> key, bytes<IV> iv, byte_t* plaintext, int plen, byte_t* ciphertext);

	// Decrypts a ciphertext, putting the resulting plaintext into a given buffer
	static int DecryptBytes(bytes<Key> key, bytes<IV> iv, byte_t* ciphertext, int clen, byte_t* plaintext);

public:
	static void Setup();
	static void Cleanup();

	static uint32_t GetRandom(uint32_t range);

	// Probabilistically encrypts a block using a
	// random IV, and places it at the start of
	// the ciphertext
	static blockData Encrypt(bytes<Key> key, blockData b);

	// Decrypts a ciphertext which has the IV at
	// the beginning of it
	static blockData Decrypt(bytes<Key> key, blockData b, size_t blockSize);

	// Gets the length of the corresponding ciphertext
	// given the length of a plaintext
	static int GetCiphertextLength(int plen);

	// Generate a randomised initialisation vector
	static bytes<IV> GenerateIV();

	static bytes<Key> GenerateKey();
};

void encryptFile(std::string decName, std::string encName, bytes<Key> key);

void decryptFile(std::string encName, std::string decName, bytes<Key> key);

blockData encryptKey(bytes<Key> encKey, bytes<Key> subjKey);

bytes<Key> decryptKey(bytes<Key> decKey, blockData subjKey);