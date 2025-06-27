//---------------------------------------------------------------------------

#ifndef UtilH
#define UtilH
#include <stdexcept>
#include <memory>
#include <vector>
#include <fstream>
//---------------------------------------------------------------------------
#ifdef _MSC_VER
    #include <string>
    typedef std::string AnsiString;
#else
    #include <System.hpp>
#endif
AnsiString Base64Encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> Base64Decode(const AnsiString& base64);
std::string GetPrimaryMacAddress();

#endif

