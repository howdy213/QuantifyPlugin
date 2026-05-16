/**
 * @file quantify.cpp
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
#include "quantify.h"
#include "WECore/metadata/WMetaDocument.h"
#include <QDir>
using namespace we;
using namespace we::Consts;
using namespace Quantify::Consts;
///
/// \brief Quantify::resolvePath
/// \param doc
/// \param relativePath
/// \return
///
QString Quantify::resolvePath(WMetaDocument* doc, const QString& relativePath) {
    if (!doc) return relativePath;
    QString configDir = doc->get(DirRoot).toString();
    if (configDir.isEmpty()) return relativePath;
    QDir baseDir(configDir);
    return baseDir.filePath(relativePath);
}
///
/// \brief Quantify::resolvePathWithKey
/// \param doc
/// \param key
/// \return
///
QString Quantify::resolvePathWithKey(WMetaDocument *doc, const QString &key)
{
    return resolvePath(doc,doc->get(key).toString());
}

QString Quantify::getConfigDir(WMetaDocument *doc)
{
    return doc->get(DirRoot).toString();
}
