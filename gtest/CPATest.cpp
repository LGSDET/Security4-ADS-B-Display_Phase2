//#include <vcl.h>
#include <windows.h>

#include "gtest/gtest.h"
#include "CPA.h"
#include "Aircraft.h"
#include "DecodeRawADS_B.h"
#include "PointInPolygon.h"
#include "TimeFunctions.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


//-----------------------------------------------------------
// CPA.cpp 함수 테스트 (gtest)
class CPATest : public ::testing::Test {
protected:
    void SetUp() override {}
};

// latLonToECEF 변환 테스트
TEST_F(CPATest, LatLonToECEF_Basic) {
    double x, y, z;
    latLonToECEF(0.0, 0.0, 0.0, &x, &y, &z);
    EXPECT_NEAR(x, 6371.0, 1e-3);
    EXPECT_NEAR(y, 0.0, 1e-3);
    EXPECT_NEAR(z, 0.0, 1e-3);

    latLonToECEF(90.0, 0.0, 0.0, &x, &y, &z);
    EXPECT_NEAR(x, 0.0, 1e-3);
    EXPECT_NEAR(y, 0.0, 1e-3);
    EXPECT_NEAR(z, 6371.0, 1e-3);
}

// velocityVector 변환 테스트
TEST_F(CPATest, VelocityVector_Basic) {
    double vx, vy, vz;
    velocityVector(0.0, 0.0, 360.0, 0.0, &vx, &vy, &vz); // 북쪽 360노트
    EXPECT_NEAR(vx, 0.0, 1e-3);
    EXPECT_NEAR(vy, 0.0, 1e-3);
    EXPECT_GT(vz, 0.0);

    velocityVector(0.0, 0.0, 360.0, 90.0, &vx, &vy, &vz); // 동쪽 360노트
    EXPECT_NEAR(vx, 0.0, 1e-3);
    EXPECT_NEAR(vy, 0.1852, 1e-3);
    EXPECT_NEAR(vz, 0.0, 1e-3);
}

// computeCPA 해피패스 테스트
TEST_F(CPATest, ComputeCPA_Basic) {
    double tcpa, cpa_distance_nm, vertical_cpa;
    // 두 항공기가 같은 고도, 평행, 10NM 떨어져 접근
    bool result = computeCPA(
        37.0, 127.0, 10000.0, 250.0, 90.0, // 항공기1: 동쪽 250노트
        37.0, 127.1, 10000.0, 250.0, 270.0, // 항공기2: 서쪽 250노트
        tcpa, cpa_distance_nm, vertical_cpa
    );
    EXPECT_TRUE(result);
    EXPECT_GT(tcpa, 0.0);
    EXPECT_NEAR(vertical_cpa, 0.0, 1e-3);
    EXPECT_GE(cpa_distance_nm, 0.0);
}

// computeCPA 수직 분리 테스트
TEST_F(CPATest, ComputeCPA_VerticalSeparation) {
    double tcpa, cpa_distance_nm, vertical_cpa;
    bool result = computeCPA(
        37.0, 127.0, 10000.0, 250.0, 90.0,
        37.0, 127.1, 20000.0, 250.0, 270.0,
        tcpa, cpa_distance_nm, vertical_cpa
    );
    EXPECT_TRUE(result);
    EXPECT_NEAR(vertical_cpa, 10000.0, 1e-3);
}

// computeCPA no meaningful CPA test
TEST_F(CPATest, ComputeCPA_NoMeaningfulCPA) {
    double tcpa, cpa_distance_nm, vertical_cpa;

    // 1. Parallel, same heading/speed, different position
    EXPECT_FALSE(computeCPA(
        37.0, 127.0, 10000.0, 250.0, 90.0,
        37.0, 127.1, 10000.0, 250.0, 90.0,
        tcpa, cpa_distance_nm, vertical_cpa
    ));

    // 2. CPA is in the past (tcpa < 0)
    EXPECT_FALSE(computeCPA(
        37.0, 127.0, 10000.0, 250.0, 90.0,
        37.0, 126.9, 10000.0, 250.0, 90.0,
        tcpa, cpa_distance_nm, vertical_cpa
    ));

    // 3. CPA is now (tcpa == 0)
    EXPECT_FALSE(computeCPA(
        37.0, 127.0, 10000.0, 250.0, 90.0,
        37.0, 127.0, 10000.0, 250.0, 90.0,
        tcpa, cpa_distance_nm, vertical_cpa
      ));
}
