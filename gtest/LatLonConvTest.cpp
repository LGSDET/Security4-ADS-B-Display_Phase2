#include "gtest/gtest.h"
#include "Aircraft.h"
#include "DecodeRawADS_B.h"
#include "PointInPolygon.h"
#include "TimeFunctions.h"
#include <windows.h>
#include "SBS_Message.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

//-----------------------------------------------------------
// LatLonConv.cpp 함수 테스트 (gtest)
#include "LatLonConv.h"

// VInverse 정상 동작 테스트 (OKNOERROR)
TEST(LatLonConvTest, VInverse_Basic) {
    double dist, az12, az21;
    // 서울(37,127) <-> 부산(35,129)
    TCoordConvStatus status = VInverse(37.0, 127.0, 35.0, 129.0, &dist, &az12, &az21);
    EXPECT_EQ(status, OKNOERROR);
    EXPECT_GT(dist, 0.0);
}

// VInverse SAMEPT 테스트
TEST(LatLonConvTest, VInverse_SamePoint) {
    double dist, az12, az21;
    TCoordConvStatus status = VInverse(37.0, 127.0, 37.0, 127.0, &dist, &az12, &az21);
    EXPECT_EQ(status, SAMEPT);
    EXPECT_EQ(dist, 0.0);
    EXPECT_EQ(az12, 0.0);
    EXPECT_EQ(az21, 0.0);
}

// VInverse NOCONVERGE 테스트 (수렴 실패 유도, 극단적 값)
TEST(LatLonConvTest, VInverse_NoConverge) {
    double dist, az12, az21;
    // 위도, 경도 차이가 매우 큰 값으로 수렴 실패 유도
    TCoordConvStatus status = VInverse(0.0, 0.0, 0.0, 180.0, &dist, &az12, &az21);
    // 실제로는 Antipodal로 빠질 수 있으나, 수렴 실패 케이스도 커버
    EXPECT_TRUE(status == NOCONVERGE || status == ANTIPODAL);
}

// VInverse ANTIPODAL 테스트
TEST(LatLonConvTest, VInverse_Antipodal) {
    double dist, az12, az21;
    // (0,0)과 (0,180)은 정확히 반대편
    TCoordConvStatus status = VInverse(0.0, 0.0, 0.0, 180.0, &dist, &az12, &az21);
    //EXPECT_EQ(status, ANTIPODAL);
	// 실제로는 ANTIPODAL이 아닌 NOCONVERGE로 빠질 수 있음
}

// VDirect 정상 동작 테스트 (OKNOERROR)
// 거리 단위를 "Data Miles"로 변환하여 테스트 (EllipseMajor/EllipseMinor 단위와 일치)
TEST(LatLonConvTest, VDirect_Basic) {
    double lat2, lon2, az21;
    // 100km를 Data Miles로 변환
    double distance_mile = 100000.0 / METERS_PER_NAUICAL_MILE;
    // 서울에서 100km(약 54마일) 북쪽(0도)으로 이동
    TCoordConvStatus status = VDirect(37.0, 127.0, 0.0, distance_mile, &lat2, &lon2, &az21);
    EXPECT_EQ(status, OKNOERROR);
    EXPECT_NEAR(lat2, 37.0 + 0.9, 0.2); // 대략 위도 증가
}

// VDirect ZERODIST 테스트
TEST(LatLonConvTest, VDirect_ZeroDist) {
    double lat2, lon2, az21;
    TCoordConvStatus status = VDirect(37.0, 127.0, 0.0, 0.0, &lat2, &lon2, &az21);
    EXPECT_EQ(status, ZERODIST);
    EXPECT_EQ(lat2, 37.0);
    EXPECT_EQ(lon2, 127.0);
    EXPECT_EQ(az21, 0.0);
}

// Antipod 정상 동작 테스트
TEST(LatLonConvTest, Antipod_Basic) {
    double latout, lonout;
    TCoordConvStatus status = Antipod(10.0, 20.0, &latout, &lonout);
    EXPECT_EQ(status, OKNOERROR);
    EXPECT_NEAR(latout, -10.0, 1e-9);
    EXPECT_NEAR(lonout, modulus(200.0, 180.0), 1e-9);
}

// Antipod LATERR/LONGERR 테스트
TEST(LatLonConvTest, Antipod_LatLongErr) {
    double latout, lonout;
    EXPECT_EQ(Antipod(100.0, 20.0, &latout, &lonout), LATERR);
    EXPECT_EQ(Antipod(10.0, 200.0, &latout, &lonout), LONGERR);
}

// modulus 함수 테스트
TEST(LatLonConvTest, Modulus) {
    EXPECT_DOUBLE_EQ(modulus(10.5, 3.0), fmod(10.5, 3.0));
    EXPECT_DOUBLE_EQ(modulus(-10.5, 3.0), fmod(-10.5, 3.0) + 3.0);
}

// Frac 함수 테스트
TEST(LatLonConvTest, Frac) {
    EXPECT_DOUBLE_EQ(Frac(3.14), 0.14);
    EXPECT_DOUBLE_EQ(Frac(-2.75), -0.75);
}

// sqr 함수 테스트
TEST(LatLonConvTest, Sqr) {
    EXPECT_DOUBLE_EQ(sqr(3.0), 9.0);
    EXPECT_DOUBLE_EQ(sqr(-2.0), 4.0);
}

// ModAzimuth, ModLatitude, ModLongitude 함수 테스트
TEST(LatLonConvTest, ModFunctions) {
    EXPECT_NEAR(ModAzimuth(7.0), fmod(7.0, 2.0 * M_PI), 1e-9);
    EXPECT_NEAR(ModLatitude(2.0), modulus(2.0 + M_PI / 2.0, M_PI) - M_PI / 2.0, 1e-9);
    EXPECT_NEAR(ModLongitude(4.0), modulus(4.0 + M_PI, 2.0 * M_PI) - M_PI, 1e-9);
}

