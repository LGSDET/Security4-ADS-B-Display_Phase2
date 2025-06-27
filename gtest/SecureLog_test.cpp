#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include "SecureLog.h"

const char* kLogFilePath = "securelog.txt";

// ��ƿ �Լ�: �α� ���� ����
void DeleteLogFileIfExists() {
    std::remove(kLogFilePath);
}

// ��ƿ �Լ�: �α� ���� ����
std::string ReadLogFileContent() {
    std::ifstream file(kLogFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 1. �α� �޽��� ��� Ȯ��
TEST(SecureLogTest, LogInfo_WritesMessageToFile) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    std::string testMessage = "Unit test log message";
    SecureLog::LogInfo(testMessage);

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find(testMessage), std::string::npos)
        << "�α� ���Ͽ� �޽����� ��ϵ��� �ʾҽ��ϴ�.";
}

// 2. ���� �� �α� �޽��� ���
TEST(SecureLogTest, LogInfo_MultipleMessages) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("Message 1");
    SecureLog::LogInfo("Message 2");

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find("Message 1"), std::string::npos);
    ASSERT_NE(content.find("Message 2"), std::string::npos);
}

// 3. �α� ī���� ���� �� ���� ��ȣ Ȯ�� (���ι�ȣ�� [n]���� ��µ�)
#if 0
TEST(SecureLogTest, LogCounterReset) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("First");
    SecureLog::ResetLogCounter();  // ����
    SecureLog::LogInfo("Second");

    std::string content = ReadLogFileContent();

    size_t pos1 = content.find("[0] First");
    size_t pos2 = content.find("[0] Second");

    ASSERT_NE(pos1, std::string::npos);
    ASSERT_NE(pos2, std::string::npos);
}
#else
TEST(SecureLogTest, LogCounterReset) {
    DeleteLogFileIfExists();                // �α� ���� ����
    SecureLog::ResetLogCounter();           // ī���� 1�� �ʱ�ȭ

    SecureLog::LogInfo("First");            // �� �α� ���� [1]
    SecureLog::ResetLogCounter();           // �ٽ� 1�� ����
    SecureLog::LogInfo("Second");           // �� �α� ���� [1]

    std::string content = ReadLogFileContent();
    std::cout << "�α� ����:\n" << content << std::endl;

    // "First"�� ���Ե� ���� ã�� �α� ī���� [1]���� Ȯ��
    size_t posFirst = content.find("First");
    ASSERT_NE(posFirst, std::string::npos) << "First �޽����� ã�� �� ����";

    size_t lineStartFirst = content.rfind('\n', posFirst);
    std::string lineFirst = content.substr(lineStartFirst + 1, content.find('\n', posFirst) - lineStartFirst - 1);
    ASSERT_NE(lineFirst.find("[1]"), std::string::npos) << "First �α��� ī���Ͱ� [1]�� �ƴ�";

    // "Second"�� ���Ե� ���� ã�� �α� ī���� [1]���� Ȯ��
    size_t posSecond = content.find("Second");
    ASSERT_NE(posSecond, std::string::npos) << "Second �޽����� ã�� �� ����";

    size_t lineStartSecond = content.rfind('\n', posSecond);
    std::string lineSecond = content.substr(lineStartSecond + 1, content.find('\n', posSecond) - lineStartSecond - 1);
    ASSERT_NE(lineSecond.find("[1]"), std::string::npos) << "Second �α��� ī���Ͱ� [1]�� �ƴ�";
}
#endif

// 4. Ư�� ���� �α� �޽��� ���
TEST(SecureLogTest, LogInfo_SpecialCharacters) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    std::string special = "Log with !@#$%^&*()_+=-{}[];:'\",.<>?";
    SecureLog::LogInfo(special);

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find(special), std::string::npos);
}

// 5. �� ���ڿ� �α� ���
TEST(SecureLogTest, LogInfo_EmptyMessage) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("");

    std::string content = ReadLogFileContent();
    ASSERT_NE(content.find("[]"), std::string::npos); // ���������� ���� ���� �ö�
}

// 6. �α� ������ �ڵ� �����Ǵ��� Ȯ��
TEST(SecureLogTest, LogFile_CreatedAutomatically) {
    DeleteLogFileIfExists();
    SecureLog::ResetLogCounter();

    SecureLog::LogInfo("Create test");

    std::ifstream file(kLogFilePath);
    ASSERT_TRUE(file.is_open()) << "�α� ������ �ڵ� �������� �ʾҽ��ϴ�.";
}
