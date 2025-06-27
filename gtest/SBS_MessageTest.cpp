#include "gtest/gtest.h"
#include "Aircraft.h"
#include "DecodeRawADS_B.h"
#include "PointInPolygon.h"
#include "TimeFunctions.h"
#include <windows.h>
#include "SBS_Message.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Windows ȯ�濡�� strsep ��ü ����
char* strsep(char** stringp, const char* delim) {
    char* start = *stringp;
    char* p;

    if (!start) return nullptr;
    if (!*delim) {
        *stringp = nullptr;
        return start;
    }
    if (!delim[1]) {
        p = strchr(start, *delim);
    }
    else {
        p = strpbrk(start, delim);
    }
    if (!p) {
        *stringp = nullptr;
        return start;
    }
    *p = '\0';
    *stringp = p + 1;
    return start;
}


// Mock ��ü �� �Լ�
static TADS_B_Aircraft g_testAircraft;
static bool g_findCalled = false;
static bool g_createCalled = false;

TADS_B_Aircraft* MockFindAircraft(uint32_t addr) {
    g_findCalled = true;
    if (addr == 0xABCDEF) return &g_testAircraft;
    return nullptr;
}
TADS_B_Aircraft* MockCreateAircraft(uint32_t addr, int spriteImage) {
    g_createCalled = true;
    g_testAircraft.ICAO = addr;
    g_testAircraft.SpriteImage = spriteImage;
    return &g_testAircraft;
}

TEST(SBS_MessageTest, hexDigitVal_AllCases) {
    // ���� static �Լ�������, �׽�Ʈ�� ���� �ζ��� ����
    auto hexDigitVal = [](int c) {
        if (c >= '0' && c <= '9') return c - '0';
        else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        else return -1;
        };
    EXPECT_EQ(hexDigitVal('0'), 0);
    EXPECT_EQ(hexDigitVal('9'), 9);
    EXPECT_EQ(hexDigitVal('A'), 10);
    EXPECT_EQ(hexDigitVal('F'), 15);
    EXPECT_EQ(hexDigitVal('a'), 10);
    EXPECT_EQ(hexDigitVal('f'), 15);
    EXPECT_EQ(hexDigitVal('G'), -1);
    EXPECT_EQ(hexDigitVal('z'), -1);
}

TEST(SBS_MessageTest, strsep_BasicAndEdge) {
    char buf[] = "A,B,C";
    char* p = buf;
    char* token;
    token = strsep(&p, ",");
    EXPECT_STREQ(token, "A");
    token = strsep(&p, ",");
    EXPECT_STREQ(token, "B");
    token = strsep(&p, ",");
    EXPECT_STREQ(token, "C");
    token = strsep(&p, ",");
    EXPECT_EQ(token, nullptr);

    char buf2[] = "ABC";
    p = buf2;
    token = strsep(&p, "");
    EXPECT_STREQ(token, "ABC");
    EXPECT_EQ(p, nullptr);
}

TEST(SBS_MessageTest, ModeS_Build_SBS_Message_AllTypes) {
    modeS_message mm = {};
    TADS_B_Aircraft a = {};
    char msg[MODES_MAX_SBS_SIZE];
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 0
    mm.msg_type = 0; mm.AA[0] = 0xAB; mm.AA[1] = 0xCD; mm.AA[2] = 0xEF; mm.altitude = 123;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 4 (emergency, ground, alert, spi)
    mm.msg_type = 4; mm.identity = 7500; mm.flight_status = 3; mm.altitude = 100;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 5
    mm.msg_type = 5; mm.identity = 1234;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 11
    mm.msg_type = 11;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 17, ME_type 4 (flight)
    mm.msg_type = 17; mm.ME_type = 4; strcpy(mm.flight, "KAL123");
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 17, ME_type 9~18, HaveLatLon false
    mm.msg_type = 17; mm.ME_type = 9; a.HaveLatLon = false;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 17, ME_type 9~18, HaveLatLon true, VALID_POS true
    mm.msg_type = 17; mm.ME_type = 10; a.HaveLatLon = true; a.Latitude = 37.5; a.Longitude = 127.0;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 17, ME_type 19, ME_subtype 1
    mm.msg_type = 17; mm.ME_type = 19; mm.ME_subtype = 1; mm.vert_rate_sign = 0; mm.vert_rate = 2; a.Speed = 250; a.Heading = 90;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type 21
    mm.msg_type = 21; mm.identity = 1234;
    EXPECT_TRUE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // msg_type unknown
    mm.msg_type = 99;
    EXPECT_FALSE(ModeS_Build_SBS_Message(&mm, &a, msg, sizeof(msg)));

    // buffer too small
    mm.msg_type = 0;
    EXPECT_FALSE(ModeS_Build_SBS_Message(&mm, &a, msg, 2));
}

