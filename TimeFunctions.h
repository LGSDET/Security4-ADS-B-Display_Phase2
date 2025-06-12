//---------------------------------------------------------------------------

#ifndef TimeFunctionsH
#define TimeFunctionsH

#ifdef __cplusplus
extern "C" {
#endif

__int64 GetCurrentTimeInMsec(void);
char* TimeToChar(__int64 hmsm);

#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------
#endif
