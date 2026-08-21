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
#include "PluginConfigWidget.h"
#include "WECore/def/wedef.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
using namespace we::Consts;
PluginConfigWidget::PluginConfigWidget(PluginConfigManager *manager,
                                       QWidget *parent)
    : QWidget(parent), manager_(*manager) {
    setupUi();
}

void PluginConfigWidget::setupUi() {
    tree_ = new QTreeWidget(this);
    tree_->setColumnCount(2);
    tree_->setHeaderLabels({QStringLiteral("KeyName"), QStringLiteral("Path")});
    tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree_->setAlternatingRowColors(true);
    tree_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    detail_ = new QPlainTextEdit(this);
    detail_->setReadOnly(true);
    detail_->setPlaceholderText(QStringLiteral("配置文件暂无内容"));

    addDirBtn_ = new QPushButton(QStringLiteral("新建子目录"), this);
    addLeafBtn_ = new QPushButton(QStringLiteral("新建叶子"), this);
    deleteBtn_ = new QPushButton(QStringLiteral("删除节点"), this);
    editPathBtn_ = new QPushButton(QStringLiteral("修改 Path"), this);
    editFileBtn_ = new QPushButton(QStringLiteral("编辑文件"), this);
    refreshBtn_ = new QPushButton(QStringLiteral("刷新"), this);
    cancelEditBtn_ = new QPushButton(QStringLiteral("取消"), this);
    cancelEditBtn_->setVisible(false);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(addDirBtn_);
    btnLayout->addWidget(addLeafBtn_);
    btnLayout->addWidget(deleteBtn_);
    btnLayout->addWidget(editPathBtn_);
    btnLayout->addWidget(editFileBtn_);
    btnLayout->addWidget(refreshBtn_);
    btnLayout->addWidget(cancelEditBtn_);
    btnLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tree_, 3);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(detail_, 2);

    connect(addDirBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onAddDirectory);
    connect(addLeafBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onAddLeaf);
    connect(deleteBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onDeleteNode);
    connect(editPathBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onEditPath);
    connect(editFileBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onEditFile);
    connect(refreshBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onRefresh);
    connect(cancelEditBtn_, &QPushButton::clicked, this,
            &PluginConfigWidget::onCancelEdit);
    connect(tree_, &QTreeWidget::currentItemChanged, this,
            &PluginConfigWidget::onCurrentItemChanged);
}

void PluginConfigWidget::setRootJsonPath(const QString &path) {
    if (manager_.loadFromFile(path)) {
        rebuildTreeFromManager();
    } else {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this,
            QStringLiteral("配置文件错误"),
            QStringLiteral("无法加载根配置文件：%1\n是否重建一个新的空配置文件？").arg(path),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (ret == QMessageBox::Yes) {
            // 重建根配置文件
            QFileInfo fileInfo(path);
            QDir().mkpath(fileInfo.absolutePath());

            QFile file(path);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QJsonObject rootObj;
                rootObj["Plugins"] = QJsonObject();
                file.write(QJsonDocument(rootObj).toJson(QJsonDocument::Indented));
                file.close();

                // 重新加载
                if (manager_.loadFromFile(path)) {
                    rebuildTreeFromManager();
                } else {
                    QMessageBox::warning(this, QStringLiteral("错误"),
                                         QStringLiteral("重建后仍无法加载根配置文件，请检查文件权限或内容。"));
                }
            } else {
                QMessageBox::warning(this, QStringLiteral("错误"),
                                     QStringLiteral("无法创建根配置文件：%1").arg(path));
            }
        }
    }
}

QString PluginConfigWidget::rootJsonPath() const {
    return manager_.rootJsonPath();
}

void PluginConfigWidget::refreshTree() {
    if (manager_.refresh()) {
        rebuildTreeFromManager();
    }
}

QString PluginConfigWidget::filePathForNode(PluginTreeNode *node) const {
    if (!node)
        return QString();
    if (node == manager_.rootNode()) {
        return manager_.rootJsonPath();
    }
    return node->absolutePath;
}

