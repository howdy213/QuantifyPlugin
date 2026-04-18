/**
 * @file lightsystem.cpp
 * @brief 轻量插件加载系统实现文件
 * @author howdy213
 * @date 2026-1-30
 * @version 1.1.0
 *
 * Copyright 2025-2026 howdy213
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
#include "lightsystem.h"
#include "WECore/WConfig/wconfigdocument.h"
#include "WECore/WE/we.h"
#include "WECore/WE/webase.h"
#include "WECore/WFile/wpath.h"
#include "WECore/WPlugin/wplugin.h"
#include "WECore/WPlugin/wpluginmanager.h"

#include <QCoreApplication>
#include <QDir>
using namespace we::Consts;

class LightSystemPrivate {
public:
};

///
/// \brief LightSystem::LightSystem
///
LightSystem::LightSystem() { d = new LightSystemPrivate; }
///
/// \brief LightSystem::~LightSystem
///
LightSystem::~LightSystem() { delete d; }
///
/// \brief LightSystem::loadAllPlugin
///
void LightSystem::loadAllPlugin() {
    QString plugins = WPath().getModuleFolder() + Plugins::ConfigPath;
    WConfigDocument doc;
    doc.load(plugins, true);
    auto map = doc.toMap();
    for (auto it = map.begin(); it != map.end(); it++) {
        QJsonDocument doc2(it.value().toJsonObject());
        loadPlugin(plugins, doc2.toJson());
    }
}
///
/// \brief LightSystem::loadPlugin
/// \param curPath
/// \param config
///
void LightSystem::loadPlugin(QString curPath, QString config) {
    WConfigDocument doc;
    doc.load(config, false);
    QString jsonPath = WPath().transPath(
        curPath, qvariant_cast<QString>(doc.get(Plugins::PluginConfigPath)));

    WConfigDocument objectDoc;
    objectDoc.load(jsonPath, true);

    for (int i = 1;; i++) {
        QString num = QString::number(i);

        auto obj = objectDoc.get(num).toJsonObject();
        if (obj.isEmpty())
            break;
        QJsonDocument doc2(obj);
        WConfigDocument doc3;
        doc3.load(doc2.toJson(), false);

        loadSinglePlugin(jsonPath, &doc3);
    }
}
///
/// \brief LightSystem::loadSinglePlugin
/// \param jsonPath
/// \param doc
///
void LightSystem::loadSinglePlugin(QString jsonPath, WConfigDocument *doc) {
    if (!doc->hasArg(Plugin::Path))
        return;
    WPlugin *plugin = new WPlugin(WE::inst()->getWEClass()->pluginManager());
    plugin->readConfig(jsonPath, doc->toJsonString());
    WE::inst()->getWEClass()->pluginManager()->addPlugin(plugin);
}
