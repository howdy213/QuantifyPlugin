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
#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <functional>

// 插件配置树节点
struct PluginTreeNode {
    QString key;                  // KeyName
    QString path;                 // 相对路径
    QString absolutePath;         // 绝对路径
    bool isContainer = false;     // 是否为容器节点
    PluginTreeNode* parent = nullptr;
    QList<PluginTreeNode*> children;
};

class PluginConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginConfigManager(QObject* parent = nullptr);
    ~PluginConfigManager();

    // 从根 JSON 文件加载整个配置树
    bool loadFromFile(const QString& rootJsonPath);
    // 重新加载
    bool refresh();

    // 根节点（虚拟根，其 children 为顶层节点）
    PluginTreeNode* rootNode() const { return rootNode_; }
    // 顶层节点列表
    QList<PluginTreeNode*> topLevelNodes() const { return rootNode_->children; }

    // 添加节点：parent 为 nullptr 表示添加到根，否则添加到 parent 的子节点
    bool addNode(PluginTreeNode* parent, const QString& key, const QString& path);
    // 删除节点（仅从父级 JSON 中移除，不删除实际文件）
    bool removeNode(PluginTreeNode* node);
    // 修改节点的 Path
    bool setNodePath(PluginTreeNode* node, const QString& newPath);

    // 读取节点指向的文件内容（用于叶子节点展示）
    QString readFileContent(PluginTreeNode* node) const;

    // 深度优先遍历所有节点
    void traverse(const std::function<void(PluginTreeNode*)>& func) const;

    // 获取根 JSON 文件路径
    QString rootJsonPath() const { return rootJsonPath_; }

private:
    void clearTree();
    void buildNodeRecursive(const QString& key, const QString& path,
                            PluginTreeNode* parent, const QString& baseDir);
    QJsonObject readJsonFromFile(const QString& filePath, bool* ok = nullptr) const;
    bool writeJsonToFile(const QString& filePath, const QJsonObject& obj) const;
    QString resolvePath(const QString& baseDir, const QString& path) const;

    PluginTreeNode* rootNode_ = nullptr;
    QString rootJsonPath_;
};