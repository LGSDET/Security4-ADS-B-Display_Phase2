//---------------------------------------------------------------------------

#ifndef AircraftH
#define AircraftH
#include "DecodeRawADS_B.h"
#include <cstdint>

#define MODES_NON_ICAO_ADDRESS       (1<<24) // Set on addresses to indicate they are not ICAO addresses

typedef struct
{
 uint32_t            ICAO;
 char                HexAddr[7];       /* Printable ICAO address */
 __int64             LastSeen;             /* Time at which the last packet was received. */
 long                NumMessagesRaw;      /* Number of Mode S messages received. */
 long                NumMessagesSBS;
 int                 odd_cprlat;       /* Encoded latitude and longitude as extracted by odd and even */
 int                 odd_cprlon;       /* CPR encoded messages. */
 int                 even_cprlat;
 int                 even_cprlon;
 __int64             odd_cprtime;
 __int64             even_cprtime;
 char                FlightNum[9];     /* Flight number */
 bool                HaveFlightNum;
 bool                HaveAltitude;
 double              Altitude;
 bool                HaveLatLon;
 double              Latitude;      /* Coordinated obtained from CPR encoded data. */
 double              Longitude;     /* Coordinated obtained from CPR encoded data. */
 bool                HaveSpeedAndHeading;
 double              Heading;
 double              Speed;
 double              VerticalRate;
 int                 SpriteImage;
} TADS_B_Aircraft;

//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// CPR CO¨ùo ¨ù¡¾¨ú©£¥ìe
int cprModFunction(int a, int b);
int cprNLFunction(double lat);
int cprNFunction(double lat, int isodd);
double cprDlonFunction(double lat, int isodd);
void decodeCPR(TADS_B_Aircraft *a);

// RawToAircraft CO¨ùo ¨ù¡¾¨ú©£
void RawToAircraft(modeS_message *mm, TADS_B_Aircraft *ADS_B_Aircraft);

#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------
#endif
