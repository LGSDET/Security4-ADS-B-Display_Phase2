// OpenSSLLoader.h
#ifndef OPENSSL_LOADER_H
#define OPENSSL_LOADER_H

#include <Windows.h>
#include <tchar.h>
#include <stdexcept>
#include <openssl/ssl.h>
#include "SecureLog.h"

class OpenSSLLoader {
private:
    HMODULE hModule = nullptr;
    OpenSSLLoader() = default; // private constructor for singleton
    OpenSSLLoader(const OpenSSLLoader&) = delete;
    OpenSSLLoader& operator=(const OpenSSLLoader&) = delete;

public:
    static OpenSSLLoader& Instance();
    ~OpenSSLLoader();

    bool Load();
    void Unload();

    // OpenSSL function pointers
    decltype(::OPENSSL_init_ssl)* OPENSSL_init_ssl = nullptr;
    decltype(::SSL_CTX_new)* SSL_CTX_new = nullptr;
    decltype(::SSL_CTX_free)* SSL_CTX_free = nullptr;
    decltype(::TLS_client_method)* TLS_client_method = nullptr;
    decltype(::SSL_CTX_load_verify_locations)* SSL_CTX_load_verify_locations = nullptr;
    decltype(::SSL_CTX_use_certificate_file)* SSL_CTX_use_certificate_file = nullptr;
    decltype(::SSL_CTX_use_PrivateKey_file)* SSL_CTX_use_PrivateKey_file = nullptr;
    decltype(::SSL_CTX_check_private_key)* SSL_CTX_check_private_key = nullptr;
	decltype(::SSL_CTX_set_verify)* SSL_CTX_set_verify = nullptr;
    decltype(::SSL_new)* SSL_new = nullptr;
    decltype(::SSL_set_fd)* SSL_set_fd = nullptr;
    decltype(::SSL_connect)* SSL_connect = nullptr;
	decltype(::SSL_read)* SSL_read = nullptr;
	decltype(::SSL_shutdown)* SSL_shutdown = nullptr;
	decltype(::SSL_free)* SSL_free = nullptr;
	decltype(::SSL_CTX_set_ciphersuites)* SSL_CTX_set_ciphersuites = nullptr;
	decltype(::SSL_get_error)* SSL_get_error = nullptr;
	decltype(::SSL_get_fd)* SSL_get_fd = nullptr;
	decltype(::SSL_is_init_finished)* SSL_is_init_finished = nullptr;
	decltype(::SSL_write)* SSL_write = nullptr;
	decltype(::SSL_CTX_set_options)* SSL_CTX_set_options = nullptr;
	decltype(::SSL_CTX_set_cipher_list)* SSL_CTX_set_cipher_list = nullptr;
};

#endif // OPENSSL_LOADER_H