void PluginConfigWidget::setSelectionMode(bool enable) {
    selectionMode_ = enable;
    addDirBtn_->setEnabled(!enable);
    addLeafBtn_->setEnabled(!enable);
    deleteBtn_->setEnabled(!enable);
    editPathBtn_->setEnabled(!enable);
    editFileBtn_->setEnabled(!enable);
    refreshBtn_->setEnabled(!enable);
}

bool PluginConfigWidget::isSelectionMode() const { return selectionMode_; }

PluginTreeNode *PluginConfigWidget::selectedNode() const {
    QTreeWidgetItem *item = tree_->currentItem();
    if (!item)
        return nullptr;
    return nodeFromItem(item);
}

PluginTreeNode *PluginConfigWidget::selectedParentNode() const {
    PluginTreeNode *node = selectedNode();
    if (!node)
        return nullptr;
    if (node == manager_.rootNode())
        return nullptr; // 根项作为父节点
    if (node->isContainer) {
        return node;
    }
    return node->parent == manager_.rootNode() ? nullptr : node->parent;
}

void PluginConfigWidget::rebuildTreeFromManager() {
    tree_->clear();

    // 添加根项，显示根配置文件路径
    auto *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, QStringLiteral("根配置"));
    rootItem->setText(1, manager_.rootJsonPath());
    rootItem->setData(
        0, Qt::UserRole,
        QVariant::fromValue(reinterpret_cast<quintptr>(manager_.rootNode())));
    tree_->addTopLevelItem(rootItem);

    // 将顶层节点作为根项的子节点
    for (PluginTreeNode *node : manager_.topLevelNodes()) {
        createItemForNode(node, rootItem);
    }
    tree_->expandAll();
}

QTreeWidgetItem *
PluginConfigWidget::createItemForNode(PluginTreeNode *node,
                                      QTreeWidgetItem *parentItem) {
    auto *item = new QTreeWidgetItem();
    item->setText(0, node->key);
    item->setText(1, node->path);
    item->setData(0, Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<quintptr>(node)));

    if (parentItem) {
        parentItem->addChild(item);
    } else {
        tree_->addTopLevelItem(item);
    }

    for (PluginTreeNode *child : std::as_const(node->children)) {
        createItemForNode(child, item);
    }
    return item;
}

PluginTreeNode *PluginConfigWidget::nodeFromItem(QTreeWidgetItem *item) const {
    if (!item)
        return nullptr;
    return reinterpret_cast<PluginTreeNode *>(
        item->data(0, Qt::UserRole).value<quintptr>());
}

void PluginConfigWidget::updateDetailForNode(PluginTreeNode *node) {
    detail_->setPlainText(manager_.readFileContent(node));
}

PluginTreeNode *PluginConfigWidget::parentForNewNode() const {
    QTreeWidgetItem *currentItem = tree_->currentItem();
    if (!currentItem)
        return nullptr;
    PluginTreeNode *node = nodeFromItem(currentItem);
    if (!node || node == manager_.rootNode())
        return nullptr; // 根项 -> 添加到根
    if (node->isContainer) {
        return node;
    }
    return (node->parent == manager_.rootNode()) ? nullptr : node->parent;
}

QString PluginConfigWidget::absolutePathForNewNode(PluginTreeNode *parent,
                                                   const QString &path) const {
    if (QDir::isAbsolutePath(path)) {
        return QDir::cleanPath(path);
    }
    QString baseDir;
    if (parent) {
        baseDir = QFileInfo(parent->absolutePath).absolutePath();
    } else {
        baseDir = QFileInfo(manager_.rootJsonPath()).absolutePath();
    }
    return QDir(baseDir).filePath(path);
}

