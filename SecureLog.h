// SecureLog.h
#ifndef SecureLogH
#define SecureLogH

#include <string>
#include <vector>
#include <mutex>
#include "CryptoLoader.h"
#include "Util.h"

class CryptoLoader;

class SecureLog {
public:
    static void LogInfo(const std::string& message);
    static void LogWarning(const std::string& message);
    static void LogError(const std::string& message);
    static void ResetLogCounter();

private:
    SecureLog();
    ~SecureLog() = default;
    SecureLog(const SecureLog&) = delete;
    SecureLog& operator=(const SecureLog&) = delete;

    static SecureLog& Instance();
    std::vector<uint8_t> DeriveKeyFromMac(const std::string& salt);;
    void Write(const std::string& level, const std::string& message);
    std::string GetLogFilePath();
    std::string GetTimestamp();
    std::vector<uint8_t> GenerateHMAC(const std::string& message);
    std::string ToHex(const std::vector<uint8_t>& data);
    void InitKey();

    int logCounter;
    bool keyInitialized;
    std::vector<uint8_t> hmacKey;
    std::mutex logMutex;

    CryptoLoader& loader;
};
#endif

