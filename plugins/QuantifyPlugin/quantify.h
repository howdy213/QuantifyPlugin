/**
 * @file classrecord.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-3-1
 * @version 1.3.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef QUANTIFY_H
#define QUANTIFY_H
#include "WConfig/wconfigdocument.h"

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
const QString DirRecord = "record";
const QString DirRule = "rule";
const QString DirGroup = "group";
const QString DirAddon = "addon";
const QString DirTemplate = "template";
const QString DirPath = "path";
const QString DirRoot = "_configDir";
const QString VarEngine = "engine";
const QString VarEncryption = "encryption";
const QString EngineNative = "native";
const QString EngineJS = "js";
}; // namespace Consts

class QuantifyComponents {
public:
    WConfigDocument *config = nullptr;
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

QString resolvePath(WConfigDocument *doc, const QString &relativePath);
QString getConfigDir(WConfigDocument *doc);
QString resolvePathWithKey(WConfigDocument *doc, const QString &key);

} // namespace Quantify

#endif // QUANTIFY_H