void PluginConfigWidget::enterEditMode(PluginTreeNode *node,
                                       const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this, QStringLiteral("文件不存在"),
            QStringLiteral("文件 %1 不存在，是否创建该文件？").arg(filePath),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {
            QFileInfo fi(filePath);
            QDir().mkpath(fi.absolutePath()); // 确保父目录存在

            QFile createFile(filePath);
            if (createFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                createFile.write("{}"); // 写入空 JSON 对象
                createFile.close();
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QMessageBox::warning(
                        this, QStringLiteral("错误"),
                        QStringLiteral("创建文件后仍无法打开：%1").arg(filePath));
                    return;
                }
            } else {
                QMessageBox::warning(this, QStringLiteral("错误"),
                                     QStringLiteral("无法创建文件：%1").arg(filePath));
                return;
            }
        } else {
            return; // 用户取消
        }
    }

    // 成功打开（或已创建并打开）
    editing_ = true;
    editingNode_ = node;
    editingFilePath_ = filePath;

    detail_->setPlainText(QString::fromUtf8(file.readAll()));
    file.close();

    detail_->setReadOnly(false);
    editFileBtn_->setText(QStringLiteral("保存"));

    addDirBtn_->setEnabled(false);
    addLeafBtn_->setEnabled(false);
    deleteBtn_->setEnabled(false);
    editPathBtn_->setEnabled(false);
    refreshBtn_->setEnabled(false);
    cancelEditBtn_->setVisible(true);
    tree_->setEnabled(false);
}

void PluginConfigWidget::exitEditMode(bool save) {
    detail_->setReadOnly(true);
    detail_->clear();
    editing_ = false;
    editingNode_ = nullptr;
    editingFilePath_.clear();
    editFileBtn_->setText(QStringLiteral("编辑文件"));

    addDirBtn_->setEnabled(true);
    addLeafBtn_->setEnabled(true);
    deleteBtn_->setEnabled(true);
    editPathBtn_->setEnabled(true);
    refreshBtn_->setEnabled(true);
    cancelEditBtn_->setVisible(false);
    tree_->setEnabled(true);

    if (save) {
        refreshTree();
    } else {
        PluginTreeNode *node = selectedNode();
        if (node)
            updateDetailForNode(node);
    }
}

void PluginConfigWidget::onAddDirectory() {
    PluginTreeNode *parent = parentForNewNode();

    bool ok1 = false;
    const QString key = QInputDialog::getText(this, QStringLiteral("新建子目录"),
                                              QStringLiteral("KeyName:"),
                                              QLineEdit::Normal, QString(), &ok1);
    if (!ok1 || key.isEmpty())
        return;

    bool ok2 = false;
    const QString path = QInputDialog::getText(
        this, QStringLiteral("新建子目录"), QStringLiteral("Path (相对或绝对):"),
        QLineEdit::Normal, QString(), &ok2);
    if (!ok2 || path.isEmpty())
        return;

    const QString absPath = absolutePathForNewNode(parent, path);

    if (!QFileInfo::exists(absPath)) {
        QFileInfo fileInfo(absPath);
        QDir().mkpath(fileInfo.absolutePath());

        QFile file(absPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                       QIODevice::Text)) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                                 QStringLiteral("无法创建文件：%1").arg(absPath));
            return;
        }
        QJsonObject containerObj;
        containerObj[Plugins::Plugins] = QJsonObject();
        file.write(QJsonDocument(containerObj).toJson(QJsonDocument::Indented));
        file.close();
    }

    if (manager_.addNode(parent, key, path)) {
        rebuildTreeFromManager();
    } else {
        QMessageBox::warning(
            this, QStringLiteral("错误"),
            QStringLiteral("添加失败，可能 KeyName 已存在或父级配置文件无法写入"));
    }
}

