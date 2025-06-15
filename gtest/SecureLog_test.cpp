#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include "SecureLog.h"

const char* kLogFilePath = "securelog.txt";

// 유틸 함수: 로그 파일 삭제
void DeleteLogFileIfExists() {
    std::remove(kLogFilePath);
}

// 유틸 함수: 로그 파일 열기
std::string ReadLogFileContent() {
    std::ifstream file(kLogFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 1. 로그 메시지 기록 확인
TEST(SecureLogTest, LogInfo_WritesMessageToFile) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    std::string testMessage = "Unit test log message";
    SecureLog::LogInfo(testMessage);

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find(testMessage), std::string::npos)
        << "로그 파일에 메시지가 기록되지 않았습니다.";
}

// 2. 여러 개 로그 메시지 기록
TEST(SecureLogTest, LogInfo_MultipleMessages) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("Message 1");
    SecureLog::LogInfo("Message 2");

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find("Message 1"), std::string::npos);
    ASSERT_NE(content.find("Message 2"), std::string::npos);
}

// 3. 로그 카운터 리셋 후 라인 번호 확인 (라인번호는 [n]으로 출력됨)
#if 0
TEST(SecureLogTest, LogCounterReset) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("First");
    SecureLog::ResetLogCounter();  // 리셋
    SecureLog::LogInfo("Second");

    std::string content = ReadLogFileContent();

    size_t pos1 = content.find("[0] First");
    size_t pos2 = content.find("[0] Second");

    ASSERT_NE(pos1, std::string::npos);
    ASSERT_NE(pos2, std::string::npos);
}
#else
TEST(SecureLogTest, LogCounterReset) {
    DeleteLogFileIfExists();                // 로그 파일 삭제
    SecureLog::ResetLogCounter();           // 카운터 1로 초기화

    SecureLog::LogInfo("First");            // → 로그 순번 [1]
    SecureLog::ResetLogCounter();           // 다시 1로 리셋
    SecureLog::LogInfo("Second");           // → 로그 순번 [1]

    std::string content = ReadLogFileContent();
    std::cout << "로그 내용:\n" << content << std::endl;

    // "First"가 포함된 줄을 찾아 로그 카운터 [1]인지 확인
    size_t posFirst = content.find("First");
    ASSERT_NE(posFirst, std::string::npos) << "First 메시지를 찾을 수 없음";

    size_t lineStartFirst = content.rfind('\n', posFirst);
    std::string lineFirst = content.substr(lineStartFirst + 1, content.find('\n', posFirst) - lineStartFirst - 1);
    ASSERT_NE(lineFirst.find("[1]"), std::string::npos) << "First 로그의 카운터가 [1]이 아님";

    // "Second"가 포함된 줄을 찾아 로그 카운터 [1]인지 확인
    size_t posSecond = content.find("Second");
    ASSERT_NE(posSecond, std::string::npos) << "Second 메시지를 찾을 수 없음";

    size_t lineStartSecond = content.rfind('\n', posSecond);
    std::string lineSecond = content.substr(lineStartSecond + 1, content.find('\n', posSecond) - lineStartSecond - 1);
    ASSERT_NE(lineSecond.find("[1]"), std::string::npos) << "Second 로그의 카운터가 [1]이 아님";
}
#endif

// 4. 특수 문자 로그 메시지 기록
TEST(SecureLogTest, LogInfo_SpecialCharacters) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    std::string special = "Log with !@#$%^&*()_+=-{}[];:'\",.<>?";
    SecureLog::LogInfo(special);

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find(special), std::string::npos);
}

// 5. 빈 문자열 로그 기록
TEST(SecureLogTest, LogInfo_EmptyMessage) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("");

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find("[]"), std::string::npos); // 공백이지만 라인 수는 올라감
}

// 6. 로그 파일이 자동 생성되는지 확인
TEST(SecureLogTest, LogFile_CreatedAutomatically) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("Create test");

    std::ifstream file(kLogFilePath);
    ASSERT_TRUE(file.is_open()) << "로그 파일이 자동 생성되지 않았습니다.";
}
