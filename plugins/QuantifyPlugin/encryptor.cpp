#include "encryptor.h"
#include "logger.h"
#include <QDir>
#include <QFile>
#include <QResource>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

void *Encryptor::m_privateKey = nullptr;
void *Encryptor::m_publicKey = nullptr;
bool Encryptor::m_initialized = false;
QString Encryptor::m_privateKeyPath;

///
/// \brief logOpenSSLError
/// \param context
///
static void logOpenSSLError(const QString &context) {
    char buf[256];
    unsigned long err;
    while ((err = ERR_get_error()) != 0) {
        ERR_error_string_n(err, buf, sizeof(buf));
        Logger::instance().error(QString("%1: %2").arg(context, buf));
    }
}
///
/// \brief Encryptor::init
/// \param privateKeyPath
///
void Encryptor::init(const QString &privateKeyPath) {
    m_privateKeyPath = QDir::cleanPath(privateKeyPath);
    if (!m_privateKeyPath.isEmpty()) {
        loadPrivateKey(m_privateKeyPath);
    } else {
        m_privateKey = nullptr;
    }
    if (!m_initialized) {
        if (!loadPublicKeyFromResource()) {
            Logger::instance().error("无法加载内置公钥，加密功能不可用");
        }
    }
    m_initialized = true;
}
///
/// \brief Encryptor::keysMatch
/// \return
///
bool Encryptor::keysMatch() {
    if (!m_privateKey || !m_publicKey)
        return false;

    RSA *priv = (RSA *)m_privateKey;
    RSA *pub = (RSA *)m_publicKey;

    const BIGNUM *priv_n = nullptr, *priv_e = nullptr, *priv_d = nullptr;
    const BIGNUM *pub_n = nullptr, *pub_e = nullptr;

    // 获取私钥中的 n, e, d
    RSA_get0_key(priv, &priv_n, &priv_e, &priv_d);
    // 获取公钥中的 n, e
    RSA_get0_key(pub, &pub_n, &pub_e, nullptr);

    if (!priv_n || !pub_n)
        return false;

    // 比较模数 n 是否相同
    return BN_cmp(priv_n, pub_n) == 0;
}
///
/// \brief Encryptor::loadPrivateKey
/// \param path
/// \return
///
bool Encryptor::loadPrivateKey(const QString &path) {
    if (m_privateKey)
        return true;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开私钥文件: " + path);
        return false;
    }
    QByteArray keyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) {
        logOpenSSLError("加载私钥失败");
        return false;
    }
    m_privateKey = rsa;
    Logger::instance().info("私钥加载成功: " + path);
    return true;
}
///
/// \brief Encryptor::loadPublicKeyFromResource
/// \return
///
bool Encryptor::loadPublicKeyFromResource() {
    if (m_publicKey)
        return true;

    QFile keyFile(":/keys/keys/public.pem");
    if (!keyFile.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开资源文件 :/keys/keys/public.pem");
        return false;
    }
    QByteArray keyData = keyFile.readAll();
    keyFile.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) {
        logOpenSSLError("加载公钥失败");
        return false;
    }
    m_publicKey = rsa;
    Logger::instance().info("公钥加载成功");
    return true;
}
///
/// \brief Encryptor::generateAESKey
/// \return
///
QByteArray Encryptor::generateAESKey() {
    QByteArray key(32, '\0');
    if (RAND_bytes((unsigned char *)key.data(), 32) != 1) {
        logOpenSSLError("生成AES密钥失败");
        return QByteArray();
    }
    return key;
}
///
/// \brief Encryptor::encryptAESKeyWithPrivateKey
/// \param aesKey
/// \return
///
QByteArray Encryptor::encryptAESKeyWithPrivateKey(const QByteArray &aesKey) {
    if (!m_privateKey) {
        Logger::instance().error("私钥未加载，无法加密AES密钥");
        return QByteArray();
    }
    RSA *rsa = (RSA *)m_privateKey;
    int rsaLen = RSA_size(rsa);
    QByteArray encrypted(rsaLen, '\0');
    int ret = RSA_private_encrypt(
        aesKey.size(), (const unsigned char *)aesKey.data(),
        (unsigned char *)encrypted.data(), rsa, RSA_PKCS1_PADDING);
    if (ret == -1) {
        logOpenSSLError("RSA私钥加密失败");
        return QByteArray();
    }
    return encrypted;
}
///
/// \brief Encryptor::decryptAESKeyWithPublicKey
/// \param encryptedKey
/// \return
///
QByteArray
Encryptor::decryptAESKeyWithPublicKey(const QByteArray &encryptedKey) {
    if (!m_publicKey) {
        Logger::instance().error("公钥未加载，无法解密AES密钥");
        return QByteArray();
    }
    RSA *rsa = (RSA *)m_publicKey;
    int rsaLen = RSA_size(rsa);
    if (encryptedKey.size() != rsaLen) {
        Logger::instance().error("加密的AES密钥长度错误");
        return QByteArray();
    }
    QByteArray aesKey(256,
                      '\0'); // AES-256 密钥长度 32
    // 字节，但RSA解密输出可能填充，实际长度由函数返回
    int ret = RSA_public_decrypt(
        encryptedKey.size(), (const unsigned char *)encryptedKey.data(),
        (unsigned char *)aesKey.data(), rsa, RSA_PKCS1_PADDING);
    if (ret == -1) {
        logOpenSSLError("RSA公钥解密失败");
        return QByteArray();
    }
    aesKey.resize(ret);
    return aesKey;
}
///
/// \brief Encryptor::encryptData
/// \param plainData
/// \return
///
QByteArray Encryptor::encryptData(const QByteArray &plainData) {
    if (!m_initialized || !m_privateKey) {
        Logger::instance().error("加密失败：私钥未加载");
        return QByteArray();
    }

    // 生成 AES-256 密钥和 Nonce
    QByteArray aesKey = generateAESKey();
    if (aesKey.isEmpty())
        return QByteArray();

    unsigned char nonce[12];
    if (RAND_bytes(nonce, sizeof(nonce)) != 1) {
        logOpenSSLError("生成随机Nonce失败");
        return QByteArray();
    }

    // 用私钥加密 AES 密钥
    QByteArray encryptedKey = encryptAESKeyWithPrivateKey(aesKey);
    if (encryptedKey.isEmpty())
        return QByteArray();

    // AES-GCM 加密
    QByteArray cipherText;
    QByteArray tag(16, '\0');
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Logger::instance().error("创建EVP上下文失败");
        return QByteArray();
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) !=
            1 ||
        EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                                                                                         (const unsigned char *)aesKey.data(), nonce) != 1) {
        logOpenSSLError("AES-GCM初始化失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    int outlen = 0;
    cipherText.resize(plainData.size() + EVP_MAX_BLOCK_LENGTH);
    if (EVP_EncryptUpdate(ctx, (unsigned char *)cipherText.data(), &outlen,
                          (const unsigned char *)plainData.data(),
                          plainData.size()) != 1) {
        logOpenSSLError("AES-GCM加密更新失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    cipherText.resize(outlen);

    if (EVP_EncryptFinal_ex(ctx, (unsigned char *)cipherText.data() + outlen,
                            &outlen) != 1) {
        logOpenSSLError("AES-GCM加密最终失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    cipherText.resize(cipherText.size() + outlen);

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16,
                            (unsigned char *)tag.data()) != 1) {
        logOpenSSLError("获取GCM标签失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    EVP_CIPHER_CTX_free(ctx);

    // 组装数据：魔数(4) + 加密密钥(256) + nonce(12) + tag(16) + 密文
    QByteArray result;
    result.append("QCRY");
    result.append(encryptedKey);
    result.append((char *)nonce, 12);
    result.append(tag);
    result.append(cipherText);
    return result;
}
///
/// \brief Encryptor::decryptData
/// \param encryptedData
/// \return
///
QByteArray Encryptor::decryptData(const QByteArray &encryptedData) {
    if (!isEncrypted(encryptedData)) {
        // 未加密，直接返回原始数据（无需公钥）
        return encryptedData;
    }

    if (!m_initialized || !m_publicKey) {
        Logger::instance().error("解密失败：文件已加密但公钥未加载");
        return QByteArray();
    }

    const int keyLen = 256; // RSA-2048 加密后长度固定256字节
    const int nonceLen = 12;
    const int tagLen = 16;
    int pos = 4; // 跳过魔数

    if (encryptedData.size() < pos + keyLen + nonceLen + tagLen) {
        Logger::instance().error("加密数据长度不足");
        return QByteArray();
    }

    QByteArray encryptedKey = encryptedData.mid(pos, keyLen);
    pos += keyLen;
    QByteArray nonce = encryptedData.mid(pos, nonceLen);
    pos += nonceLen;
    QByteArray tag = encryptedData.mid(pos, tagLen);
    pos += tagLen;
    QByteArray cipherText = encryptedData.mid(pos);

    // 用公钥解密 AES 密钥
    QByteArray aesKey = decryptAESKeyWithPublicKey(encryptedKey);
    if (aesKey.isEmpty())
        return QByteArray();

    // AES-GCM 解密
    QByteArray plainText;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Logger::instance().error("创建EVP上下文失败");
        return QByteArray();
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) !=
            1 ||
        EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                (const unsigned char *)aesKey.data(),
                (const unsigned char *)nonce.data()) != 1) {
        logOpenSSLError("AES-GCM解密初始化失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    int outlen = 0;
    plainText.resize(cipherText.size());
    if (EVP_DecryptUpdate(ctx, (unsigned char *)plainText.data(), &outlen,
                          (const unsigned char *)cipherText.data(),
                          cipherText.size()) != 1) {
        logOpenSSLError("AES-GCM解密更新失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plainText.resize(outlen);

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tagLen,
                            (void *)tag.data()) != 1) {
        logOpenSSLError("设置GCM标签失败");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)plainText.data() + outlen,
                            &finalLen) != 1) {
        logOpenSSLError("AES-GCM解密最终失败，标签验证错误");
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plainText.resize(plainText.size() + finalLen);
    EVP_CIPHER_CTX_free(ctx);

    return plainText;
}

bool Encryptor::encryptFile(const QString &inputPath,
                            const QString &outputPath) {
    QFile inFile(inputPath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开输入文件: " + inputPath);
        return false;
    }
    QByteArray plainData = inFile.readAll();
    inFile.close();

    QByteArray encrypted = encryptData(plainData);
    if (encrypted.isEmpty())
        return false;

    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        Logger::instance().error("无法写入输出文件: " + outputPath);
        return false;
    }
    outFile.write(encrypted);
    outFile.close();
    return true;
}

bool Encryptor::decryptFile(const QString &inputPath,
                            const QString &outputPath) {
    QFile inFile(inputPath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开输入文件: " + inputPath);
        return false;
    }
    QByteArray encryptedData = inFile.readAll();
    inFile.close();

    QByteArray plainData = decryptData(encryptedData);
    if (plainData.isEmpty() && !encryptedData.isEmpty()) {
        Logger::instance().error("解密文件失败: " + inputPath);
        return false;
    }

    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        Logger::instance().error("无法写入输出文件: " + outputPath);
        return false;
    }
    outFile.write(plainData);
    outFile.close();
    return true;
}

bool Encryptor::isEncrypted(const QByteArray &data) {
    return data.size() >= 4 && data.left(4) == "QCRY";
}

bool Encryptor::isEncryptedFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    char magic[4];
    if (file.read(magic, 4) != 4) {
        file.close();
        return false;
    }
    file.close();
    return memcmp(magic, "QCRY", 4) == 0;
}

