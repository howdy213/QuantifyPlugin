/**
 * @file quantifydialog.cpp
 * @brief 主对话框
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
#include <QCloseEvent>
#include <QDir>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <mutex>

#include "WECore/def/wedef.h"
#include "WECore/file/wpath.h"
#include "WECore/metadata/WMetaDocument.h"
#include "WECore/plugin/wplugin.h"
#include "WECore/plugin/wplugindata.h"

#include "encryptor.h"
#include "logger.h"
#include "quantify.h"
#include "quantifydisplaywindow.h"
#include "quantifyeditwindow.h"
#include "quantifyhelpdialog.h"
#include "quantifyplugin.h"
#include "quantifysettingwindow.h"

#include "quantifydialog.h"
#include "ui_quantifydialog.h"
using namespace we;
using namespace we::Consts;
using namespace Quantify;
using namespace Quantify::Consts;
static std::once_flag opensslInitFlag;

class QuantifyDialogPrivate {
public:
    QuantifyDialogPrivate() = default;

    Ui::QuantifyDialog *ui = nullptr;
    QuantifyPlugin *plugin = nullptr;
    Quantify::QuantifyComponents *m_components = nullptr;
    Quantify::QuantifyUI *m_ui = nullptr;
};

QuantifyDialog::QuantifyDialog(QWidget *parent)
    : QWidget(parent), d_ptr(new QuantifyDialogPrivate) {
    Q_D(QuantifyDialog);
    d->ui = new Ui::QuantifyDialog;
    d->ui->setupUi(this);

    QDir pluginDir = PPlugin->path();
    pluginDir.cdUp();
    Logger::instance().setLogPath(pluginDir.filePath("Quantify/logs"));
    Logger::instance().clear();
    d->m_components = new QuantifyComponents;
    d->m_ui = new QuantifyUI;
    d->m_components->config = new WMetaDocument;
    readConfig();
    auto &config = d->m_components->config;
    d->m_components->classRecord = new ClassRecord(
        getTermDir(config).absolutePath(),
        config->get(VarEngine).toString() == EngineJS ? RuleEngine::JS
                                                      : RuleEngine::Native);

    d->m_ui->displayWindow = new QuantifyDisplayWindow(this);
    d->m_ui->editWindow = new QuantifyEditWindow(this);
    d->m_ui->settingWindow = new QuantifySettingWindow(this);
    d->m_ui->helpDialog = new QuantifyHelpDialog(d->m_components->config, this);

    d->m_ui->displayWindow->initialize(*d->m_components, *d->m_ui);
    d->m_ui->editWindow->initialize(*d->m_components, *d->m_ui);
    d->m_ui->settingWindow->initialize(*d->m_components, *d->m_ui);

    d->ui->stackedWidget->addWidget(d->m_ui->displayWindow);
    d->ui->stackedWidget->addWidget(d->m_ui->editWindow);
    d->ui->stackedWidget->addWidget(d->m_ui->settingWindow);
    d->ui->stackedWidget->addWidget(d->m_ui->helpDialog);

    d->ui->sideBar->setBtnCount(4);
    d->ui->sideBar->connectStack(d->ui->stackedWidget);
    d->ui->sideBar->setButtonContent(0, "查看");
    d->ui->sideBar->setButtonContent(1, "编辑");
    d->ui->sideBar->setButtonContent(2, "设置");
    d->ui->sideBar->setButtonContent(3, "帮助");

    d->ui->stackedWidget->setCurrentIndex(0);

    setMaximumSize(1300, 650);
    setMinimumSize(1000, 500);

    connect(d->m_ui->settingWindow, &QuantifySettingWindow::requestDialogRestart,
            this, &QuantifyDialog::onRequestDialogRestart);
    connect(d->m_ui->settingWindow, &QuantifySettingWindow::settingsChanged, this,
            &QuantifyDialog::onSettingsChanged);
    connect(d->m_ui->settingWindow, &QuantifySettingWindow::unsavedChangesChanged,
            this, &QuantifyDialog::onSettingUnsavedChanged);
    d->ui->sideBar->setEnabled(true);
}

QuantifyDialog::~QuantifyDialog() {
    Q_D(QuantifyDialog);
    delete d->ui;
}

bool QuantifyDialog::readConfig() {
    Q_D(QuantifyDialog);
    d_func();
    if (!d->m_components->config)
        d->m_components->config = new WMetaDocument;

    QDir pluginDir = PPlugin->path();
    pluginDir.cdUp();
    QDir configPath = pluginDir.filePath("Quantify/config.json");
    if (!d->m_components->config->load(configPath.absolutePath(), true)) {
        QMessageBox::information(this, "首次使用",
                                 "前往 设置>新建配置-新建 创建初始数据后开始使用");
    };

    QFileInfo configFileInfo(configPath.absolutePath());
    QDir configDir = configFileInfo.dir();

    setConfigDir(d->m_components->config, configDir.absolutePath());

    QString path = getTermDir(d->m_components->config).path();
    QString addon = resolvePathWithKey(d->m_components->config, DirAddon).path();
    QString temp = resolvePathWithKey(d->m_components->config, DirTemplate).path();

    auto findPrivateKey = []() -> QString {
        QFileInfoList drives = QDir::drives();
        for (const QFileInfo &drive : std::as_const(drives)) {
            QString keyPath = drive.absolutePath() + "Quantify.pem";
            if (QFile::exists(keyPath))
                return keyPath;
        }
        return QString();
    };

    std::call_once(opensslInitFlag, []() { Encryptor::init(); });

    // 加载公钥（优先配置目录，否则内置）
    if (!Encryptor::loadPublicKeyWithFallback(configDir.absolutePath())) {
        Logger::instance().warn("公钥加载失败，解密功能将不可用");
    }

    // 查找私钥
    QString privateKeyPath = findPrivateKey();
    if (!privateKeyPath.isEmpty()) {
        if (!Encryptor::loadPrivateKey(privateKeyPath)) {
            Logger::instance().error("私钥加载失败: " + privateKeyPath);
        } else if (!Encryptor::keysMatch()) {
            Encryptor::clearPrivateKey(); // 清除不匹配的密钥
            m_keyMismatchError = "公私钥不匹配：" + privateKeyPath;
            Logger::instance().error(m_keyMismatchError);
        }
    }

    if (path.isEmpty()) {
        QMessageBox::information(this, "提示", "无路径！将使用默认路径");
        path = "./term1";
        d->m_components->config->set(DirPath, path);
    }
    if (addon.isEmpty()) {
        addon = "./addon";
        d->m_components->config->set(DirAddon, addon);
    }
    if (temp.isEmpty()) {
        temp = "./template";
        d->m_components->config->set(DirTemplate, temp);
    }
    if (d->m_components->config->get(VarEngine).toString() != EngineJS)
        d->m_components->config->set(VarEngine, EngineNative);

    return true;
}

void QuantifyDialog::setPlugin(QuantifyPlugin *plugin) {
    Q_D(QuantifyDialog);
    d->plugin = plugin;
}

void QuantifyDialog::onSettingsChanged() { rebuildClassRecord(); }

void QuantifyDialog::onSettingUnsavedChanged(bool hasUnsaved) {
    Q_D(QuantifyDialog);
    if (d->ui && d->ui->sideBar) {
        d->ui->sideBar->setEnabled(!hasUnsaved);
    }
}

void QuantifyDialog::rebuildClassRecord() {
    Q_D(QuantifyDialog);
    // 1. 删除旧的 ClassRecord
    delete d->m_components->classRecord;
    QString dataPath = getTermDir(d->m_components->config).absolutePath();
    RuleEngine engine =
        (d->m_components->config->get(VarEngine).toString() == EngineJS)
                            ? RuleEngine::JS
                            : RuleEngine::Native;
    // 3. 创建新的 ClassRecord
    d->m_components->classRecord = new ClassRecord(dataPath, engine);
    // 4. 通知所有依赖的子窗口更新指针
    d->m_ui->displayWindow->setClassRecord(d->m_components->classRecord);
    d->m_ui->editWindow->setClassRecord(d->m_components->classRecord);
    // 5. 强制刷新显示窗口（重新加载并绘制表格）
    d->m_ui->displayWindow->refresh();
}

void QuantifyDialog::onRequestDialogRestart() {
    // 获取插件指针
    Q_D(QuantifyDialog);
    if (d->plugin) {
        // 先隐藏并删除当前对话框
        this->hide();
        // 调用插件的重启方法
        d->plugin->restartDialog();
        // 延迟删除当前对话框，避免在插件的 restartDialog 中立即销毁导致问题
        this->deleteLater();
    } else {
        // 没有插件指针，简单关闭
        close();
    }
}

void QuantifyDialog::closeEvent(QCloseEvent *event) {
    Q_D(QuantifyDialog);
    if (d->plugin)
        d->plugin->setWidget(nullptr);
    event->accept();
    deleteLater();
}

void QuantifyDialog::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    // 延迟显示密钥不匹配错误（确保窗口已经完全显示，避免构造时弹窗崩溃）
    if (!m_keyMismatchError.isEmpty()) {
        QMessageBox::critical(this, "错误", m_keyMismatchError);
        m_keyMismatchError.clear(); // 只显示一次
    }
}