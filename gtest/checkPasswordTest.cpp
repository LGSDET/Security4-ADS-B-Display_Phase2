
#include "gtest/gtest.h"
#include "checkPassword.h"
#include "checkPassword_logic.h"


TEST(CheckPasswordTest, CorrectPassword_ReturnsTrue) {
    std::string correct = "T9@eZ#6pW!mX2$vR";  // 실제 시스템에 설정된 해시값과 맞추어야 함
    EXPECT_TRUE(checkPassword(correct));
}

TEST(CheckPasswordTest, WrongPassword_ReturnsFalse) {
    std::string wrong = "T9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vRT9@eZ#6pW!mX2$vR";
    EXPECT_FALSE(checkPassword(wrong));
}

TEST(CheckPasswordTest, EmptyPassword_ReturnsFalse) {
    std::string empty = "";
    EXPECT_FALSE(checkPassword(empty));
}

TEST(CheckPasswordTest, SpecialCharacters_ReturnsFalse) {
    std::string special = "@dm!n#";
    EXPECT_FALSE(checkPassword(special));
}
