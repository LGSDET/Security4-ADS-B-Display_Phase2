#include "checkPassword_logic.h"
#include "AwsConfig.h"
#include <System.JSON.hpp>
#include <Windows.h>           // LPVOID 등 Windows 타입 정의
#include <WinInet.h>           // WinINet API
//#include <System.JSON.hpp>     // JSON 처리
#include <System.SysUtils.hpp> // Trim 등 문자열 함수
#include <System.StrUtils.hpp> // StringReplace 등

bool checkPassword(const std::string& password) {
    HINTERNET hInternet = InternetOpen(TEXT("MyApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hConnect = InternetConnectW(
        hInternet,
        API_HOST,
        INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequestW(
        hConnect, L"POST", API_PATH, NULL, NULL, NULL,
        INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    LPCWSTR headers = L"Content-Type: application/json\r\nx-api-key: " API_KEY L"\r\n";
    HttpAddRequestHeadersW(hRequest, headers, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

    AnsiString jsonData = "{\"password\":\"" + AnsiString(password.c_str()) + "\"}";
    if (!HttpSendRequestA(hRequest, NULL, 0, (LPVOID)jsonData.c_str(), jsonData.Length())) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    String response = "";
    do {
        if (!InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) || bytesRead == 0)
            break;
        buffer[bytesRead] = 0;
        response += String(buffer);
    } while (bytesRead > 0);

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    TJSONObject* json = (TJSONObject*)TJSONObject::ParseJSONValue(response);
    if (!json) return false;

    String bodyValue = json->GetValue("body")->Value();
    delete json;

    bodyValue = StringReplace(bodyValue, "\"", "", TReplaceFlags() << rfReplaceAll);
    return bodyValue.Trim() == "true";
}
