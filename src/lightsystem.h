/**
 * @file lightsystem.h
 * @brief Header file for the lightweight plugin loading system.
 * @author howdy213
 * @date 2026-08-20
 * @version 2.0.0
 *
 * @copyright Copyright 2025-2026 howdy213
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LIGHTSYSTEM_H
#define LIGHTSYSTEM_H

#include "WECore/def/wedef.h"
#include "PluginConfigManager.h"

#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QDir>
class LightSystemPrivate;

namespace we {
class LightSystem {
public:
    LightSystem();
    virtual ~LightSystem();

    void findAllPlugin();
    void findSinglePlugin(QDir curPath, QJsonObject pluginObj);
    void findPluginRecursive(QDir rootPath, QJsonObject object);
    void findPlugin(QDir curPath, QJsonObject object);
    PluginConfigManager*  pluginConfigManager();
private:
    static QJsonObject readJsonFile(QString filePath);
    LightSystemPrivate *d = nullptr;
    PluginConfigManager* m_configManager;
};
} // namespace we

#endif // LIGHTSYSTEM_H