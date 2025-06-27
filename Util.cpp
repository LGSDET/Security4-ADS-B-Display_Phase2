//---------------------------------------------------------------------------

#pragma hdrstop

#include "Util.h"
//#include <System.hpp>
#include <vector>
#include <unordered_map>
#include <iphlpapi.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "iphlpapi.lib")

AnsiString Base64Encode(const std::vector<uint8_t>& data) {
    const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    AnsiString encoded;
    size_t i = 0;

    while (i < data.size()) {
        uint32_t buf = 0;
        int padding = 0;

		for (int j = 0; j < 3; ++j) {
            buf <<= 8;
            if (i < data.size()) {
                buf |= data[i++];
            } else {
                ++padding;
            }
        }

        for (int j = 0; j < 4 - padding; ++j) {
            int index = (buf >> (18 - j * 6)) & 0x3F;
            encoded += base64_chars[index];
        }

        for (int j = 0; j < padding; ++j)
            encoded += '=';
    }

    return encoded;
}

std::vector<uint8_t> Base64Decode(const AnsiString& base64) {
    static const std::unordered_map<char, uint8_t> base64_map = [] {
        std::unordered_map<char, uint8_t> m;
        const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) {
            m[chars[i]] = static_cast<uint8_t>(i);
        }
        return m;
    }();

    std::vector<uint8_t> result;
    uint32_t buf = 0;
    int bits_collected = 0;

    for (int i = 1; i <= base64.Length(); ++i) {
        char c = base64[i];
        if (c == '=')
            break;

        auto it = base64_map.find(c);
		if (it == base64_map.end())
            continue;

        buf = (buf << 6) | it->second;
        bits_collected += 6;

        if (bits_collected >= 8) {
            bits_collected -= 8;
            result.push_back((buf >> bits_collected) & 0xFF);
        }
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

