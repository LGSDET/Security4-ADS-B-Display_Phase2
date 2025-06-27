#include <gtest/gtest.h>
#include "Encryptor.h"


class EncryptorTest : public ::testing::Test {
protected:
    Encryptor encryptor; 
    // 소멸자 선언 추가
    ~EncryptorTest() override;
};

EncryptorTest::~EncryptorTest() {
    // 현재 특별한 자원 해제 필요 없음 → 빈 구현 가능
}
TEST_F(EncryptorTest, EncryptAndDecrypt_ReturnsOriginal) {
    AnsiString original = "테스트 메시지123!@#";
    AnsiString encrypted = encryptor.Encrypt(original);
#ifdef _MSC_VER
    ASSERT_FALSE(encrypted.empty());
#else
    ASSERT_FALSE(encrypted.IsEmpty());
#endif

    AnsiString decrypted = encryptor.Decrypt(encrypted);
    ASSERT_EQ(original, decrypted);
}

TEST_F(EncryptorTest, Encrypt_ProducesDifferentCiphertextForSameInput) {
    AnsiString original = "같은 입력값";
    AnsiString encrypted1 = encryptor.Encrypt(original);
    AnsiString encrypted2 = encryptor.Encrypt(original);
    ASSERT_NE(encrypted1, encrypted2);
}

TEST_F(EncryptorTest, Decrypt_InvalidBase64_ReturnsEmpty) {
    AnsiString invalidBase64 = "!!!@@@###";
    AnsiString decrypted = encryptor.Decrypt(invalidBase64);
#ifdef _MSC_VER
    ASSERT_TRUE(decrypted.empty());
#else
    ASSERT_TRUE(decrypted.IsEmpty());
#endif
}

#if 0
TEST_F(EncryptorTest, Decrypt_TamperedCiphertext_ReturnsEmpty) {
    AnsiString original = "정상 메시지";
    AnsiString encrypted = encryptor.Encrypt(original);
    AnsiString tampered = encrypted.c_str();
#ifdef _MSC_VER
    if (!tampered.empty())
#else
    if (!tampered.IsEmpty())
#endif
    {
    tampered[0] = (tampered[0] == 'A' ? 'B' : 'A');
    }
    AnsiString decrypted = encryptor.Decrypt(tampered.c_str());
#ifdef _MSC_VER
    ASSERT_TRUE(decrypted.empty());
#else
    ASSERT_TRUE(decrypted.IsEmpty());
#endif
}
#endif