bool Encryptor::generateKeyPair(const QString &privateKeyPath,
                                const QString &publicKeyPath) {
    int bits = 2048;
    unsigned long e = RSA_F4;
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    BN_set_word(bn, e);
    if (!RSA_generate_key_ex(rsa, bits, bn, nullptr)) {
        logOpenSSLError("生成RSA密钥对失败");
        RSA_free(rsa);
        BN_free(bn);
        return false;
    }
    BN_free(bn);

    // 保存私钥（PKCS#1 格式）
    BIO *bio_priv = BIO_new_file(privateKeyPath.toUtf8().constData(), "w");
    if (!bio_priv) {
        Logger::instance().error("无法创建私钥文件: " + privateKeyPath);
        RSA_free(rsa);
        return false;
    }
    if (!PEM_write_bio_RSAPrivateKey(bio_priv, rsa, nullptr, nullptr, 0, nullptr,
                                     nullptr)) {
        logOpenSSLError("写入私钥失败");
        BIO_free(bio_priv);
        RSA_free(rsa);
        return false;
    }
    BIO_free(bio_priv);

    // 保存公钥（PKCS#8 格式）
    BIO *bio_pub = BIO_new_file(publicKeyPath.toUtf8().constData(), "w");
    if (!bio_pub) {
        Logger::instance().error("无法创建公钥文件: " + publicKeyPath);
        RSA_free(rsa);
        return false;
    }
    if (!PEM_write_bio_RSA_PUBKEY(bio_pub, rsa)) {
        logOpenSSLError("写入公钥失败");
        BIO_free(bio_pub);
        RSA_free(rsa);
        return false;
    }
    BIO_free(bio_pub);

    RSA_free(rsa);
    Logger::instance().info("密钥对生成成功: " + privateKeyPath + ", " +
                            publicKeyPath);
    return true;
}

