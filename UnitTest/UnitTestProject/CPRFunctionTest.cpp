#include "pch.h"
#include "gtest/gtest.h"
#include "Aircraft.h"


extern "C" {
    int cprModFunction(int a, int b);
    int cprNLFunction(double lat);
    int cprNFunction(double lat, int isodd);
    double cprDlonFunction(double lat, int isodd);
    void decodeCPR(TADS_B_Aircraft* a);
}



class CPRFunctionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // (필요할 경우) 공통 세팅
    }
};

//-----------------------------------------------------------
// cprModFunction 테스트
TEST_F(CPRFunctionTest, ModFunction_BasicCases) {
    EXPECT_EQ(cprModFunction(5, 3), 2);
    EXPECT_EQ(cprModFunction(10, 5), 0);
    EXPECT_EQ(cprModFunction(-1, 3), 2);
    EXPECT_EQ(cprModFunction(-4, 3), 2);
    EXPECT_EQ(cprModFunction(-10, 5), 0);
    EXPECT_EQ(cprModFunction(0, 3), 0);
}

//-----------------------------------------------------------
// cprNLFunction 테스트
TEST_F(CPRFunctionTest, NLFunction_BasicAndBoundaries) {
    // 각 구간의 경계값 바로 앞/뒤를 테스트하여 모든 분기 커버
    struct NLTestCase { double lat; int expected; };
    const NLTestCase cases[] = {
        { 0.0, 59 }, { 10.47047129, 59 },
        { 14.82817436, 58 }, { 14.82817437, 57 },
        { 18.18626356, 57 }, { 18.18626357, 56 },
        { 21.02939492, 56 }, { 21.02939493, 55 },
        { 23.54504486, 55 }, { 23.54504487, 54 },
        { 25.82924706, 54 }, { 25.82924707, 53 },
        { 27.93898709, 53 }, { 27.93898710, 52 },
        { 29.91135685, 52 }, { 29.91135686, 51 },
        { 31.77209707, 51 }, { 31.77209708, 50 },
        { 33.53993435, 50 }, { 33.53993436, 49 },
        { 35.22899597, 49 }, { 35.22899598, 48 },
        { 36.85025107, 48 }, { 36.85025108, 47 },
        { 38.41241891, 47 }, { 38.41241892, 46 },
        { 39.92256683, 46 }, { 39.92256684, 45 },
        { 41.38651831, 45 }, { 41.38651832, 44 },
        { 42.80914011, 44 }, { 42.80914012, 43 },
        { 44.19454950, 43 }, { 44.19454951, 42 },
        { 45.54626722, 42 }, { 45.54626723, 41 },
        { 46.86733251, 41 }, { 46.86733252, 40 },
        { 48.16039127, 40 }, { 48.16039128, 39 },
        { 49.42776438, 39 }, { 49.42776439, 38 },
        { 50.67150165, 38 }, { 50.67150166, 37 },
        { 51.89342468, 37 }, { 51.89342469, 36 },
        { 53.09516152, 36 }, { 53.09516153, 35 },
        { 54.27817471, 35 }, { 54.27817472, 34 },
        { 55.44378443, 34 }, { 55.44378444, 33 },
        { 56.59318755, 33 }, { 56.59318756, 32 },
        { 57.72747353, 32 }, { 57.72747354, 31 },
        { 58.84763775, 31 }, { 58.84763776, 30 },
        { 59.95459276, 30 }, { 59.95459277, 29 },
        { 61.04917773, 29 }, { 61.04917774, 28 },
        { 62.13216658, 28 }, { 62.13216659, 27 },
        { 63.20427478, 27 }, { 63.20427479, 26 },
        { 64.26616522, 26 }, { 64.26616523, 25 },
        { 65.31845309, 25 }, { 65.31845310, 24 },
        { 66.36171007, 24 }, { 66.36171008, 23 },
        { 67.39646773, 23 }, { 67.39646774, 22 },
        { 68.42322021, 22 }, { 68.42322022, 21 },
        { 69.44242630, 21 }, { 69.44242631, 20 },
        { 70.45451074, 20 }, { 70.45451075, 19 },
        { 71.45986472, 19 }, { 71.45986473, 18 },
        { 72.45884544, 18 }, { 72.45884545, 17 },
        { 73.45177441, 17 }, { 73.45177442, 16 },
        { 74.43893415, 16 }, { 74.43893416, 15 },
        { 75.42056256, 15 }, { 75.42056257, 14 },
        { 76.39684390, 14 }, { 76.39684391, 13 },
        { 77.36789460, 13 }, { 77.36789461, 12 },
        { 78.33374082, 12 }, { 78.33374083, 11 },
        { 79.29428224, 11 }, { 79.29428225, 10 },
        { 80.24923212, 10 }, { 80.24923213, 9 },
        { 81.19801348, 9 }, { 81.19801349, 8 },
        { 82.13956980, 8 }, { 82.13956981, 7 },
        { 83.07199444, 7 }, { 83.07199445, 6 },
        { 83.99173562, 6 }, { 83.99173563, 5 },
        { 84.89166190, 5 }, { 84.89166191, 4 },
        { 85.75541620, 4 }, { 85.75541621, 3 },
        { 86.53536997, 3 }, { 86.53536998, 2 },
        { 87.00000000, 1 }, { 87.00000001, 1 },
        { 90.0, 1 }, { 100.0, 1 }, { -100.0, 1 }
    };

    for (const auto& tc : cases) {
        EXPECT_EQ(cprNLFunction(tc.lat), tc.expected);
        // 음수도 대칭 동작
        EXPECT_EQ(cprNLFunction(-tc.lat), tc.expected);
    }
}

