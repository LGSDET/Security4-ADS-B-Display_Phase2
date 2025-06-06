//---------------------------------------------------------------------------

#ifndef UtilH
#define UtilH
#include <stdexcept>
#include <memory>
#include <vector>
#include <fstream>
//---------------------------------------------------------------------------

AnsiString Base64Encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> Base64Decode(const AnsiString& base64);
std::string GetPrimaryMacAddress();

#endif

