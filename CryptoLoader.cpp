//---------------------------------------------------------------------------

#pragma hdrstop

#include "CryptoLoader.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// CryptoLoader.cpp
#include "CryptoLoader.h"
#include <tchar.h>
#include <mutex>

CryptoLoader& CryptoLoader::Instance() {
    static CryptoLoader instance;
    return instance;
}

CryptoLoader::~CryptoLoader() {
    //Unload();
}

bool CryptoLoader::Load() {

    if (hCrypto)
        return true;

    SetDllDirectory(TEXT("C:\\OpenSSL-Win64\\bin"));
    hCrypto = LoadLibraryW(L"C:\\OpenSSL-Win64\\bin\\libcrypto-3-x64.dll");

    if (!hCrypto) {
		MessageBox(nullptr, _T("libcrypto DLL 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		printf("libcrypto DLL 로드 실패!");
        return false;
    }

    // 함수 포인터 로딩
    RAND_bytes = (decltype(RAND_bytes))GetProcAddress(hCrypto, "RAND_bytes");
    EVP_CIPHER_CTX_new = (decltype(EVP_CIPHER_CTX_new))GetProcAddress(hCrypto, "EVP_CIPHER_CTX_new");
    EVP_CIPHER_CTX_ctrl = (decltype(EVP_CIPHER_CTX_ctrl))GetProcAddress(hCrypto, "EVP_CIPHER_CTX_ctrl");
    EVP_EncryptInit_ex = (decltype(EVP_EncryptInit_ex))GetProcAddress(hCrypto, "EVP_EncryptInit_ex");
    EVP_EncryptUpdate = (decltype(EVP_EncryptUpdate))GetProcAddress(hCrypto, "EVP_EncryptUpdate");
    EVP_EncryptFinal_ex = (decltype(EVP_EncryptFinal_ex))GetProcAddress(hCrypto, "EVP_EncryptFinal_ex");
    EVP_CIPHER_CTX_free = (decltype(EVP_CIPHER_CTX_free))GetProcAddress(hCrypto, "EVP_CIPHER_CTX_free");
    EVP_DecryptInit_ex = (decltype(EVP_DecryptInit_ex))GetProcAddress(hCrypto, "EVP_DecryptInit_ex");
    EVP_DecryptUpdate = (decltype(EVP_DecryptUpdate))GetProcAddress(hCrypto, "EVP_DecryptUpdate");
    EVP_DecryptFinal_ex = (decltype(EVP_DecryptFinal_ex))GetProcAddress(hCrypto, "EVP_DecryptFinal_ex");
	EVP_aes_128_gcm = (decltype(EVP_aes_128_gcm))GetProcAddress(hCrypto, "EVP_aes_128_gcm");
	SHA256 = (decltype(SHA256))GetProcAddress(hCrypto, "SHA256");
	EVP_sha256 = (decltype(EVP_sha256))GetProcAddress(hCrypto, "EVP_sha256");
	HMAC = (decltype(HMAC))GetProcAddress(hCrypto, "HMAC");
    if (!RAND_bytes || !EVP_CIPHER_CTX_new || !EVP_CIPHER_CTX_ctrl || !EVP_EncryptInit_ex ||
		!EVP_EncryptUpdate || !EVP_EncryptFinal_ex || !EVP_CIPHER_CTX_free ||
		!EVP_DecryptInit_ex || !EVP_DecryptUpdate || !EVP_DecryptFinal_ex || !EVP_aes_128_gcm || !SHA256 || !EVP_sha256 || !HMAC) {
        Unload();
        return false;
    }

    return true;
}

void CryptoLoader::Unload() {
    if (hCrypto) {
        FreeLibrary(hCrypto);
        hCrypto = nullptr;

        // 포인터 초기화
        RAND_bytes = nullptr;
        EVP_CIPHER_CTX_new = nullptr;
        EVP_CIPHER_CTX_ctrl = nullptr;
        EVP_EncryptInit_ex = nullptr;
        EVP_EncryptUpdate = nullptr;
        EVP_EncryptFinal_ex = nullptr;
        EVP_CIPHER_CTX_free = nullptr;
        EVP_DecryptInit_ex = nullptr;
        EVP_DecryptUpdate = nullptr;
        EVP_DecryptFinal_ex = nullptr;
		EVP_aes_128_gcm = nullptr;
		SHA256 = nullptr;
		EVP_sha256 = nullptr;
        HMAC = nullptr;
    }
}
