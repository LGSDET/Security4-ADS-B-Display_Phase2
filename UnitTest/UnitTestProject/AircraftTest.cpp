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

// RawToAircraft 함수 단위 테스트
TEST(AircraftTest, RawToAircraft_BasicAltitude) {
    modeS_message mm{};
    TADS_B_Aircraft ac{};

    mm.msg_type = 4;
    mm.altitude = 12345;

    RawToAircraft(&mm, &ac);

    EXPECT_TRUE(ac.HaveAltitude);
    EXPECT_EQ(ac.Altitude, 12345);
}

TEST(AircraftTest, RawToAircraft_BasicFlightNum) {
    modeS_message mm{};
    TADS_B_Aircraft ac{};

    mm.msg_type = 17;
    mm.ME_type = 1;
    strcpy(mm.flight, "KAL123");

    RawToAircraft(&mm, &ac);

    EXPECT_TRUE(ac.HaveFlightNum);
    EXPECT_STREQ(ac.FlightNum, "KAL123");
}

TEST(AircraftTest, RawToAircraft_BasicCPR) {
    modeS_message mm{};
    TADS_B_Aircraft ac{};

    mm.msg_type = 17;
    mm.ME_type = 9;
    mm.altitude = 10000;
    mm.odd_flag = 1;
    mm.raw_latitude = 123456;
    mm.raw_longitude = 654321;

    RawToAircraft(&mm, &ac);

    EXPECT_TRUE(ac.HaveAltitude);
    EXPECT_EQ(ac.Altitude, 10000);
    EXPECT_EQ(ac.odd_cprlat, 123456);
    EXPECT_EQ(ac.odd_cprlon, 654321);
    EXPECT_NE(ac.odd_cprtime, 0);
}

TEST(AircraftTest, RawToAircraft_BasicSpeedHeading) {
    modeS_message mm{};
    TADS_B_Aircraft ac{};

    mm.msg_type = 17;
    mm.ME_type = 19;
    mm.ME_subtype = 1;
    mm.velocity = 250;
    mm.heading = 90;
    mm.vert_rate_sign = 0;
    mm.vert_rate = 2;

    RawToAircraft(&mm, &ac);

    EXPECT_TRUE(ac.HaveSpeedAndHeading);
    EXPECT_EQ(ac.Speed, 250);
    EXPECT_EQ(ac.Heading, 90);
    EXPECT_EQ(ac.VerticalRate, 64); // (2-1)*64*1
}