bool Encryptor::migrateRecordDirectory(const QString &dirPath,
                                       bool enableEncryption) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        Logger::instance().error("目录不存在: " + dirPath);
        return false;
    }
    QStringList filters;
    filters << "*.record";
    dir.setNameFilters(filters);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    bool allOk = true;
    for (const QFileInfo &fi : files) {
        QString originalPath = fi.absoluteFilePath();
        QString tempPath = originalPath + ".tmp";
        if (enableEncryption) {
            // 明文 -> 密文
            if (!isEncryptedFile(originalPath)) {
                if (!encryptFile(originalPath, tempPath)) {
                    allOk = false;
                    continue;
                }
                if (!QFile::remove(originalPath) ||
                    !QFile::rename(tempPath, originalPath)) {
                    Logger::instance().error("替换文件失败: " + originalPath);
                    allOk = false;
                }
            }
        } else {
            // 密文 -> 明文
            if (isEncryptedFile(originalPath)) {
                if (!decryptFile(originalPath, tempPath)) {
                    allOk = false;
                    continue;
                }
                if (!QFile::remove(originalPath) ||
                    !QFile::rename(tempPath, originalPath)) {
                    Logger::instance().error("替换文件失败: " + originalPath);
                    allOk = false;
                }
            }
        }
    }
    return allOk;
}

bool Encryptor::hasPrivateKey() { return m_privateKey != nullptr; }

QString Encryptor::getPrivateKeyPath() { return m_privateKeyPath; }