void PluginConfigWidget::onAddLeaf() {
    PluginTreeNode *parent = parentForNewNode();

    bool ok1 = false;
    const QString key = QInputDialog::getText(this, QStringLiteral("新建叶子"),
                                              QStringLiteral("KeyName:"),
                                              QLineEdit::Normal, QString(), &ok1);
    if (!ok1 || key.isEmpty())
        return;

    bool ok2 = false;
    const QString path = QInputDialog::getText(
        this, QStringLiteral("新建叶子"), QStringLiteral("Path (相对或绝对):"),
        QLineEdit::Normal, QString(), &ok2);
    if (!ok2 || path.isEmpty())
        return;

    const QString absPath = absolutePathForNewNode(parent, path);

    if (!QFileInfo::exists(absPath)) {
        QFileInfo fileInfo(absPath);
        QDir().mkpath(fileInfo.absolutePath());

        QFile file(absPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                       QIODevice::Text)) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                                 QStringLiteral("无法创建文件：%1").arg(absPath));
            return;
        }
        QJsonObject pluginObj;
        pluginObj["Path"] = ""; // 占位，后续可编辑
        file.write(QJsonDocument(pluginObj).toJson(QJsonDocument::Indented));
        file.close();
    }

    if (manager_.addNode(parent, key, path)) {
        rebuildTreeFromManager();
    } else {
        QMessageBox::warning(
            this, QStringLiteral("错误"),
            QStringLiteral("添加失败，可能 KeyName 已存在或父级配置文件无法写入"));
    }
}

void PluginConfigWidget::onDeleteNode() {
    PluginTreeNode *node = selectedNode();
    if (!node || node == manager_.rootNode()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选择要删除的节点"));
        return;
    }

    QString msg = node->isContainer
                      ? QStringLiteral("节点“%"
                                       "1”是容器节点，其所有子节点将不再显示（文"
                                       "件本身不会被删除）。\n确定删除吗？")
                            .arg(node->key)
                      : QStringLiteral("确定删除叶子节点“%1”？").arg(node->key);

    if (QMessageBox::question(this, QStringLiteral("确认删除"), msg) !=
        QMessageBox::Yes) {
        return;
    }

    if (manager_.removeNode(node)) {
        rebuildTreeFromManager();
    } else {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("删除失败"));
    }
}

void PluginConfigWidget::onEditPath() {
    PluginTreeNode *node = selectedNode();
    if (!node || node == manager_.rootNode()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选择要修改的节点"));
        return;
    }

    bool ok = false;
    const QString newPath = QInputDialog::getText(
        this, QStringLiteral("修改 Path"), QStringLiteral("Path:"),
        QLineEdit::Normal, node->path, &ok);
    if (!ok || newPath.isEmpty())
        return;

    if (manager_.setNodePath(node, newPath)) {
        rebuildTreeFromManager();
    } else {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("修改失败"));
    }
}

void PluginConfigWidget::onEditFile() {
    if (editing_) {
        // 保存模式
        if (!editingNode_)
            return;
        const QString newContent = detail_->toPlainText();
        QFile file(editingFilePath_);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                       QIODevice::Text)) {
            QMessageBox::warning(
                this, QStringLiteral("错误"),
                QStringLiteral("无法写入文件：%1").arg(editingFilePath_));
            return;
        }
        file.write(newContent.toUtf8());
        file.close();
        exitEditMode(true);
    } else {
        PluginTreeNode *node = selectedNode();
        if (!node) {
            QMessageBox::information(this, "提示", "请先选择要编辑的节点");
            return;
        }
        QString filePath = filePathForNode(node);
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, "错误", "节点没有关联文件路径");
            return;
        }
        enterEditMode(node, filePath);
    }
}

void PluginConfigWidget::onCancelEdit() {
    if (editing_) {
        exitEditMode(false);
    }
}

void PluginConfigWidget::onRefresh() { refreshTree(); }

void PluginConfigWidget::onCurrentItemChanged(QTreeWidgetItem *current,
                                              QTreeWidgetItem *previous) {
    Q_UNUSED(previous)
    if (!current)
        return;
    PluginTreeNode *node = nodeFromItem(current);
    if (node) {
        emit nodeSelected(node);
        if (!selectionMode_ && !editing_) {
            updateDetailForNode(node);
        }
    }
}