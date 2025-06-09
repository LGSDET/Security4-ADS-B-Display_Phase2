//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "ws2_32.a")

// TLSSession.cpp
#include "TLSSession.h"

TLSSession::TLSSession() : sock(INVALID_SOCKET), ssl_ctx(nullptr), ssl(nullptr), initialized(false), loader(OpenSSLLoader::Instance()) {
    WSAStartup(MAKEWORD(2, 2), new WSADATA());
    Lock = new TCriticalSection();
}

TLSSession::~TLSSession() {
	Disconnect();
    initialized = false;
	if (ssl_ctx) {
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
	}
	if (Lock) {
		delete Lock;
		Lock = nullptr;
	}
	WSACleanup();
}

bool TLSSession::Init() {
	// 1. OpenSSL DLL 동적 로딩
	if (!loader.Load()) {
		MessageBox(nullptr, _T("OpenSSL DLL 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("OpenSSL DLL 로드 실패");
		return false;
    }
    // 2. OpenSSL 라이브러리 초기화
	if (loader.OPENSSL_init_ssl(0, nullptr) != 1) {
		MessageBox(nullptr, _T("OPENSSL_init_ssl 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("OPENSSL_init_ssl 실패!");
        return false;
    }

    // 3. TLS 클라이언트 메서드로 SSL_CTX 생성
	ssl_ctx = loader.SSL_CTX_new(loader.TLS_client_method());
	if (!ssl_ctx) {
		MessageBox(nullptr, _T("SSL_CTX_new 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("SSL_CTX_new 실패!");
        return false;
    }

  //	loader.SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_2_VERSION);

	const char* caCert = "lgess2025s4rpicert.pem";
	const char* clientCert = "lgess2025s4clientcert.pem";
	const char* clientKey = "lgess2025s4clientkey.pem";
	// 4. 서버 CA 인증서 설정 - 서버 인증서 검증용
	if (loader.SSL_CTX_load_verify_locations(ssl_ctx, caCert, nullptr) != 1) {
		MessageBox(nullptr, _T("CA 인증서 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("CA 인증서 로드 실패!");
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
        return false;
    }

    // 5. 클라이언트 인증서와 개인 키 설정 (mutual TLS)
	if (loader.SSL_CTX_use_certificate_file(ssl_ctx, clientCert, SSL_FILETYPE_PEM) != 1) {
		MessageBox(nullptr, _T("클라이언트 인증서 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("클라이언트 인증서 로드 실패!");
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
        return false;
    }

	if (loader.SSL_CTX_use_PrivateKey_file(ssl_ctx, clientKey, SSL_FILETYPE_PEM) != 1) {
		MessageBox(nullptr, _T("클라이언트 개인 키 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("클라이언트 개인 키 로드 실패!");
		loader.SSL_CTX_free(ssl_ctx);
        ssl_ctx = nullptr;
        return false;
    }

	// 6. 개인 키와 인증서가 일치하는지 확인
	if (loader.SSL_CTX_check_private_key(ssl_ctx) != 1) {
		MessageBox(nullptr, _T("개인 키와 인증서가 일치하지 않습니다!"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("개인 키와 인증서가 일치하지 않습니다!");
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
        return false;
	}

	// 7. 서버 인증서 검증 모드 설정
	loader.SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, nullptr);

	const char* tls13Ciphers = "TLS_AES_256_GCM_SHA384";
    if (!loader.SSL_CTX_set_ciphersuites(ssl_ctx, tls13Ciphers)) {
		MessageBox(nullptr, _T("TLS 1.3 cipher 설정 실패"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("TLS 1.3 cipher 설정 실패");
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
		return false;
	}

	if (!loader.SSL_CTX_set_cipher_list(ssl_ctx, "ECDHE-RSA-AES128-GCM-SHA256")) {
		MessageBox(nullptr, _T("TLS 1.2 cipher 설정 실패"), _T("Error"), MB_OK | MB_ICONERROR);
		SecureLog::LogError("TLS 1.2 cipher 설정 실패");
		loader.SSL_CTX_free(ssl_ctx);
		ssl_ctx = nullptr;
		return false;
	}

    loader.SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_TICKET);

	initialized = true;
	//MessageBox(nullptr, _T("OpenSSL 초기화 및 인증서 설정 완료"), _T("Info"), MB_OK | MB_ICONINFORMATION);
	SecureLog::LogInfo("OpenSSL 초기화 및 인증서 설정 완료");
	return true;

}

bool TLSSession::Connect(const AnsiString& ip, int port) {

	if (!initialized) return false;

	AnsiString msg = "try to connect with IP=" + ip + ", Port=" + IntToStr(port);
	SecureLog::LogInfo(std::string(msg.c_str()));
    // WinSock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		SecureLog::LogError("WSAStartup 실패");
        return 1;
    }

    // TCP 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
		SecureLog::LogError("소켓 생성 실패");
		WSACleanup();
		return 1;
	}

    int size = 5 * 1024 * 1024;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size));

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // 서버 연결
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
		SecureLog::LogError("서버 연결 실패");
        closesocket(sock);
        WSACleanup();
        return 1;
	}

    // SSL 객체 생성 및 핸드셰이크
	ssl = loader.SSL_new(ssl_ctx);

	long disableOldTLS = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;
	loader.SSL_CTX_set_options(ssl_ctx, disableOldTLS);

	loader.SSL_set_fd(ssl, (int)sock);

	if (loader.SSL_connect(ssl) != 1) {
		SecureLog::LogError("TLS 연결 실패");
		loader.SSL_free(ssl);
		closesocket(sock);
        WSACleanup();
        return 1;
    }

    sockaddr_in addr = {};
    int len = sizeof(addr);
	if (getpeername(sock, (sockaddr*)&addr, &len) == 0) {
        char ipStr[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &(addr.sin_addr), ipStr, sizeof(ipStr));
        int remotePort = ntohs(addr.sin_port);
		int fd = loader.SSL_get_fd(ssl);
        std::ostringstream oss;
		oss << "TLS 연결 성공 - Peer: " << ipStr << ":" << remotePort
			<< ", FD: " << fd;
		SecureLog::LogInfo(oss.str());
    } else {
        SecureLog::LogWarning("TLS 연결 후 peer 정보 조회 실패");
	}

    return true;
}

AnsiString TLSSession::Read() {
   Lock->Acquire();
   try {
		char buf[2048] = {0};
		int bytes = loader.SSL_read(ssl, buf, sizeof(buf) - 1);
		if (bytes > 0) {
			buf[bytes] = '\0';
			AnsiString result = AnsiString(buf);

			std::string partial(buf, std::min(bytes, 5));
			AnsiString msg = "TLS 수신: " + AnsiString(partial.c_str()) + "...";
			SecureLog::LogInfo(std::string(msg.c_str()));

			return result;
		} else {
			int err =  loader.SSL_get_error(ssl, bytes);
			switch (err) {
			case SSL_ERROR_ZERO_RETURN:
				// TLS connection closed cleanly (shutdown)
				SecureLog::LogError("TLS connection closed cleanly (shutdown)");
				return "";
			case SSL_ERROR_SYSCALL:
				// Low-level I/O error (possibly connection reset)
				SecureLog::LogError("TLS Low-level I/O error (possibly connection reset)");
				return "";
			case SSL_ERROR_SSL:
				// TLS protocol error
				SecureLog::LogError("TLS protocol error");
				return "";
			default:
				SecureLog::LogError("TLS unkonwn error");
				return "";
			}
		}
	}
	__finally {
		Lock->Release();
	}
}

bool TLSSession::Write(const AnsiString& data) {
	Lock->Acquire();
	try {
		return loader.SSL_write(ssl, data.c_str(), data.Length()) > 0;
	}
	__finally {
		Lock->Release();
	}
}

void TLSSession::Disconnect() {
    if (ssl) {
		int ret = loader.SSL_shutdown(ssl);
		if (ret < 0) {
			int err = loader.SSL_get_error(ssl, ret);
			SecureLog::LogWarning(std::string(AnsiString("SSL_shutdown failed: " + IntToStr(err)).c_str()));
		} else if (ret == 0) {
			loader.SSL_shutdown(ssl);  // do second call
		}
        loader.SSL_free(ssl);
        ssl = nullptr;
    }

	if (sock != INVALID_SOCKET) {
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		sock = INVALID_SOCKET;
    }
}

bool TLSSession::IsConnected() const {
	return ssl && loader.SSL_get_fd(ssl) >= 0 && loader.SSL_is_init_finished(ssl);

}

