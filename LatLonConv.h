//---------------------------------------------------------------------------

#ifndef LatLonConvH
#define LatLonConvH
#define _USE_MATH_DEFINES
#include <math.h>

#define DEGTORAD(degree) ((M_PI  / 180.0f) * (degree)) // converts from degrees to radians
#define RADTODEG(radian) ((180.0f / M_PI ) * (radian)) // converts from radians to degrees
#define METERS_PER_NAUICAL_MILE 1852.0
#define EllipseMajorInMeters  6378137.000 // wgs 84 in meters
#define EllipseMinorInMeters  6356752.314 // wgs 84 in meters
#define EllipseMajor   (EllipseMajorInMeters/METERS_PER_NAUICAL_MILE)  // wgs 84 in Data Miles
#define EllipseMinor   (EllipseMinorInMeters/METERS_PER_NAUICAL_MILE)  // wgs 84 in Data Miles

typedef enum
{ OKNOERROR  = 0,     // no error
  LONGERR    = 1,     // longitude maximum exceeded
  LATERR     = 2,     // latitude maximum exceeded
  ANTIPODAL  = 3,     // using antipodal points in VInverse
  SAMEPT     = 4,     // lat/longs are same point - VInverse
  ZERODIST   = 5,     // distance is zero - VDirect
  NOCONVERGE = 6      // loop did not converge
} TCoordConvStatus;

#ifdef __cplusplus
extern "C" {
#endif

    TCoordConvStatus VInverse(double Latitude1, double Longitude1,
        double Latitude2, double Longitude2,
        double* Distance, double* Azimuth1,
        double* Azimuth2);
    TCoordConvStatus VDirect(double Latitude1, double Longitude1,
        double Azimuth12, double Distance,
        double* Latitude2, double* Longitude2,
        double* Azimuth21);

    // 유틸리티 함수 정의와 구현을 모두 header에 작성
    static double Frac(double Num1) {
        return (Num1 - ((int)Num1));
    }
    static double modulus(double Num1, double Num2) {
        return (Num1 - Num2 * floor(Num1 / Num2));
    }
    static double sqr(double X) {
        return (X * X);
    }
    static double ModAzimuth(double az) {
        return modulus(az, 2.0 * M_PI);
    }
    static double ModLatitude(double lat) {
        return modulus(lat + M_PI / 2.0, M_PI) - M_PI / 2.0;
    }
    static double ModLongitude(double lon) {
        return modulus(lon + M_PI, 2.0 * M_PI) - M_PI;
    }
    static TCoordConvStatus Antipod(double latin, double lonin, double* latout, double* lonout) {
        if (fabs(lonin) > 180.0) {
            *latout = 9999.0;
            *lonout = 9999.0;
            return LONGERR;
        }
        if (fabs(latin) > 90.0) {
            *latout = 9999.0;
            *lonout = 9999.0;
            return LATERR;
        }
        *latout = -latin;
        *lonout = modulus(lonin + 180.0, 180.0);
        return OKNOERROR;
    }
    static bool IsAntipodal(double Latitude1, double Latitude2, double Longitude1, double Longitude2) {
        double la, lo;
        int eflag = Antipod(Latitude1, Longitude1, &la, &lo);
        if (eflag != OKNOERROR) return false;

        const double EPSILON = 1e-6;
        return (fabs(Latitude2 - la) < EPSILON) && (fabs(Longitude2 - lo) < EPSILON);
    }

#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------
#endif