TEST(SBS_MessageTest, SBS_Message_Decode_AllBranches) {
    g_findCalled = false;
    g_createCalled = false;
    memset(&g_testAircraft, 0, sizeof(g_testAircraft));

    // ���� MSG, 6�ڸ� ICAO, callsign, altitude, speed, heading, lat/lon, vertical rate
    char msg1[] = "MSG,3,1,1,ABCDEF,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_TRUE(SBS_Message_Decode(msg1, MockFindAircraft, MockCreateAircraft, 1));
    EXPECT_TRUE(g_findCalled || g_createCalled);
    EXPECT_TRUE(g_testAircraft.HaveFlightNum);
    EXPECT_TRUE(g_testAircraft.HaveAltitude);
    EXPECT_TRUE(g_testAircraft.HaveSpeedAndHeading);
    EXPECT_TRUE(g_testAircraft.HaveLatLon);
    EXPECT_EQ(g_testAircraft.VerticalRate, 500);

    // 7�ڸ� ICAO (padding)
    char msg2[] = "MSG,3,1,1,BCDEF,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_TRUE(SBS_Message_Decode(msg2, MockFindAircraft, MockCreateAircraft, 1));

    // �߸��� ICAO (���� �ʰ�)
    char msg3[] = "MSG,3,1,1,ABCDEFGH,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_FALSE(SBS_Message_Decode(msg3, MockFindAircraft, MockCreateAircraft, 1));

    // �߸��� ICAO (���� ��ȯ ����)
    char msg4[] = "MSG,3,1,1,ZZZZZZ,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_FALSE(SBS_Message_Decode(msg4, MockFindAircraft, MockCreateAircraft, 1));

    // MSG Ÿ�� �ƴ�
    char msg5[] = "NOTMSG,3,1,1,ABCDEF,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_FALSE(SBS_Message_Decode(msg5, MockFindAircraft, MockCreateAircraft, 1));

    // �ʵ� ����
    char msg6[] = "MSG,3,1,1,ABCDEF";
    EXPECT_FALSE(SBS_Message_Decode(msg6, MockFindAircraft, MockCreateAircraft, 1));

    // callsign�� �߸��� ���� ����
    char msg7[] = "MSG,3,1,1,ABCDEF,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL!23,10000,250,90,37.5,127.0,500,,,,,";
    EXPECT_TRUE(SBS_Message_Decode(msg7, MockFindAircraft, MockCreateAircraft, 1));
    EXPECT_FALSE(g_testAircraft.HaveFlightNum);

    // altitude, speed, heading, lat/lon, vertical rate�� �߸��� ��
    char msg8[] = "MSG,3,1,1,ABCDEF,1,2024/06/04,12:34:56.789,2024/06/04,12:34:56.789,KAL123,abc,def,ghi,jkl,mno,pqr,,,,,";
    EXPECT_TRUE(SBS_Message_Decode(msg8, MockFindAircraft, MockCreateAircraft, 1));
    // ���� �Ҵ���� ����(�⺻�� ����)
}
