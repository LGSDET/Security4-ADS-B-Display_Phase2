#include "pch.h"
#include "gtest/gtest.h"
#include "Aircraft.h"
#include "DecodeRawADS_B.h"
#include "PointInPolygon.h"
#include "TimeFunctions.h"
#include <windows.h>
#include "SBS_Message.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


extern "C" {
    char* TimeToChar(__int64 hmsm);
}

//-----------------------------------------------------------
// ntds2d.cpp 함수 테스트 (gtest)
#include "ntds2d.h"

// ComputeTimeToGoPosition 함수 단위 테스트
TEST(Ntds2dTest, ComputeTimeToGoPosition_Basic) {
    float xs = 100.0f, ys = 200.0f;
    float xv = 360.0f, yv = 720.0f; // 360 knots, 720 knots
    float timeToGo = 3600.0f; // 1 hour
    float xe = 0.0f, ye = 0.0f;

    ComputeTimeToGoPosition(timeToGo, xs, ys, xv, yv, xe, ye);

    // xv/3600 * timeToGo = 360/3600*3600 = 360
    // xe = 100 + 360 = 460
    // yv/3600 * timeToGo = 720/3600*3600 = 720
    // ye = 200 + 720 = 920
    EXPECT_FLOAT_EQ(xe, 460.0f);
    EXPECT_FLOAT_EQ(ye, 920.0f);
}

// DrawLines 함수 단위 테스트 (렌더링 결과는 검증 불가, 단순 호출만 테스트)
TEST(Ntds2dTest, DrawLines_Call) {
    const DWORD resolution = 4;
    double xpts[resolution] = {0.0, 1.0, 1.0, 0.0};
    double ypts[resolution] = {0.0, 0.0, 1.0, 1.0};
    // OpenGL context가 없으면 실제로는 아무 동작도 하지 않음
    // 단순히 함수가 예외 없이 호출되는지만 확인
    EXPECT_NO_THROW(DrawLines(resolution, xpts, ypts));
}

// MakeAirplaneImages 함수 테스트 (파일이 없으면 0 반환, 예외 없이 동작만 확인)
TEST(Ntds2dTest, MakeAirplaneImages_Call) {
    EXPECT_NO_THROW({
        int result = MakeAirplaneImages();
        EXPECT_GE(result, 0);
    });
}

// MakeAirTrackFriend 함수 호출 테스트
TEST(Ntds2dTest, MakeAirTrackFriend_Call) {
    EXPECT_NO_THROW(MakeAirTrackFriend());
}

// MakeAirTrackHostile 함수 호출 테스트
TEST(Ntds2dTest, MakeAirTrackHostile_Call) {
    EXPECT_NO_THROW(MakeAirTrackHostile());
}

// MakeAirTrackUnknown 함수 호출 테스트
TEST(Ntds2dTest, MakeAirTrackUnknown_Call) {
    EXPECT_NO_THROW(MakeAirTrackUnknown());
}

// MakePoint 함수 호출 테스트
TEST(Ntds2dTest, MakePoint_Call) {
    EXPECT_NO_THROW(MakePoint());
}

// MakeTrackHook 함수 호출 테스트
TEST(Ntds2dTest, MakeTrackHook_Call) {
    EXPECT_NO_THROW(MakeTrackHook());
}

// DrawAirplaneImage 함수 호출 테스트
TEST(Ntds2dTest, DrawAirplaneImage_Call) {
    EXPECT_NO_THROW(DrawAirplaneImage(0.0f, 0.0f, 1.0f, 0.0f, 0));
}

// DrawAirTrackFriend 함수 호출 테스트
TEST(Ntds2dTest, DrawAirTrackFriend_Call) {
    EXPECT_NO_THROW(DrawAirTrackFriend(0.0f, 0.0f));
}

// DrawAirTrackHostile 함수 호출 테스트
TEST(Ntds2dTest, DrawAirTrackHostile_Call) {
    EXPECT_NO_THROW(DrawAirTrackHostile(0.0f, 0.0f));
}

// DrawAirTrackUnknown 함수 호출 테스트
TEST(Ntds2dTest, DrawAirTrackUnknown_Call) {
    EXPECT_NO_THROW(DrawAirTrackUnknown(0.0f, 0.0f));
}

// DrawPoint 함수 호출 테스트
TEST(Ntds2dTest, DrawPoint_Call) {
    EXPECT_NO_THROW(DrawPoint(0.0f, 0.0f));
}

// DrawTrackHook 함수 호출 테스트
TEST(Ntds2dTest, DrawTrackHook_Call) {
    EXPECT_NO_THROW(DrawTrackHook(0.0f, 0.0f));
}

// DrawRadarCoverage 함수 호출 테스트
TEST(Ntds2dTest, DrawRadarCoverage_Call) {
    EXPECT_NO_THROW(DrawRadarCoverage(0.0f, 0.0f, 10.0f, 5.0f));
}

// DrawLeader 함수 호출 테스트
TEST(Ntds2dTest, DrawLeader_Call) {
    EXPECT_NO_THROW(DrawLeader(0.0f, 0.0f, 1.0f, 1.0f));
}
