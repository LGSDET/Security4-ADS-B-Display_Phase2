//---------------------------------------------------------------------------

#pragma hdrstop

#include "Encryptor.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

Encryptor::Encryptor() : loader(CryptoLoader::Instance())
{
	if (!loader.Load()) {
		MessageBox(nullptr, _T("Crypto DLL 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
    }

    std::string salt = "MySalt123";  // 보안 강화를 위한 고정된 salt
    keyBytes = DeriveKeyFromMac(salt);

    if (keyBytes.size() < 16) {
        std::hash<std::string> hasher;
        size_t hashVal = hasher(std::string(keyBytes.begin(), keyBytes.end()));

        while (keyBytes.size() < 16) {
            uint8_t byte = static_cast<uint8_t>(
                (hashVal >> ((keyBytes.size() % sizeof(size_t)) * 8)) & 0xFF);
            keyBytes.push_back(byte);
        }
    } else if (keyBytes.size() > 16) {
        keyBytes.resize(16);  // 필요시 자름
    }
}

std::vector<uint8_t> Encryptor::DeriveKeyFromMac(const std::string& salt) {
	std::string mac = GetPrimaryMacAddress();
	std::string combined = mac + salt;

	unsigned char hash[SHA256_DIGEST_LENGTH];
	loader.SHA256((const unsigned char*)combined.c_str(), combined.size(), hash);

    return std::vector<uint8_t>(hash, hash + 16);  // AES-128 키 사용
}

void Encryptor::GenerateRandomBytes(std::vector<uint8_t>& buffer, size_t length) {
    buffer.resize(length);
  	loader.RAND_bytes(buffer.data(), static_cast<int>(length));
}

AnsiString Encryptor::Encrypt(const AnsiString& plainText) {
    const size_t iv_len = 12;
    const size_t tag_len = 16;
	std::vector<uint8_t> iv;
	std::vector<uint8_t> tag;

	GenerateRandomBytes(iv, iv_len);
    tag.resize(tag_len);

	std::vector<uint8_t> cipherText(plainText.Length() + 16);
	EVP_CIPHER_CTX* ctx = loader.EVP_CIPHER_CTX_new();
	if (!ctx) {
		SecureLog::LogWarning("Failed to create EVP_CIPHER_CTX");
		return "";
	}

	loader.EVP_EncryptInit_ex(ctx, loader.EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	loader.EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, nullptr);
    loader.EVP_EncryptInit_ex(ctx, nullptr, nullptr, keyBytes.data(), iv.data());

    int len, ciphertext_len = 0;
    loader.EVP_EncryptUpdate(ctx, cipherText.data(), &len, reinterpret_cast<const uint8_t*>(plainText.c_str()), plainText.Length());
    ciphertext_len = len;

	loader.EVP_EncryptFinal_ex(ctx, cipherText.data() + len, &len);
    ciphertext_len += len;

	loader.EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag_len, tag.data());
    loader.EVP_CIPHER_CTX_free(ctx);

	cipherText.resize(ciphertext_len);
	std::vector<uint8_t> combined;
    combined.reserve(iv.size() + tag.size() + cipherText.size());
    combined.insert(combined.end(), iv.begin(), iv.end());
    combined.insert(combined.end(), tag.begin(), tag.end());
	combined.insert(combined.end(), cipherText.begin(), cipherText.end());

	return Base64Encode(combined);
}

AnsiString Encryptor::Decrypt(const AnsiString& encryptedBase64) {

	std::vector<uint8_t> combined = Base64Decode(encryptedBase64);

	if (combined.size() < (12 + 16)) {
		SecureLog::LogWarning("Invalid encrypted data");
		return "";
	}

    const size_t iv_len = 12;
    const size_t tag_len = 16;

    std::vector<uint8_t> iv(combined.begin(), combined.begin() + iv_len);
    std::vector<uint8_t> tag(combined.begin() + iv_len, combined.begin() + iv_len + tag_len);
    std::vector<uint8_t> cipherBytes(combined.begin() + iv_len + tag_len, combined.end());

	std::vector<uint8_t> plainText(cipherBytes.size());

	EVP_CIPHER_CTX* ctx = loader.EVP_CIPHER_CTX_new();
	if (!ctx) {
		SecureLog::LogWarning("Failed to create EVP_CIPHER_CTX");
        return "";
	}

	loader.EVP_DecryptInit_ex(ctx, loader.EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	loader.EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr);
	loader.EVP_DecryptInit_ex(ctx, nullptr, nullptr, keyBytes.data(), iv.data());

	int len, plaintext_len = 0;
	loader.EVP_DecryptUpdate(ctx, plainText.data(), &len, cipherBytes.data(), cipherBytes.size());
	plaintext_len = len;

	loader.EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), (void*)tag.data());
	if (loader.EVP_DecryptFinal_ex(ctx, plainText.data() + len, &len) <= 0) {
		SecureLog::LogWarning("Decryption failed or tag mismatch");
        return "";
	}

    plaintext_len += len;
    loader.EVP_CIPHER_CTX_free(ctx);

    return AnsiString(reinterpret_cast<const char*>(plainText.data()), plaintext_len);
}

Encryptor::~Encryptor() {
    // 특별히 해줄 일이 없으면 그냥 빈 구현 가능
}
