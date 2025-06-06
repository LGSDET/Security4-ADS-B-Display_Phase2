//---------------------------------------------------------------------------

#pragma hdrstop

#include "Util.h"
#include <System.hpp>
#include <IdCoderMIME.hpp>
#include <IdGlobal.hpp>
#include <iphlpapi.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "iphlpapi.lib")

AnsiString Base64Encode(const std::vector<uint8_t>& data) {
    TIdBytes idBytes;

    // Resize TIdBytes to match input vector size
    idBytes.Length = static_cast<int>(data.size());

    // Copy data from std::vector to TIdBytes
    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        idBytes[i] = data[i];
    }

    TIdEncoderMIME* encoder = new TIdEncoderMIME();
    AnsiString result = encoder->EncodeBytes(idBytes);
    delete encoder;

    return result;
}

std::vector<uint8_t> Base64Decode(const AnsiString& base64) {
    TIdDecoderMIME* decoder = new TIdDecoderMIME();

    // 디코딩
    TIdBytes idBytes = decoder->DecodeBytes(base64);
    delete decoder;

    // TIdBytes → std::vector<uint8_t> 변환
    std::vector<uint8_t> result(idBytes.Length);
    for (int i = 0; i < idBytes.Length; ++i) {
        result[i] = idBytes[i];
    }

    return result;
}

std::string GetPrimaryMacAddress() {
    IP_ADAPTER_INFO AdapterInfo[16];       // 최대 16개 어댑터
    DWORD buflen = sizeof(AdapterInfo);
    DWORD status = GetAdaptersInfo(AdapterInfo, &buflen);
    if (status != ERROR_SUCCESS) return "";

    BYTE* mac = AdapterInfo[0].Address;     // 첫 번째 어댑터 사용
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(macStr);
}
