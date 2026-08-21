/**
 * @file encryptor.cpp
 * @brief 加密类
 * @author howdy213
 * @date 2026-05-16
 * @version 2.0.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "encryptor.h"
#include "logger.h"

#include <QDir>
#include <QFile>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

// 静态成员初始化
void *Encryptor::m_privateKey = nullptr;
void *Encryptor::m_publicKey = nullptr;
QString Encryptor::m_privateKeyPath;
QString Encryptor::m_publicKeyPath;

// 辅助宏：获取 EVP_PKEY 指针
#define PRIV_KEY static_cast<EVP_PKEY *>(m_privateKey)
#define PUB_KEY static_cast<EVP_PKEY *>(m_publicKey)

static void logOpenSSLError(const QString &context) {
    char buf[256];
    unsigned long err;
    while ((err = ERR_get_error()) != 0) {
        ERR_error_string_n(err, buf, sizeof(buf));
        Logger::instance().error(QString("%1: %2").arg(context, buf));
    }
}

void Encryptor::init() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    // 不加载任何密钥
}

bool Encryptor::loadPrivateKey(const QString &path) {
    QString path2;
    path2 = QDir::cleanPath(path);
    if (m_privateKey) {
        EVP_PKEY_free(PRIV_KEY);
        m_privateKey = nullptr;
    }
    QFile file(path2);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开私钥文件: " + path2);
        return false;
    }
    QByteArray keyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        logOpenSSLError("加载私钥失败: " + path);
        return false;
    }
    m_privateKey = pkey;
    m_privateKeyPath = path2;
    Logger::instance().info("私钥加载成功: " + path);
    return true;
}

bool Encryptor::loadPublicKeyFromFile(const QString &filePath) {
    QString path2;
    path2 = QDir::cleanPath(filePath);
    // 若新公钥与当前私钥（如果存在）不匹配，但存在加密记录，则禁止替换
    if (m_privateKey && hasEncryptedRecords(QFileInfo(path2).dir().absolutePath())) {
        Logger::instance().error("尝试替换公钥，但存在加密记录文件，操作被拒绝");
        return false;
    }

    if (PUB_KEY) {
        EVP_PKEY_free(PUB_KEY);
        m_publicKey = nullptr;
        m_publicKeyPath.clear();
    }

    QFile file(path2);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开公钥文件: " + path2);
        return false;
    }
    QByteArray keyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        logOpenSSLError("加载公钥失败: " + path2);
        return false;
    }
    m_publicKey = pkey;
    m_publicKeyPath = path2;
    Logger::instance().info("公钥加载成功: " + path2);
    return true;
}

bool Encryptor::hasEncryptedRecords(const QString &recordDir) {
    return isEncryptionModeActive(recordDir + "/record");
}

void Encryptor::clearPublicKey() {
    if (m_publicKey) {
        EVP_PKEY_free(PUB_KEY);
        m_publicKey = nullptr;
    }
    m_publicKeyPath.clear();
}

void Encryptor::clearPrivateKey() {
    if (m_privateKey) {
        EVP_PKEY_free(PRIV_KEY);
        m_privateKey = nullptr;
    }
    m_privateKeyPath.clear();
}

bool Encryptor::hasPrivateKey() { return m_privateKey != nullptr; }
bool Encryptor::hasPublicKey() { return m_publicKey != nullptr; }
QString Encryptor::getPrivateKeyPath() { return m_privateKeyPath; }
QString Encryptor::getPublicKeyPath() { return m_publicKeyPath; }

// =============== 密钥匹配 ===============
bool Encryptor::keysMatch() {
    if (!m_privateKey || !m_publicKey)
        return false;

    EVP_PKEY *priv = PRIV_KEY;
    EVP_PKEY *pub = PUB_KEY;

    // 比较模数 n
    BIGNUM *priv_n = nullptr;
    BIGNUM *pub_n = nullptr;

    if (EVP_PKEY_get_bn_param(priv, "n", &priv_n) != 1)
        return false;
    if (EVP_PKEY_get_bn_param(pub, "n", &pub_n) != 1) {
        BN_free(priv_n);
        return false;
    }

    bool equal = (BN_cmp(priv_n, pub_n) == 0);
    BN_free(priv_n);
    BN_free(pub_n);
    return equal;
}

bool Encryptor::loadPublicKeyFromResource() {
    if (PUB_KEY)
        return true;

    QFile keyFile(":/keys/keys/public.pem");
    if (!keyFile.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开资源文件 :/keys/keys/public.pem");
        return false;
    }
    QByteArray keyData = keyFile.readAll();
    keyFile.close();

    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        logOpenSSLError("加载公钥失败（内置资源）");
        return false;
    }
    m_publicKey = pkey;
    m_publicKeyPath.clear(); // 内置无文件路径
    Logger::instance().info("公钥加载成功（内置资源）");
    return true;
}

bool Encryptor::loadPublicKeyWithFallback(const QString &configDir) {
    if (!configDir.isEmpty()) {
        QString pubPath = QDir(configDir).filePath("public.pem");
        if (QFile::exists(pubPath) && loadPublicKeyFromFile(pubPath))
            return true;
    }
    // 回退到内置公钥
    return loadPublicKeyFromResource();
}
// =============== AES 密钥加密/解密 ===============
QByteArray Encryptor::encryptAESKeyWithPrivateKey(const QByteArray &aesKey) {
    if (!PRIV_KEY) {
        Logger::instance().error("私钥未加载，无法加密AES密钥");
        return {};
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(PRIV_KEY, nullptr);
    if (!ctx) {
        logOpenSSLError("创建签名上下文失败");
        return {};
    }

    if (EVP_PKEY_sign_init(ctx) <= 0) {
        logOpenSSLError("签名初始化失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    // 设置填充为 PKCS#1 v1.5 (与原来一致)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        logOpenSSLError("设置RSA填充失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    size_t siglen = 0;
    if (EVP_PKEY_sign(ctx, nullptr, &siglen, (const unsigned char *)aesKey.data(),
                      aesKey.size()) <= 0) {
        logOpenSSLError("获取签名长度失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray signature(siglen, '\0');
    if (EVP_PKEY_sign(ctx, (unsigned char *)signature.data(), &siglen,
                      (const unsigned char *)aesKey.data(), aesKey.size()) <= 0) {
        logOpenSSLError("私钥签名失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    signature.resize(siglen);
    EVP_PKEY_CTX_free(ctx);
    return signature;
}

QByteArray
Encryptor::decryptAESKeyWithPublicKey(const QByteArray &encryptedKey) {
    if (!PUB_KEY) {
        Logger::instance().error("公钥未加载，无法解密AES密钥");
        return {};
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(PUB_KEY, nullptr);
    if (!ctx) {
        logOpenSSLError("创建验证恢复上下文失败");
        return {};
    }

    if (EVP_PKEY_verify_recover_init(ctx) <= 0) {
        logOpenSSLError("验证恢复初始化失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        logOpenSSLError("设置RSA填充失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    size_t outlen = 0;
    if (EVP_PKEY_verify_recover(ctx, nullptr, &outlen,
                                (const unsigned char *)encryptedKey.data(),
                                encryptedKey.size()) <= 0) {
        logOpenSSLError("获取恢复长度失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray aesKey(outlen, '\0');
    if (EVP_PKEY_verify_recover(ctx, (unsigned char *)aesKey.data(), &outlen,
                                (const unsigned char *)encryptedKey.data(),
                                encryptedKey.size()) <= 0) {
        logOpenSSLError("公钥恢复AES密钥失败");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    aesKey.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return aesKey;
}

QByteArray Encryptor::generateAESKey() {
    QByteArray key(32, '\0');
    if (RAND_bytes((unsigned char *)key.data(), 32) != 1) {
        logOpenSSLError("生成AES密钥失败");
        return {};
    }
    return key;
}

// =============== 数据加密/解密 ===============
QByteArray Encryptor::encryptData(const QByteArray &plainData) {
    if (!m_privateKey) {
        Logger::instance().error("加密失败：私钥未加载");
        return {};
    }

    QByteArray aesKey = generateAESKey();
    if (aesKey.isEmpty())
        return {};

    unsigned char nonce[12];
    if (RAND_bytes(nonce, sizeof(nonce)) != 1) {
        logOpenSSLError("生成Nonce失败");
        return {};
    }

    QByteArray encryptedKey = encryptAESKeyWithPrivateKey(aesKey);
    if (encryptedKey.isEmpty())
        return {};

    QByteArray cipherText;
    QByteArray tag(16, '\0');
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Logger::instance().error("创建EVP_CIPHER_CTX失败");
        return {};
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) !=
            1 ||
        EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                                                                                         (const unsigned char *)aesKey.data(), nonce) != 1) {
        logOpenSSLError("AES-GCM初始化失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int outlen = 0;
    cipherText.resize(plainData.size() + EVP_MAX_BLOCK_LENGTH);
    if (EVP_EncryptUpdate(ctx, (unsigned char *)cipherText.data(), &outlen,
                          (const unsigned char *)plainData.data(),
                          plainData.size()) != 1) {
        logOpenSSLError("AES-GCM加密更新失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    cipherText.resize(outlen);

    if (EVP_EncryptFinal_ex(ctx, (unsigned char *)cipherText.data() + outlen,
                            &outlen) != 1) {
        logOpenSSLError("AES-GCM加密最终失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    cipherText.resize(cipherText.size() + outlen);

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16,
                            (unsigned char *)tag.data()) != 1) {
        logOpenSSLError("获取GCM标签失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    EVP_CIPHER_CTX_free(ctx);

    QByteArray result;
    result.append("QCRY");
    result.append(encryptedKey);
    result.append((const char *)nonce, 12);
    result.append(tag);
    result.append(cipherText);
    return result;
}

QByteArray Encryptor::decryptData(const QByteArray &encryptedData) {
    if (!isEncrypted(encryptedData))
        return encryptedData;

    if (!PUB_KEY) {
        Logger::instance().error("解密失败：文件已加密但公钥未加载");
        return {};
    }

    const int keyLen = 256; // RSA-2048 输出固定 256 字节
    const int nonceLen = 12;
    const int tagLen = 16;
    int pos = 4;

    if (encryptedData.size() < pos + keyLen + nonceLen + tagLen) {
        Logger::instance().error("加密数据长度不足");
        return {};
    }

    QByteArray encryptedKey = encryptedData.mid(pos, keyLen);
    pos += keyLen;
    QByteArray nonce = encryptedData.mid(pos, nonceLen);
    pos += nonceLen;
    QByteArray tag = encryptedData.mid(pos, tagLen);
    pos += tagLen;
    QByteArray cipherText = encryptedData.mid(pos);

    QByteArray aesKey = decryptAESKeyWithPublicKey(encryptedKey);
    if (aesKey.isEmpty())
        return {};

    QByteArray plainText;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Logger::instance().error("创建EVP_CIPHER_CTX失败");
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) !=
            1 ||
        EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                                                                                                         (const unsigned char *)aesKey.data(),
                                                                                                         (const unsigned char *)nonce.data()) != 1) {
        logOpenSSLError("AES-GCM解密初始化失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int outlen = 0;
    plainText.resize(cipherText.size());
    if (EVP_DecryptUpdate(ctx, (unsigned char *)plainText.data(), &outlen,
                          (const unsigned char *)cipherText.data(),
                          cipherText.size()) != 1) {
        logOpenSSLError("AES-GCM解密更新失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plainText.resize(outlen);

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tagLen,
                            (void *)tag.data()) != 1) {
        logOpenSSLError("设置GCM标签失败");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)plainText.data() + outlen,
                            &finalLen) != 1) {
        logOpenSSLError("AES-GCM解密最终失败，标签验证错误");
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plainText.resize(plainText.size() + finalLen);
    EVP_CIPHER_CTX_free(ctx);
    return plainText;
}

// =============== 文件级加解密 ===============
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

// =============== 工具函数 ===============
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

// =============== 密钥对生成 ===============
bool Encryptor::generateKeyPair(const QString &privateKeyPath,
                                const QString &publicKeyPath) {
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
    if (!pctx) {
        logOpenSSLError("无法创建RSA上下文");
        return false;
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        logOpenSSLError("密钥生成初始化失败");
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) {
        logOpenSSLError("设置RSA密钥长度失败");
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0 || !pkey) {
        logOpenSSLError("密钥生成失败");
        EVP_PKEY_CTX_free(pctx);
        return false;
    }
    EVP_PKEY_CTX_free(pctx);

    // 保存私钥
    BIO *bio_priv = BIO_new_file(privateKeyPath.toUtf8().constData(), "w");
    if (!bio_priv) {
        Logger::instance().error("无法创建私钥文件: " + privateKeyPath);
        EVP_PKEY_free(pkey);
        return false;
    }
    if (!PEM_write_bio_PrivateKey(bio_priv, pkey, nullptr, nullptr, 0, nullptr,
                                  nullptr)) {
        logOpenSSLError("写入私钥失败");
        BIO_free(bio_priv);
        EVP_PKEY_free(pkey);
        return false;
    }
    BIO_free(bio_priv);

    // 保存公钥
    BIO *bio_pub = BIO_new_file(publicKeyPath.toUtf8().constData(), "w");
    if (!bio_pub) {
        Logger::instance().error("无法创建公钥文件: " + publicKeyPath);
        EVP_PKEY_free(pkey);
        return false;
    }
    if (!PEM_write_bio_PUBKEY(bio_pub, pkey)) {
        logOpenSSLError("写入公钥失败");
        BIO_free(bio_pub);
        EVP_PKEY_free(pkey);
        return false;
    }
    BIO_free(bio_pub);

    EVP_PKEY_free(pkey);
    Logger::instance().info("密钥对生成成功");
    return true;
}

bool Encryptor::convertRecordFile(const QString &filePath,
                                  bool targetEncrypted) {
    bool currentlyEncrypted = isEncryptedFile(filePath);
    if (currentlyEncrypted == targetEncrypted)
        return true;

    QString tempPath = filePath + ".tmp";
    bool ok = false;
    if (targetEncrypted) {
        // 明文 → 加密
        ok = encryptFile(filePath, tempPath);
    } else {
        // 加密 → 明文
        ok = decryptFile(filePath, tempPath);
    }
    if (!ok)
        return false;

    if (!QFile::remove(filePath) || !QFile::rename(tempPath, filePath)) {
        Logger::instance().error("替换文件失败: " + filePath);
        return false;
    }
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
    for (const QFileInfo &fi : std::as_const(files)) {
        QString originalPath = fi.absoluteFilePath();
        QString tempPath = originalPath + ".tmp";
        if (enableEncryption) {
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

// =============== 自动判断加密模式 ===============
bool Encryptor::isEncryptionModeActive(const QString &recordDir) {
    QDir dir(recordDir);
    if (!dir.exists())
        return false;
    QStringList filters{"*.record"};
    dir.setNameFilters(filters);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fi : std::as_const(files)) {
        if (isEncryptedFile(fi.absoluteFilePath()))
            return true;
    }
    return false;
}