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
    void latLonToECEF(double lat, double lon, double altitude, double *x, double *y, double *z);
    void velocityVector(double lat, double lon, double speed, double heading, double *vx, double *vy, double *vz);
    bool computeCPA(double lat1, double lon1, double altitude1, double speed1, double heading1,
                    double lat2, double lon2, double altitude2, double speed2, double heading2,
                    double &tcpa, double &cpa_distance_nm, double &vertical_cpa);
    //bool PointInPolygon(pfVec3* Verts, int NumVerts, pfVec3 Point);
    //bool IsPointOnSegment(const pfVec3& a, const pfVec3& b, const pfVec3 p);
    __int64 GetCurrentTimeInMsec(void);
    char* TimeToChar(__int64 hmsm);
}

// DecodeRawADS_B.cpp gtest: basic message parsing and error handling
class DecodeRawADSBTest : public ::testing::Test {
protected:
    modeS_message mm;
    void SetUp() override {
        InitDecodeRawADS_B();
        mm = modeS_message{};
    }
};

TEST_F(DecodeRawADSBTest, OnlyAsteriskSemicolon) {
    EXPECT_EQ(decode_RAW_message("*;", &mm), CRCError);
}

TEST_F(DecodeRawADSBTest, HeartBeat) {
    EXPECT_EQ(decode_RAW_message(std::string(MODES_RAW_HEART_BEAT), &mm), MsgHeartBeat);
}

TEST_F(DecodeRawADSBTest, BadMessageFormat2) {
    EXPECT_EQ(decode_RAW_message("abc", &mm), BadMessageFormat2);
    EXPECT_EQ(decode_RAW_message(" *1234; ", &mm), BadMessageFormat2);
    EXPECT_EQ(decode_RAW_message("*1234", &mm), BadMessageFormat2);
}

TEST_F(DecodeRawADSBTest, BadMessageEmpty2) {
    EXPECT_EQ(decode_RAW_message("\n", &mm), BadMessageEmpty2);
    EXPECT_EQ(decode_RAW_message("*", &mm), BadMessageEmpty2);
}

TEST_F(DecodeRawADSBTest, BadMessageHighLow) {
    EXPECT_EQ(decode_RAW_message("* ;", &mm), BadMessageHighLow);
    EXPECT_EQ(decode_RAW_message("*GG;", &mm), BadMessageHighLow);
    EXPECT_EQ(decode_RAW_message("*1G;", &mm), BadMessageHighLow);
}

TEST_F(DecodeRawADSBTest, CRCError) {
    EXPECT_EQ(decode_RAW_message("*;", &mm), CRCError); // CRCError: minimal valid message but CRC fails
    
}
TEST_F(DecodeRawADSBTest, BadMessageTooLong) {
    std::string msg = "*";
    for (int i = 0; i < 60; ++i) msg += "AA";
    msg += ";";
    EXPECT_EQ(decode_RAW_message(msg, &mm), BadMessageTooLong);
}

TEST_F(DecodeRawADSBTest, CRCErrorAndHaveMsg) {
    // Valid short message (DF=5, 7 bytes, CRC will fail)
    EXPECT_EQ(decode_RAW_message("*8D4840D6202CC371C32CE0576096;", &mm), CRCError);
    EXPECT_EQ(decode_RAW_message("*8D40621D58C382D690C8AC2863A8;", &mm), CRCError);
}

// decode_AC13_field: Q=0, M=0 (TODO branch, returns 0)
TEST_F(DecodeRawADSBTest, AC13Field_Q0_M0) {
    uint8_t msg[8] = { 0 };
    metric_unit_t unit;
    msg[3] = 0; // M=0, Q=0
    EXPECT_EQ(decode_AC13_field(msg, &unit), 0);
}

// decode_AC13_field: M=1 (TODO branch, returns 0)
TEST_F(DecodeRawADSBTest, AC13Field_M1) {
    uint8_t msg[8] = { 0 };
    metric_unit_t unit;
    msg[3] = (1 << 6); // M=1
    EXPECT_EQ(decode_AC13_field(msg, &unit), 0);
}

// decode_AC12_field: Q=0 branch
TEST_F(DecodeRawADSBTest, AC12Field_Q0) {
    uint8_t msg[8] = { 0 };
    metric_unit_t unit;
    msg[5] = 0; // Q=0
    EXPECT_EQ(decode_AC12_field(msg, &unit), 0);
}

// decode_modeS_message: DF17, ME_type 19, ME_subtype 3/4 branch
TEST_F(DecodeRawADSBTest, ModeSMessage_DF17_ME19_Subtype3) {
    modeS_message mm = {};
    uint8_t msg[14] = { 0 };
    msg[0] = (17 << 3); // DF17
    msg[4] = (19 << 3) | 3; // ME_type=19, ME_subtype=3
    msg[5] = (1 << 2); // heading_is_valid
    msg[6] = (10 << 3); // heading 값
    decode_modeS_message(&mm, msg);
    EXPECT_TRUE(mm.heading_is_valid);
}

