/**
 * @file quantify.h
 * @brief 辅助函数
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
#ifndef QUANTIFY_H
#define QUANTIFY_H
#include "WECore/metadata/WMetaDocument.h"

#include <QDir>

class ClassRecord;
class Encryptor;
class Logger;

class QuantifyDialog;
class QuantifyDisplayWindow;
class QuantifyEditWindow;
class QuantifySettingWindow;
class QuantifyHelpDialog;
class QuantifyDisplayViewDialog;

namespace Quantify {
namespace Consts {
const QString DirRecord = "record";         // Quantify/record/
const QString DirRule = "rule";             // Quantify/rule/
const QString DirGroup = "group";           // Quantify/group/
const QString DirAddon = "addon";           // Quantify/addon/
const QString DirTemplate = "template";     // Quantify/template/
const QString DirPath = "path";             // Quantify/<term>/
const QString DirRoot = "_configDir";       // Quantify/
const QString VarEngine = "engine";         // js/native
const QString VarEncryption = "encryption"; // bool
const QString EngineNative = "native";
const QString EngineJS = "js";
}; // namespace Consts

class QuantifyComponents {
public:
    we::WMetaDocument *config = nullptr;
    ClassRecord *classRecord = nullptr;
    Encryptor *encryptor = nullptr;
    Logger *logger = nullptr;
};

class QuantifyUI {
public:
    QuantifyDialog *mainDialog = nullptr;
    QuantifyDisplayWindow *displayWindow = nullptr;
    QuantifyEditWindow *editWindow = nullptr;
    QuantifySettingWindow *settingWindow = nullptr;
    QuantifyHelpDialog *helpDialog = nullptr;
};

QString resolvePath(we::WMetaDocument *doc, const QString &relativePath);
QDir getConfigDir(we::WMetaDocument *doc);
bool setConfigDir(we::WMetaDocument *doc, QString path);
QDir getTermDir(we::WMetaDocument *doc);
QDir resolvePathWithKey(we::WMetaDocument *doc, const QString &key);

} // namespace Quantify

#endif // QUANTIFY_H