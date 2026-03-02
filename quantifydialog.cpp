/**
 * @file classrecord.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-3-1
 * @version 1.3.0
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "WECore/WConfig/wconfigdocument.h"
#include "WECore/WDef/wedef.h"
#include "WECore/WFile/wpath.h"
#include "WECore/WPlugin/wplugin.h"
#include "WECore/WPlugin/wplugindata.h"

#include "quantifydisplaywindow.h"
#include "quantifyeditwindow.h"
#include "quantifyhelpdialog.h"
#include "quantifyplugin.h"
#include "quantifysettingwindow.h"

#include "quantifydialog.h"
#include "ui_quantifydialog.h"
using namespace we::Consts;

class QuantifyDialogPrivate {
public:
    QuantifyDialogPrivate() = default;

    Ui::QuantifyDialog *ui = nullptr;
    QuantifyPlugin *plugin = nullptr;
    WConfigDocument *doc = nullptr;
    QuantifyDisplayWindow *displayWnd = nullptr;
    QuantifyEditWindow *editWnd = nullptr;
    QuantifySettingWindow *settingWnd = nullptr;
    QuantifyHelpDialog *helpWnd = nullptr;
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

    readConfig();

    QString path = qvariant_cast<QString>(d->doc->get("path"));
    d->displayWnd = new QuantifyDisplayWindow(d->doc, this);
    d->editWnd = new QuantifyEditWindow(this);
    d->settingWnd = new QuantifySettingWindow(this);
    d->helpWnd = new QuantifyHelpDialog(d->doc, this);

    d->settingWnd->setDoc(d->doc);
    d->editWnd->doc = d->doc;

    d->ui->stackedWidget->addWidget(d->displayWnd);
    d->ui->stackedWidget->addWidget(d->editWnd);
    d->ui->stackedWidget->addWidget(d->settingWnd);
    d->ui->stackedWidget->addWidget(d->helpWnd);

    d->ui->sideBar->setBtnCount(4);
    d->ui->sideBar->connectStack(d->ui->stackedWidget);
    d->ui->sideBar->setButtonContent(0, "查看");
    d->ui->sideBar->setButtonContent(1, "编辑");
    d->ui->sideBar->setButtonContent(2, "设置");
    d->ui->sideBar->setButtonContent(3, "帮助");

    d->ui->stackedWidget->setCurrentIndex(0);

    d->displayWnd->editWnd = d->editWnd;

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
    if (!d->doc)
        d->doc = new WConfigDocument;

    QString configPath =
        WPath(PData).getModuleFolder(PPlugin->getId()) + "Quantify/config.json";
    if (!d->doc->load(configPath, true)) {
        QMessageBox::information(this, "首次使用",
                                 "前往 帮助>示例 创建初始数据后开始使用");
        return false;
    };

    QString path = d->doc->get("path").toString();
    QString addon = d->doc->get("addon").toString();
    QString engine = d->doc->get("engine").toString();
    QString temp = d->doc->get("template").toString();

    if (path.isEmpty()) {
        QMessageBox::information(this, "提示", "无路径！将使用默认路径");
        path = WPath().transPath(configPath, "./term1");
        d->doc->set("path", path);
    }

    path = WPath().transPath(configPath, path);
    d->doc->set("path", path);
    addon = WPath().transPath(configPath, addon);
    d->doc->set("addon", addon);
    temp = WPath().transPath(configPath, temp);
    d->doc->set("template", temp);
    if (engine != "js")
        d->doc->set("engine", "native");

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