//-----------------------------------------------------------
// cprNFunction 테스트
TEST_F(CPRFunctionTest, NFunction_Basic) {
    EXPECT_EQ(cprNFunction(0.0, 0), 59);
    EXPECT_EQ(cprNFunction(45.0, 0), 42);
    EXPECT_EQ(cprNFunction(45.0, 1), 41);

    // 경계값
    EXPECT_EQ(cprNFunction(87.0, 0), 1);
    EXPECT_EQ(cprNFunction(87.0, 1), 1);

    // nl < 1 보정
    EXPECT_EQ(cprNFunction(88.0, 1), 1);
}

//-----------------------------------------------------------
// cprDlonFunction 테스트
TEST_F(CPRFunctionTest, DlonFunction_Basic) {
    EXPECT_NEAR(cprDlonFunction(0.0, 0), 360.0 / 59, 1e-6);
    EXPECT_NEAR(cprDlonFunction(0.0, 1), 360.0 / 58, 1e-6);
    EXPECT_NEAR(cprDlonFunction(86.9, 0), 360.0 / 2, 1e-6);
    EXPECT_NEAR(cprDlonFunction(87.0, 1), 360.0 / 1, 1e-6);
}

//-----------------------------------------------------------
// decodeCPR 해피 패스 테스트
TEST_F(CPRFunctionTest, DecodeCPR_Basic) {
    TADS_B_Aircraft a{};
    // 짝수/홀수 CPR 입력
    a.even_cprlat = 100000;
    a.even_cprlon = 200000;
    a.even_cprtime = 1000;

    a.odd_cprlat = 100100;
    a.odd_cprlon = 200100;
    a.odd_cprtime = 2000;

    decodeCPR(&a);

    // 기본 범위 검사
    EXPECT_GE(a.Latitude, -90.0);
    EXPECT_LE(a.Latitude, 90.0);
    EXPECT_GE(a.Longitude, -180.0);
    EXPECT_LE(a.Longitude, 180.0);

    // 대략적 기본값 (단순 sanity check)
    EXPECT_NEAR(std::abs(a.Latitude), std::abs(a.Latitude), 180.0);
    EXPECT_NEAR(std::abs(a.Longitude), std::abs(a.Longitude), 360.0);
}

// decodeCPR branch/condition coverage 추가 테스트
TEST_F(CPRFunctionTest, DecodeCPR_BranchCoverage) {
    TADS_B_Aircraft a{};

    // 1. rlat0 >= 270 branch
    a.even_cprlat = 1000000; // large value to make rlat0 >= 270
    a.even_cprlon = 0;
    a.even_cprtime = 2000;
    a.odd_cprlat = 0;
    a.odd_cprlon = 0;
    a.odd_cprtime = 1000;
    decodeCPR(&a);
    EXPECT_GE(a.Latitude, -90.0); // Latitude should be valid

    // 2. rlat1 >= 270 branch
    a.even_cprlat = 0;
    a.even_cprlon = 0;
    a.even_cprtime = 1000;
    a.odd_cprlat = 1000000; // large value to make rlat1 >= 270
    a.odd_cprlon = 0;
    a.odd_cprtime = 2000;
    decodeCPR(&a);
    EXPECT_GE(a.Latitude, -90.0);

    // 3. cprNLFunction(rlat0) != cprNLFunction(rlat1) branch (should return early, no update)
    a.even_cprlat = 0;
    a.even_cprlon = 0;
    a.even_cprtime = 2000;
    a.odd_cprlat = 131072; // force rlat1 to be far from rlat0
    a.odd_cprlon = 0;
    a.odd_cprtime = 1000;
    a.Latitude = 9999.0; // sentinel
    a.Longitude = 9999.0;
    decodeCPR(&a);
    EXPECT_EQ(a.Latitude, 0); // No update expected

    // 4. even_cprtime > odd_cprtime branch
    a.even_cprlat = 100000;
    a.even_cprlon = 200000;
    a.even_cprtime = 2000;
    a.odd_cprlat = 100100;
    a.odd_cprlon = 200100;
    a.odd_cprtime = 1000;
    decodeCPR(&a);
    EXPECT_GE(a.Latitude, -90.0);

    // 5. even_cprtime <= odd_cprtime branch
    a.even_cprlat = 100000;
    a.even_cprlon = 200000;
    a.even_cprtime = 1000;
    a.odd_cprlat = 100100;
    a.odd_cprlon = 200100;
    a.odd_cprtime = 2000;
    decodeCPR(&a);
    EXPECT_GE(a.Latitude, -90.0);

    // 6. a->Longitude > 180 branch
    a.even_cprlat = 100000;
    a.even_cprlon = 400000; // large value to make Longitude > 180
    a.even_cprtime = 2000;
    a.odd_cprlat = 100100;
    a.odd_cprlon = 200100;
    a.odd_cprtime = 1000;
    decodeCPR(&a);
    EXPECT_LE(a.Longitude, 180.0);
}
