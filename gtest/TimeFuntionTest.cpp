#include "gtest/gtest.h"
#include "TimeFunctions.h"


extern "C" {
    char* TimeToChar(__int64 hmsm);
}

class TimeFuntionTest : public ::testing::Test {
protected:
    void SetUp() override {}
};
// TimeToChar 정상 변환 테스트
TEST(TimeFuntionTest, TimeToChar_NormalCases) {
    // 0ms -> 00:00:00:000
    EXPECT_STREQ(TimeToChar(0), "00:00:00:000");
    // 1초 -> 00:00:01:000
    EXPECT_STREQ(TimeToChar(1000), "00:00:01:000");
    // 1분 -> 00:01:00:000
    EXPECT_STREQ(TimeToChar(60000), "00:01:00:000");
    // 1시간 -> 01:00:00:000
    EXPECT_STREQ(TimeToChar(3600000), "01:00:00:000");
    // 12:34:56.789
    EXPECT_STREQ(TimeToChar(12 * 3600000 + 34 * 60000 + 56 * 1000 + 789), "12:34:56:789");
    // 23:59:59.999
    EXPECT_STREQ(TimeToChar(23 * 3600000 + 59 * 60000 + 59 * 1000 + 999), "23:59:59:999");
}

// TimeToChar 경계값 및 모듈로 연산 테스트
TEST(TimeFuntionTest, TimeToChar_BoundaryCases) {
    // 24시간(하루) -> 00:00:00:000 (24시가 0시로 롤오버)
    EXPECT_STREQ(TimeToChar(24 * 3600000), "00:00:00:000");
    // 25시간 -> 01:00:00:000
    EXPECT_STREQ(TimeToChar(25 * 3600000), "01:00:00:000");
    // 음수 입력 (비정상 입력)
    EXPECT_STREQ(TimeToChar(-1), "00:00:00:000");
    EXPECT_STREQ(TimeToChar(-(12 * 3600000 + 34 * 60000 + 56 * 1000 + 789)), "00:00:00:000");
}

// TimeToChar 자리수별(한자리/두자리) 테스트
TEST(TimeFuntionTest, TimeToChar_SingleDoubleDigit) {
    // 1:02:03.004
    EXPECT_STREQ(TimeToChar(1 * 3600000 + 2 * 60000 + 3 * 1000 + 4), "01:02:03:004");
    // 9:08:07.006
    EXPECT_STREQ(TimeToChar(9 * 3600000 + 8 * 60000 + 7 * 1000 + 6), "09:08:07:006");
    // 10:11:12.013
    EXPECT_STREQ(TimeToChar(10 * 3600000 + 11 * 60000 + 12 * 1000 + 13), "10:11:12:013");
}
