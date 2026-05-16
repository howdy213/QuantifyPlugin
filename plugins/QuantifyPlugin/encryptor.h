/**
 * @file encryptor.h
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
#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QString>
#include <QByteArray>

class Encryptor
{
public:
    static void init(const QString& privateKeyPath = QString(),
                     const QString& publicKeyDir = QString());
    static bool loadPublicKey(const QString& filePath);
    static bool keysMatch();
    static QByteArray encryptData(const QByteArray& plainData);
    static QByteArray decryptData(const QByteArray& encryptedData);
    static bool encryptFile(const QString& inputPath, const QString& outputPath);
    static bool decryptFile(const QString& inputPath, const QString& outputPath);
    static bool isEncrypted(const QByteArray& data);
    static bool isEncryptedFile(const QString& filePath);
    static bool generateKeyPair(const QString& privateKeyPath, const QString& publicKeyPath);
    static bool migrateRecordDirectory(const QString& dirPath, bool enableEncryption);
    static bool hasPrivateKey();
    static QString getPrivateKeyPath();
    static QString getPublicKeyPath();

private:
    static bool loadPrivateKey(const QString& path);
    static bool loadPublicKeyFromResource();
    static bool loadPublicKeyFromFile(const QString& filePath);
    static QByteArray encryptAESKeyWithPrivateKey(const QByteArray& aesKey);
    static QByteArray decryptAESKeyWithPublicKey(const QByteArray& encryptedKey);
    static QByteArray generateAESKey();

    static void* m_privateKey;   // EVP_PKEY*
    static void* m_publicKey;    // EVP_PKEY*
    static bool m_initialized;
    static QString m_privateKeyPath;
    static QString m_publicKeyPath;
};

#endif // ENCRYPTOR_H