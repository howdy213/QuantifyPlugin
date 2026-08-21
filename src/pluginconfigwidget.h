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

#include <QWidget>
#include "PluginConfigManager.h"

class QTreeWidget;
class QTreeWidgetItem;
class QPlainTextEdit;
class QPushButton;

class PluginConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PluginConfigWidget(PluginConfigManager* manager, QWidget* parent = nullptr);

    void setRootJsonPath(const QString& path);
    QString rootJsonPath() const;
    void refreshTree();
    QString filePathForNode(PluginTreeNode* node) const;
    void setSelectionMode(bool enable);
    bool isSelectionMode() const;

    PluginTreeNode* selectedNode() const;
    PluginTreeNode* selectedParentNode() const;

signals:
    void nodeSelected(PluginTreeNode* node);
    void nodeDoubleClicked(PluginTreeNode* node);
    void parentNodeChosen(PluginTreeNode* node);

private slots:
    void onAddDirectory();
    void onAddLeaf();
    void onDeleteNode();
    void onEditPath();
    void onEditFile();
    void onCancelEdit();
    void onRefresh();
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void setupUi();
    void rebuildTreeFromManager();
    QTreeWidgetItem* createItemForNode(PluginTreeNode* node, QTreeWidgetItem* parentItem);
    PluginTreeNode* nodeFromItem(QTreeWidgetItem* item) const;
    void updateDetailForNode(PluginTreeNode* node);

    PluginTreeNode* parentForNewNode() const;
    QString absolutePathForNewNode(PluginTreeNode* parent, const QString& path) const;
    void enterEditMode(PluginTreeNode *node, const QString &filePath);
    void exitEditMode(bool save);

    PluginConfigManager& manager_;
    QTreeWidget* tree_ = nullptr;
    QPlainTextEdit* detail_ = nullptr;
    QPushButton* addDirBtn_ = nullptr;
    QPushButton* addLeafBtn_ = nullptr;
    QPushButton* deleteBtn_ = nullptr;
    QPushButton* editPathBtn_ = nullptr;
    QPushButton* editFileBtn_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    QPushButton* cancelEditBtn_ = nullptr;
    bool selectionMode_ = false;

    // 编辑状态
    bool editing_ = false;
    QString editingFilePath_;
    PluginTreeNode* editingNode_ = nullptr;
};