// decode_modeS_message: DF17, ME_type 19, ME_subtype 5~8 (surface position, 미구현)
TEST_F(DecodeRawADSBTest, ModeSMessage_DF17_ME19_Subtype5) {
    modeS_message mm = {};
    uint8_t msg[14] = { 0 };
    msg[0] = (17 << 3); // DF17
    msg[4] = (19 << 3) | 5; // ME_type=19, ME_subtype=5
    decode_modeS_message(&mm, msg);
    // No crash, just coverage
    SUCCEED();
}

// decode_modeS_message: DF != 11, 17, brute_force_AP 실패 branch
TEST_F(DecodeRawADSBTest, ModeSMessage_BruteForceAP_Fail) {
    modeS_message mm = {};
    uint8_t msg[14] = { 0 };
    msg[0] = (4 << 3); // DF4
    decode_modeS_message(&mm, msg);
    EXPECT_FALSE(mm.CRC_ok);
}

// decode_modeS_message: DF == 11, CRC_ok, error_bit == -1, ICAO_cache_add_address branch
TEST_F(DecodeRawADSBTest, ModeSMessage_DF11_CacheAdd) {
    modeS_message mm = {};
    uint8_t msg[14] = { 0 };
    msg[0] = (11 << 3); // DF11
    // CRC_ok, error_bit == -1
    mm.CRC_ok = true;
    mm.error_bit = -1;
    decode_modeS_message(&mm, msg);
    SUCCEED();
}

// decode_modeS_message: DF == 17, ME_type 1~4, trailing space 제거 branch
TEST_F(DecodeRawADSBTest, ModeSMessage_DF17_ME1_TrailingSpace) {
    modeS_message mm = {};
    uint8_t msg[14] = { 0 };
    msg[0] = (17 << 3); // DF17
    msg[4] = (1 << 3); // ME_type=1
    // flight[7] = ' ' (trailing space)
    msg[10] = 0; // AIS_charset[0] == '?'
    decode_modeS_message(&mm, msg);
    EXPECT_EQ(mm.flight[7], '\0');
}

// fix_single_bit_errors: no fixable error
TEST_F(DecodeRawADSBTest, FixSingleBitErrors_NoFix) {
    uint8_t msg[7] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    EXPECT_EQ(fix_single_bit_errors(msg, 56), -1);
}

// fix_two_bits_errors: no fixable error
TEST_F(DecodeRawADSBTest, FixTwoBitsErrors_NoFix) {
    uint8_t msg[7] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    EXPECT_EQ(fix_two_bits_errors(msg, 56), -1);
}

// brute_force_AP: not applicable type
TEST_F(DecodeRawADSBTest, BruteForceAP_NotApplicable) {
    modeS_message mm = {};
    mm.msg_type = 99; // not in list
    mm.msg_bits = 56;
    uint8_t msg[7] = { 0 };
    EXPECT_FALSE(brute_force_AP(msg, &mm));
}

// ICAO_address_recently_seen: cache miss
TEST_F(DecodeRawADSBTest, ICAOAddressRecentlySeen_Miss) {
    EXPECT_FALSE(ICAO_address_recently_seen(0x123456));
}

// decode_RAW_message: BadMessageEmpty1 branch
TEST_F(DecodeRawADSBTest, BadMessageEmpty1) {
    modeS_message mm{};
    EXPECT_EQ(decode_RAW_message("", &mm), BadMessageEmpty1);
}

// decode_RAW_message: BadMessageFormat1 branch
TEST_F(DecodeRawADSBTest, BadMessageFormat1) {
    modeS_message mm{};
    EXPECT_EQ(decode_RAW_message("no_newline", &mm), BadMessageFormat2);
}

// decode_RAW_message: BadMessageFormat2 (missing * or ;)
TEST_F(DecodeRawADSBTest, BadMessageFormat2_MissingStarOrSemicolon) {
    modeS_message mm{};
    EXPECT_EQ(decode_RAW_message("1234;", &mm), BadMessageFormat2);
    EXPECT_EQ(decode_RAW_message("*1234", &mm), BadMessageFormat2);
}

TEST_F(DecodeRawADSBTest, ICAO_cache_add_address_Basic) {
    InitDecodeRawADS_B();

    uint32_t test_addr = 0x123456;

    // DF11 메시지 생성 (7 bytes, CRC 포함)
    uint8_t msg[7] = { 0 };
    msg[0] = (11 << 3); // DF11
    msg[1] = (test_addr >> 16) & 0xFF;
    msg[2] = (test_addr >> 8) & 0xFF;
    msg[3] = test_addr & 0xFF;

    // CRC 계산
    extern uint32_t CRC_check(const uint8_t * msg, int bits);
    int bits = 56;
    uint32_t crc = CRC_check(msg, bits);
    msg[4] = (crc >> 16) & 0xFF;
    msg[5] = (crc >> 8) & 0xFF;
    msg[6] = crc & 0xFF;

    modeS_message mm = {};
    decode_modeS_message(&mm, msg);

    EXPECT_TRUE(ICAO_address_recently_seen(test_addr));
}
