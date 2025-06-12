#include "gtest/gtest.h"
#include "Aircraft.h"
#include "DecodeRawADS_B.h"
#include "PointInPolygon.h"

extern "C" {
    //bool PointInPolygon(pfVec3* Verts, int NumVerts, pfVec3 Point);
    //bool IsPointOnSegment(const pfVec3& a, const pfVec3& b, const pfVec3 p);
    __int64 GetCurrentTimeInMsec(void);
    char* TimeToChar(__int64 hmsm);
}

// PointInPolygon 함수 테스트
class PointInPolygonTest : public ::testing::Test {
protected:
    void SetUp() override {}
};


// PointInPolygon 경계 체크(변 위, 꼭짓점) 테스트
TEST_F(PointInPolygonTest, EdgeAndVertexCases) {
    pfVec3 square[4] = { {0,0,0}, {4,0,0}, {4,4,0}, {0,4,0} };

    // 변 위
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 2,0,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 4,2,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 2,4,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 0,2,0 }));

    // 꼭짓점
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 0,0,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 4,0,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 4,4,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 0,4,0 }));

    // 내부
    EXPECT_TRUE(PointInPolygon(square, 4, pfVec3{ 2,2,0 }));

    // 외부
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 5,5,0 }));
}


// 단순 삼각형 내부/외부/경계 테스트
TEST_F(PointInPolygonTest, SimpleTriangle) {
    pfVec3 triangle[3] = { {0,0,0}, {5,0,0}, {0,5,0} };
    EXPECT_TRUE(PointInPolygon(triangle, 3, pfVec3{ 1,1,0 }));
    EXPECT_FALSE(PointInPolygon(triangle, 3, pfVec3{ 6,6,0 }));
    EXPECT_FALSE(PointInPolygon(triangle, 3, pfVec3{ 0,0,0 })); // 꼭짓점
    EXPECT_FALSE(PointInPolygon(triangle, 3, pfVec3{ 2.5,0,0 })); //변 위
}

// 사각형 내부/외부/경계 테스트
TEST_F(PointInPolygonTest, SquareCases) {
    pfVec3 square[4] = { {0,0,0}, {4,0,0}, {4,4,0}, {0,4,0} };
    EXPECT_TRUE(PointInPolygon(square, 4, pfVec3{ 2,2,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 5,5,0 }));
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 0,0,0 }));    // 꼭지점
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 4,2,0 }));   // 변 위
    EXPECT_FALSE(PointInPolygon(square, 4, pfVec3{ 2,4,0 }));   // 변 위
}

// 복잡한 다각형(오목) 테스트
TEST_F(PointInPolygonTest, ConcavePolygon) {
    pfVec3 concave[5] = { {0,0,0}, {4,0,0}, {2,2,0}, {4,4,0}, {0,4,0} };
    EXPECT_TRUE(PointInPolygon(concave, 5, pfVec3{ 1,1,0 }));
    EXPECT_FALSE(PointInPolygon(concave, 5, pfVec3{ 3,1,0 }));
    EXPECT_TRUE(PointInPolygon(concave, 5, pfVec3{ 2,3,0 }));
}

// 꼭짓점이 음수/0/양수 혼합된 경우
TEST_F(PointInPolygonTest, NegativeCoordinates) {
    pfVec3 poly[4] = { {-2,-2,0}, {2,-2,0}, {2,2,0}, {-2,2,0} };
    EXPECT_TRUE(PointInPolygon(poly, 4, pfVec3{ 0,0,0 }));
    EXPECT_FALSE(PointInPolygon(poly, 4, pfVec3{ 3,0,0 }));
    EXPECT_FALSE(PointInPolygon(poly, 4, pfVec3{ -2,0,0 }));
}

// 2개 꼭짓점(선분) 테스트
TEST_F(PointInPolygonTest, DegenerateLine) {
    pfVec3 line[2] = { {0,0,0}, {1,0,0} };
    EXPECT_FALSE(PointInPolygon(line, 2, pfVec3{ 0.5,0,0 }));
    EXPECT_FALSE(PointInPolygon(line, 2, pfVec3{ 0,1,0 }));
}

// 1개 꼭짓점(점) 테스트
TEST_F(PointInPolygonTest, DegeneratePoint) {
    pfVec3 pt[1] = { {1,1,0} };
    EXPECT_FALSE(PointInPolygon(pt, 1, pfVec3{ 1,1,0 }));
    EXPECT_FALSE(PointInPolygon(pt, 1, pfVec3{ 0,0,0 }));
}

// PointInPolygon 함수 테스트
class IsPointOnSegmentTest : public ::testing::Test {
protected:
    void SetUp() override {}
};
/*/
// IsPointOnSegment 함수 단위 테스트
TEST(IsPointOnSegmentTest, BasicCases) {
    pfVec3 a = { 0, 0, 0 };
    pfVec3 b = { 4, 0, 0 };

    // 선분 위 (중간)
    pfVec3 p1 = { 2, 0, 0 };
    EXPECT_TRUE(IsPointOnSegment(a, b, p1));

    // 선분 위 (끝점)
    pfVec3 p2 = { 0, 0, 0 };
    pfVec3 p3 = { 4, 0, 0 };
    EXPECT_TRUE(IsPointOnSegment(a, b, p2));
    EXPECT_TRUE(IsPointOnSegment(a, b, p3));

    // 선분 밖 (연장선)
    pfVec3 p4 = { 5, 0, 0 };
    pfVec3 p5 = { -1, 0, 0 };
    EXPECT_FALSE(IsPointOnSegment(a, b, p4));
    EXPECT_FALSE(IsPointOnSegment(a, b, p5));

    // 선분 밖 (직선 위지만 y 다름)
    pfVec3 p6 = { 2, 1, 0 };
    EXPECT_FALSE(IsPointOnSegment(a, b, p6));

    // 대각선 위
    pfVec3 c = { 0, 0, 0 };
    pfVec3 d = { 4, 4, 0 };
    pfVec3 p7 = { 2, 2, 0 };
    EXPECT_TRUE(IsPointOnSegment(c, d, p7));
    pfVec3 p8 = { 3, 3, 0 };
    EXPECT_TRUE(IsPointOnSegment(c, d, p8));
    pfVec3 p9 = { 5, 5, 0 };
    EXPECT_FALSE(IsPointOnSegment(c, d, p9));
}


class GetCurrentTimeInMsec : public ::testing::Test {
protected:
    void SetUp() override {}
};

// GetCurrentTimeInMsec 테스트
TEST(GetCurrentTimeInMsec, GetCurrentTimeInMsec_Basic) {
    __int64 t1 = GetCurrentTimeInMsec();
    Sleep(10); // 10ms 대기
    __int64 t2 = GetCurrentTimeInMsec();
    EXPECT_LE(t1, t2); // t2가 t1보다 크거나 같아야 함
    EXPECT_GT(t2 - t1, 0); // 시간은 증가해야 함
}
*/