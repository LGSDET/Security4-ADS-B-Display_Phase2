//---------------------------------------------------------------------------

#ifndef TLSSessionH
#define TLSSessionH
//---------------------------------------------------------------------------
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sstream>
#include "OpenSSLLoader.h"  // 앞서 만든 동적 로더 클래스 헤더
#include "SecureLog.h"

class TLSSession {
private:
    SOCKET sock;
	SSL_CTX* ssl_ctx;
    SSL* ssl;
    bool initialized;
	TCriticalSection* Lock;
    // 인증서 관련 내부 경로
    AnsiString caCertPath;
    AnsiString clientCertPath;
    AnsiString clientKeyPath;
	OpenSSLLoader& loader;
public:
    TLSSession();
    ~TLSSession();

    void SetCertificateFiles(const AnsiString& ca, const AnsiString& clientCert, const AnsiString& clientKey);
    bool Init();
    bool Connect(const AnsiString& ip, int port);
    AnsiString Read();
    bool Write(const AnsiString& data);
    void Disconnect();
    bool IsConnected() const;
};
