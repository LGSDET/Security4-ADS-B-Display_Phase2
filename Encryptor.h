//---------------------------------------------------------------------------

#ifndef EncryptorH
#define EncryptorH
//---------------------------------------------------------------------------
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <memory>
#include "CryptoLoader.h"
#include <vector>
#include <fstream>
#include "SecureLog.h"
#include "Util.h"

class Encryptor {
private:
	CryptoLoader& loader;
	void GenerateRandomBytes(std::vector<uint8_t>& buffer, size_t length);
	std::vector<uint8_t> keyBytes;
    std::vector<uint8_t> DeriveKeyFromMac(const std::string& salt);
public:
	Encryptor();
    ~Encryptor();
    AnsiString Encrypt(const AnsiString& plainText); // iv/tag 郴何包府
    AnsiString Decrypt(const AnsiString& encryptedBase64); // 郴何 iv/tag 荤侩

};

#endif
