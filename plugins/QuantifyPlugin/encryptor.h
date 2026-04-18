#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QString>
#include <QByteArray>

class Encryptor
{
public:
    // 初始化：公钥从资源文件 :/keys/public.pem 加载，私钥路径可选
    static void init(const QString& privateKeyPath = QString());

    static bool keysMatch();
    // 加密内存数据（使用私钥），返回加密后的完整数据（包含头部）
    static QByteArray encryptData(const QByteArray& plainData);

    // 解密内存数据（自动识别是否加密），成功返回明文，失败返回空
    static QByteArray decryptData(const QByteArray& encryptedData);

    // 加密文件到文件（使用私钥）
    static bool encryptFile(const QString& inputPath, const QString& outputPath);

    // 解密文件到文件（使用公钥），自动识别是否加密
    static bool decryptFile(const QString& inputPath, const QString& outputPath);

    // 检查数据是否已加密（读取魔数）
    static bool isEncrypted(const QByteArray& data);
    static bool isEncryptedFile(const QString& filePath);

    // 生成密钥对（教师初始化U盘时调用），私钥保存到 privateKeyPath，公钥保存到 publicKeyPath
    static bool generateKeyPair(const QString& privateKeyPath, const QString& publicKeyPath);

    // 批量迁移目录下所有 .record 文件（根据目标模式加密/解密）
    static bool migrateRecordDirectory(const QString& dirPath, bool enableEncryption);

    // 获取当前是否可用（私钥已加载）
    static bool hasPrivateKey();
    static QString getPrivateKeyPath();
private:
    static bool loadPrivateKey(const QString& path);
    static bool loadPublicKeyFromResource();
    static QByteArray encryptAESKeyWithPrivateKey(const QByteArray& aesKey);
    static QByteArray decryptAESKeyWithPublicKey(const QByteArray& encryptedKey);
    static QByteArray generateAESKey();

    static void* m_privateKey;   // RSA*
    static void* m_publicKey;    // RSA*
    static bool m_initialized;
    static QString m_privateKeyPath;
};

#endif // ENCRYPTOR_H