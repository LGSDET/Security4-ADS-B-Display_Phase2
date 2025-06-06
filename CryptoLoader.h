//---------------------------------------------------------------------------

#ifndef CryptoLoaderH
#define CryptoLoaderH
//---------------------------------------------------------------------------

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <windows.h>
#include "SecureLog.h"

class CryptoLoader {
private:
    HMODULE hCrypto = nullptr;
    CryptoLoader() = default;

public:
    static CryptoLoader& Instance();
    bool Load();
    void Unload();
    ~CryptoLoader();

    // 복사 금지
    CryptoLoader(const CryptoLoader&) = delete;
    CryptoLoader& operator=(const CryptoLoader&) = delete;

    // OpenSSL libcrypto 함수 포인터
    decltype(::RAND_bytes)* RAND_bytes = nullptr;
    decltype(::EVP_CIPHER_CTX_new)* EVP_CIPHER_CTX_new = nullptr;
    decltype(::EVP_CIPHER_CTX_ctrl)* EVP_CIPHER_CTX_ctrl = nullptr;
    decltype(::EVP_EncryptInit_ex)* EVP_EncryptInit_ex = nullptr;
    decltype(::EVP_EncryptUpdate)* EVP_EncryptUpdate = nullptr;
    decltype(::EVP_EncryptFinal_ex)* EVP_EncryptFinal_ex = nullptr;
    decltype(::EVP_CIPHER_CTX_free)* EVP_CIPHER_CTX_free = nullptr;
    decltype(::EVP_DecryptInit_ex)* EVP_DecryptInit_ex = nullptr;
    decltype(::EVP_DecryptUpdate)* EVP_DecryptUpdate = nullptr;
    decltype(::EVP_DecryptFinal_ex)* EVP_DecryptFinal_ex = nullptr;
	decltype(::EVP_aes_128_gcm)* EVP_aes_128_gcm = nullptr;
	decltype(::SHA256)* SHA256 = nullptr;
	decltype(::EVP_sha256)* EVP_sha256 = nullptr;
	decltype(::HMAC)* HMAC = nullptr;

};

#endif // CRYPTO_LOADER_H