#include <gtest/gtest.h>
#include "Encryptor.h"


class EncryptorTest : public ::testing::Test {
protected:
    Encryptor encryptor; 
    // �Ҹ��� ���� �߰�
    ~EncryptorTest() override;
};

EncryptorTest::~EncryptorTest() {
    // ���� Ư���� �ڿ� ���� �ʿ� ���� �� �� ���� ����
}
TEST_F(EncryptorTest, EncryptAndDecrypt_ReturnsOriginal) {
    AnsiString original = "�׽�Ʈ �޽���123!@#";
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
    AnsiString original = "���� �Է°�";
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
    AnsiString original = "���� �޽���";
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
