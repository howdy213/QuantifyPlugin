/**
 * @file classrecord.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-4-5
 * @version 1.4.0
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
#include <QCloseEvent>
#include <QDir>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "WECore/WConfig/wconfigdocument.h"
#include "WECore/WDef/wedef.h"
#include "WECore/WFile/wpath.h"
#include "WECore/WPlugin/wplugin.h"
#include "WECore/WPlugin/wplugindata.h"

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
using namespace we::Consts;
using namespace Quantify;
using namespace Quantify::Consts;

class QuantifyDialogPrivate {
public:
    QuantifyDialogPrivate() = default;

    Ui::QuantifyDialog *ui = nullptr;
    QuantifyPlugin *plugin = nullptr;
    Quantify::QuantifyComponents *m_components = nullptr;
    Quantify::QuantifyUI *m_ui = nullptr;
};
///
/// \brief QuantifyDialog::QuantifyDialog
/// \param parent
///
QuantifyDialog::QuantifyDialog(QWidget *parent)
    : QWidget(parent), d_ptr(new QuantifyDialogPrivate) {
    Q_D(QuantifyDialog);
    d->ui = new Ui::QuantifyDialog;
    d->ui->setupUi(this);

    Logger::instance().setLogPath(
        QDir(PPlugin->getMetaData(Plugin::Path).toString())
            .filePath("../Quantify/logs"));
    Logger::instance().clear();
    d->m_components = new Quantify::QuantifyComponents;
    d->m_ui = new Quantify::QuantifyUI;
    d->m_components->config = new WConfigDocument;
    readConfig();
    auto &config = d->m_components->config;
    d->m_components->classRecord = new ClassRecord(
        resolvePathWithKey(config, DirPath),
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
}
///
/// \brief QuantifyDialog::~QuantifyDialog
///
QuantifyDialog::~QuantifyDialog() {
    Q_D(QuantifyDialog);
    delete d->ui;
}
///
/// \brief QuantifyDialog::readConfig
/// \return
///
bool QuantifyDialog::readConfig() {
    Q_D(QuantifyDialog);
    d_func();
    if (!d->m_components->config)
        d->m_components->config = new WConfigDocument;

    QString configPath =
        WPath(PData).getModuleFolder(PPlugin->getId()) + "Quantify/config.json";
    if (!d->m_components->config->load(configPath, true)) {
        QMessageBox::information(this, "首次使用",
                                 "前往 设置>新建配置-新建 创建初始数据后开始使用");
        return false;
    };

    QFileInfo configFileInfo(configPath);
    QString configDir = configFileInfo.absolutePath();
    d->m_components->config->set(DirRoot, configDir);

    QString path = d->m_components->config->get(DirPath).toString();
    QString addon = d->m_components->config->get(DirAddon).toString();
    QString temp = d->m_components->config->get(DirTemplate).toString();

    auto findPrivateKey = []() -> QString {
        QFileInfoList drives = QDir::drives();
        for (const QFileInfo &drive : std::as_const(drives)) {
            QString keyPath = drive.absolutePath() + "Quantify.pem";
            if (QFile::exists(keyPath))
                return keyPath;
        }
        return QString();
    };
    QString privateKeyPath = findPrivateKey();
    Encryptor::init(privateKeyPath);
    if (!privateKeyPath.isEmpty()) {
        if (!Encryptor::keysMatch()) {
            Encryptor::init("");
            QString errMsg = "公私钥不匹配：" + privateKeyPath;
            Logger::instance().error(errMsg);
            QMessageBox::critical(this, "错误", errMsg);
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
///
/// \brief QuantifyDialog::setPlugin
/// \param plugin
///
void QuantifyDialog::setPlugin(QuantifyPlugin *plugin) {
    Q_D(QuantifyDialog);
    d->plugin = plugin;
}
///
/// \brief QuantifyDialog::closeEvent
/// \param event
///
void QuantifyDialog::closeEvent(QCloseEvent *event) {
    Q_D(QuantifyDialog);
    if (d->plugin)
        d->plugin->setWidget(nullptr);
    event->accept();
    deleteLater();
}