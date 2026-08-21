/**
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
#include "PluginConfigManager.h"

#include "WECore/file/wpath.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
using namespace we;
using namespace Consts;
PluginConfigManager::PluginConfigManager(QObject *parent) : QObject(parent) {
    rootNode_ = new PluginTreeNode;
    rootNode_->isContainer = true;
}

PluginConfigManager::~PluginConfigManager() {
    clearTree();
    delete rootNode_;
}

void PluginConfigManager::clearTree() {
    std::function<void(PluginTreeNode *)> deleteChildren =
        [&](PluginTreeNode *node) {
        for (PluginTreeNode *child : std::as_const(node->children)) {
                deleteChildren(child);
            delete child;
        }
        node->children.clear();
        };
    deleteChildren(rootNode_);
}

bool PluginConfigManager::loadFromFile(const QString &rootJsonPath) {
    clearTree();
    rootJsonPath_ = rootJsonPath;
    rootNode_->absolutePath = rootJsonPath_;

    QJsonObject rootObj;
    bool ok = false;
    rootObj = readJsonFromFile(rootJsonPath_, &ok);
    if (!ok)
        return false;

    const QJsonValue pluginsVal = rootObj.value(Plugins::Plugins);
    if (!pluginsVal.isObject())
        return false;

    const QJsonObject plugins = pluginsVal.toObject();
    const QString baseDir = QFileInfo(rootJsonPath_).absolutePath();

    for (const QString &key : plugins.keys()) {
        const QJsonObject val = plugins.value(key).toObject();
        const QString path = val.value(Plugins::PluginConfigPath).toString();
        buildNodeRecursive(key, path, rootNode_, baseDir);
    }
    return true;
}

bool PluginConfigManager::refresh() {
    if (rootJsonPath_.isEmpty())
        return false;
    return loadFromFile(rootJsonPath_);
}

void PluginConfigManager::buildNodeRecursive(const QString &key,
                                             const QString &path,
                                             PluginTreeNode *parent,
                                             const QString &baseDir) {
    auto *node = new PluginTreeNode;
    node->key = key;
    node->path = path;
    node->absolutePath = resolvePath(baseDir, path);
    node->parent = parent;
    parent->children.append(node);

    QJsonObject childObj;
    bool ok = false;
    childObj = readJsonFromFile(node->absolutePath, &ok);
    if (ok && childObj.value(Plugins::Plugins).isObject()) {
        node->isContainer = true;
        const QJsonObject childPlugins =
            childObj.value(Plugins::Plugins).toObject();
        const QString childBaseDir = QFileInfo(node->absolutePath).absolutePath();
        for (const QString &childKey : childPlugins.keys()) {
            const QJsonObject childVal = childPlugins.value(childKey).toObject();
            const QString childPath =
                childVal.value(Plugins::PluginConfigPath).toString();
            buildNodeRecursive(childKey, childPath, node, childBaseDir);
        }
    } else {
        node->isContainer = false;
    }
}

QString PluginConfigManager::resolvePath(const QString &baseDir,
                                         const QString &path) const {
    WPath wpath;
    return wpath.resolvePath(baseDir, path);
}

QJsonObject PluginConfigManager::readJsonFromFile(const QString &filePath,
                                                  bool *ok) const {
    if (ok)
        *ok = false;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return QJsonObject();
    }
    if (ok)
        *ok = true;
    return doc.object();
}

bool PluginConfigManager::writeJsonToFile(const QString &filePath,
                                          const QJsonObject &obj) const {
    const QFileInfo fi(filePath);
    QDir().mkpath(fi.absolutePath());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}

bool PluginConfigManager::addNode(PluginTreeNode *parent, const QString &key,
                                  const QString &path) {
    if (key.isEmpty() || path.isEmpty())
        return false;

    QString parentJsonPath;
    if (parent == nullptr || parent == rootNode_) {
        parentJsonPath = rootJsonPath_;
    } else {
        parentJsonPath = parent->absolutePath;
    }

    QJsonObject parentObj;
    bool ok = false;
    parentObj = readJsonFromFile(parentJsonPath, &ok);
    if (!ok)
        return false;

    QJsonObject plugins = parentObj.value(Plugins::Plugins).toObject();
    if (plugins.contains(key))
        return false; // 同级 Key 已存在

    QJsonObject newObj;
    newObj[Plugins::PluginConfigPath] = path;
    plugins.insert(key, newObj);
    parentObj[Plugins::Plugins] = plugins;

    if (!writeJsonToFile(parentJsonPath, parentObj))
        return false;
    return refresh();
}

bool PluginConfigManager::removeNode(PluginTreeNode *node) {
    if (!node || node == rootNode_)
        return false;

    QString parentJsonPath;
    if (node->parent == rootNode_ || node->parent == nullptr) {
        parentJsonPath = rootJsonPath_;
    } else {
        parentJsonPath = node->parent->absolutePath;
    }

    QJsonObject parentObj;
    bool ok = false;
    parentObj = readJsonFromFile(parentJsonPath, &ok);
    if (!ok)
        return false;

    QJsonObject plugins = parentObj.value(Plugins::Plugins).toObject();
    if (!plugins.contains(node->key))
        return false;
    plugins.remove(node->key);
    parentObj[Plugins::Plugins] = plugins;

    if (!writeJsonToFile(parentJsonPath, parentObj))
        return false;
    return refresh();
}

bool PluginConfigManager::setNodePath(PluginTreeNode *node,
                                      const QString &newPath) {
    if (!node || node == rootNode_ || newPath.isEmpty())
        return false;

    QString parentJsonPath;
    if (node->parent == rootNode_ || node->parent == nullptr) {
        parentJsonPath = rootJsonPath_;
    } else {
        parentJsonPath = node->parent->absolutePath;
    }

    QJsonObject parentObj;
    bool ok = false;
    parentObj = readJsonFromFile(parentJsonPath, &ok);
    if (!ok)
        return false;

    QJsonObject plugins = parentObj.value(Plugins::Plugins).toObject();
    QJsonObject target = plugins.value(node->key).toObject();
    if (target.isEmpty())
        return false;
    target[Plugins::PluginConfigPath] = newPath;
    plugins[node->key] = target;
    parentObj[Plugins::Plugins] = plugins;

    if (!writeJsonToFile(parentJsonPath, parentObj))
        return false;
    return refresh();
}

QString PluginConfigManager::readFileContent(PluginTreeNode *node) const {
    if (!node)
        return QString();
    QFile file(node->absolutePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(file.readAll());
}

void PluginConfigManager::traverse(
    const std::function<void(PluginTreeNode *)> &func) const {
    std::function<void(PluginTreeNode *)> dfs = [&](PluginTreeNode *node) {
        func(node);
        for (PluginTreeNode *child : std::as_const(node->children)) {
            dfs(child);
        }
    };
    for (PluginTreeNode *child : std::as_const(rootNode_->children)) {
        dfs(child);
    }
}