/**
 * @file lightsystem.cpp
 * @brief Implementation file for the lightweight plugin loading system.
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
#include "lightsystem.h"
#include "WECore/file/wpath.h"
#include "WECore/plugin/wplugin.h"
#include "WECore/plugin/wpluginmanager.h"
#include "WECore/we/we.h"
#include "WECore/we/webase.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

using namespace we::Consts;
using namespace we;

class LightSystemPrivate {
public:
    // 保留
};

LightSystem::LightSystem() {
    d = new LightSystemPrivate;
    m_configManager = new PluginConfigManager();
}

LightSystem::~LightSystem() {
    delete m_configManager;
    delete d;
}

QJsonObject LightSystem::readJsonFile(QString filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    QString content = file.readAll();
    file.close();
    return QJsonDocument::fromJson(content.toUtf8()).object();
}

void LightSystem::findAllPlugin() {
    QString pluginsPath = WPath().getModuleFolder() + Plugins::ConfigPath;
    if (!m_configManager->loadFromFile(pluginsPath)) {
        return; // 加载失败
    }

    PluginTreeNode* root = m_configManager->rootNode();
    std::function<void(PluginTreeNode*)> loadNode = [&](PluginTreeNode* node) {
        if (!node->isContainer) {
            QFileInfo fileInfo(node->absolutePath);
            QDir curPath = fileInfo.filePath();
            QJsonObject pluginObj = readJsonFile(node->absolutePath);
            if (!pluginObj.isEmpty()) {
                findSinglePlugin(curPath, pluginObj);
            }
        }
        // 递归子节点
        for (PluginTreeNode* child : std::as_const(node->children)) {
            loadNode(child);
        }
    };

    for (PluginTreeNode* child : std::as_const(root->children)) {
        loadNode(child);
    }
}

/**
 * @brief Recursively loads plugins from configuration.
 * @param curPath Path to the directory containing the configuration.
 * @param config JSON object containing the configuration.
 */
void LightSystem::findPluginRecursive(QDir curPath, QJsonObject object) {
    QDir rootPath = curPath;
    QDir filePath = WPath().resolvePath(
        rootPath.path(), object[Plugins::PluginConfigPath].toString());

    QJsonObject jsonObj = readJsonFile(filePath.path());
    if (jsonObj.isEmpty())
        return;

    if (jsonObj.contains(Plugins::Plugins)) {
        QJsonObject pluginsObj = jsonObj[Plugins::Plugins].toObject();
        for (auto it = pluginsObj.begin(); it != pluginsObj.end(); ++it) {
            findPluginRecursive(filePath, it.value().toObject());
        }
    } else {
        QDir rootPath = filePath;
        findSinglePlugin(rootPath, jsonObj);
    }
}

/**
 * @brief Loads a plugin using its JSON configuration.
 * @param curPath Path to the directory containing the configuration.
 * @param config JSON configuration string.
 */
void LightSystem::findPlugin(QDir curPath, QJsonObject object) {
    QDir pluginPath = WPath().resolvePath(
        curPath.path(), object[Plugins::PluginConfigPath].toString());
    QJsonObject pluginObj = readJsonFile(pluginPath.path());
    if (pluginObj.isEmpty())
        return;
    findSinglePlugin(curPath, pluginObj);
}

PluginConfigManager *LightSystem::pluginConfigManager()
{
    return m_configManager;
}

/**
 * @brief Loads a single plugin instance.
 * @param curPath Path to the directory containing the plugin configuration.
 * @param pluginObj JSON object containing the plugin configuration.
 */
void LightSystem::findSinglePlugin(QDir curPath, QJsonObject pluginObj) {
    if (!pluginObj.contains(Plugin::RelativePath))
        return;

    WPlugin *plugin = new WPlugin(WE::inst()->getWEClass()->pluginManager());
    plugin->readConfig(curPath.path(),pluginObj);
    WE::inst()->getWEClass()->pluginManager()->addPlugin(plugin);
}