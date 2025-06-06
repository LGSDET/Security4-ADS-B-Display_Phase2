// OpenSSLLoader.cpp
#include "OpenSSLLoader.h"

OpenSSLLoader& OpenSSLLoader::Instance() {
    static OpenSSLLoader instance;
    return instance;
}

OpenSSLLoader::~OpenSSLLoader() {
    //Unload();
}

bool OpenSSLLoader::Load() {
    if (hModule)
        return true;

    SetDllDirectory(L"C:\\OpenSSL-Win64\\bin");
    hModule = LoadLibraryW(L"C:\\OpenSSL-Win64\\bin\\libssl-3-x64.dll");
    if (!hModule) {
		MessageBox(nullptr, _T("OpenSSL DLL 로드 실패!"), _T("Error"), MB_OK | MB_ICONERROR);
		printf("OpenSSL DLL 로드 실패!");
        return false;
    }

    // 필수 함수들 로딩
    OPENSSL_init_ssl = (decltype(OPENSSL_init_ssl))GetProcAddress(hModule, "OPENSSL_init_ssl");
    SSL_CTX_new = (decltype(SSL_CTX_new))GetProcAddress(hModule, "SSL_CTX_new");
    SSL_CTX_free = (decltype(SSL_CTX_free))GetProcAddress(hModule, "SSL_CTX_free");
    TLS_client_method = (decltype(TLS_client_method))GetProcAddress(hModule, "TLS_client_method");
    SSL_CTX_load_verify_locations = (decltype(SSL_CTX_load_verify_locations))GetProcAddress(hModule, "SSL_CTX_load_verify_locations");
    SSL_CTX_use_certificate_file = (decltype(SSL_CTX_use_certificate_file))GetProcAddress(hModule, "SSL_CTX_use_certificate_file");
    SSL_CTX_use_PrivateKey_file = (decltype(SSL_CTX_use_PrivateKey_file))GetProcAddress(hModule, "SSL_CTX_use_PrivateKey_file");
    SSL_CTX_check_private_key = (decltype(SSL_CTX_check_private_key))GetProcAddress(hModule, "SSL_CTX_check_private_key");
    SSL_CTX_set_verify = (decltype(SSL_CTX_set_verify))GetProcAddress(hModule, "SSL_CTX_set_verify");

    // 암호화/세션 관련 함수 로딩
    SSL_new = (decltype(SSL_new))GetProcAddress(hModule, "SSL_new");
    SSL_set_fd = (decltype(SSL_set_fd))GetProcAddress(hModule, "SSL_set_fd");
    SSL_connect = (decltype(SSL_connect))GetProcAddress(hModule, "SSL_connect");
    SSL_read = (decltype(SSL_read))GetProcAddress(hModule, "SSL_read");
    SSL_shutdown = (decltype(SSL_shutdown))GetProcAddress(hModule, "SSL_shutdown");
    SSL_free = (decltype(SSL_free))GetProcAddress(hModule, "SSL_free");
    SSL_CTX_set_ciphersuites = (decltype(SSL_CTX_set_ciphersuites))GetProcAddress(hModule, "SSL_CTX_set_ciphersuites");
    SSL_get_error = (decltype(SSL_get_error))GetProcAddress(hModule, "SSL_get_error");
    SSL_get_fd = (decltype(SSL_get_fd))GetProcAddress(hModule, "SSL_get_fd");
    SSL_is_init_finished = (decltype(SSL_is_init_finished))GetProcAddress(hModule, "SSL_is_init_finished");
	SSL_write = (decltype(SSL_write))GetProcAddress(hModule, "SSL_write");

    // 로딩 확인
    if (!OPENSSL_init_ssl || !SSL_CTX_new || !SSL_CTX_free || !TLS_client_method ||
        !SSL_CTX_load_verify_locations || !SSL_CTX_use_certificate_file || !SSL_CTX_use_PrivateKey_file ||
        !SSL_CTX_check_private_key || !SSL_CTX_set_verify || !SSL_new || !SSL_set_fd ||
        !SSL_connect || !SSL_read || !SSL_shutdown || !SSL_free || !SSL_CTX_set_ciphersuites ||
		!SSL_get_error || !SSL_get_fd || !SSL_is_init_finished || !SSL_write) {
        printf("check openssl loading failed\n");
        Unload();
        return false;
    }

    return true;
}

void OpenSSLLoader::Unload() {
    if (hModule) {
        FreeLibrary(hModule);
        hModule = nullptr;

        OPENSSL_init_ssl = nullptr;
        SSL_CTX_new = nullptr;
        SSL_CTX_free = nullptr;
        TLS_client_method = nullptr;
        SSL_CTX_load_verify_locations = nullptr;
        SSL_CTX_use_certificate_file = nullptr;
        SSL_CTX_use_PrivateKey_file = nullptr;
        SSL_CTX_check_private_key = nullptr;
        SSL_CTX_set_verify = nullptr;
        SSL_new = nullptr;
        SSL_set_fd = nullptr;
        SSL_connect = nullptr;
        SSL_read = nullptr;
        SSL_shutdown = nullptr;
        SSL_free = nullptr;
        SSL_CTX_set_ciphersuites = nullptr;
        SSL_get_error = nullptr;
        SSL_get_fd = nullptr;
        SSL_is_init_finished = nullptr;
		SSL_write = nullptr;
    }
}
