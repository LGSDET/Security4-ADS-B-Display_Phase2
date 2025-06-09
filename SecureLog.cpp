// SecureLog.cpp
#include "SecureLog.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <Windows.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

SecureLog::SecureLog()
	: logCounter(1), keyInitialized(false), loader(CryptoLoader::Instance()) {
	if (!loader.Load()) {
        MessageBox(nullptr, _T("OpenSSL DLL 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
    }
    InitKey();
}

SecureLog& SecureLog::Instance() {
    static SecureLog instance;
    return instance;
}

std::vector<uint8_t> SecureLog::DeriveKeyFromMac(const std::string& salt) {
	std::string mac = GetPrimaryMacAddress();
	std::string combined = mac + salt;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	loader.SHA256((const unsigned char*)combined.c_str(), combined.size(), hash);
	return std::vector<uint8_t>(hash, hash + 32);  // AES-128 키 사용
}

void SecureLog::LogInfo(const std::string& message) {
    Instance().Write("INFO", message);
}

void SecureLog::LogWarning(const std::string& message) {
    Instance().Write("WARNING", message);
}

void SecureLog::LogError(const std::string& message) {
    Instance().Write("ERROR", message);
}

void SecureLog::ResetLogCounter() {
    Instance().logCounter = 1;
}

void SecureLog::Write(const std::string& level, const std::string& message) {
	std::lock_guard<std::mutex> guard(logMutex);
    std::ostringstream logEntry;
    logEntry << "[" << logCounter << "] " << GetTimestamp() << " [" << level << "] " << " - " << message;

    std::string entryStr = logEntry.str();
	std::vector<uint8_t> hmac = GenerateHMAC(entryStr);
    std::ofstream out(GetLogFilePath(), std::ios::app);
    if (out.is_open()) {
        out << entryStr << " | HMAC: " << ToHex(hmac) << std::endl;
        ++logCounter;
    }
}

std::string SecureLog::GetLogFilePath() {
	char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
	std::string exePath(path);
    size_t pos = exePath.find_last_of("\\/");
    return exePath.substr(0, pos) + "\\securelog.txt";
}

std::string SecureLog::GetTimestamp() {
	std::time_t now = std::time(nullptr);
    std::tm tmStruct;
    localtime_s(&tmStruct, &now);
    std::ostringstream oss;
    oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<uint8_t> SecureLog::GenerateHMAC(const std::string& message) {
	unsigned int len = SHA256_DIGEST_LENGTH;
    std::vector<uint8_t> result(len);
	loader.HMAC(loader.EVP_sha256(), hmacKey.data(), static_cast<int>(hmacKey.size()),
                reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), result.data(), &len);
    return result;
}

std::string SecureLog::ToHex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (uint8_t byte : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    return oss.str();
}

void SecureLog::InitKey() {
	if (keyInitialized) return;
	std::string salt = "MySalt1234567890";
	hmacKey = DeriveKeyFromMac(salt);
	if (hmacKey.size() < 32) {
        std::hash<std::string> hasher;
        size_t hashVal = hasher(std::string(hmacKey.begin(), hmacKey.end()));

        while (hmacKey.size() < 32) {
            uint8_t byte = static_cast<uint8_t>(
                (hashVal >> ((hmacKey.size() % sizeof(size_t)) * 8)) & 0xFF);
            hmacKey.push_back(byte);
        }
    } else if (hmacKey.size() > 32) {
        hmacKey.resize(32);
    }
	keyInitialized = true;
}

