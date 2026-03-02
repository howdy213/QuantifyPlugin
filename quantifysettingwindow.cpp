/**
 * @file quantifysettingwindow.h
 * @brief 考勤设置
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
#include "quantifysettingwindow.h"
#include "WConfig/wconfigdocument.h"
#include "ui_quantifysettingwindow.h"

///
/// \brief QuantifySettingWindow::QuantifySettingWindow
/// \param parent
///
QuantifySettingWindow::QuantifySettingWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::QuantifySettingWindow) {
    ui->setupUi(this);
    ui->comboBox->addItems({"native", "js"});
    ui->comboBox->setDisabled(true);
    ui->btnSelectPath->setDisabled(true);
    ui->btnConfig->setDisabled(true);
}
///
/// \brief QuantifySettingWindow::~QuantifySettingWindow
///
QuantifySettingWindow::~QuantifySettingWindow() { delete ui; }

void QuantifySettingWindow::setDoc(WConfigDocument *doc) {
    this->doc = doc;
    path = doc->get("path").toString();
    ui->labelPathContent->setText(path);
    ui->comboBox->setCurrentText(doc->get("engine").toString());
}
///
/// \brief QuantifySettingWindow::on_btnPath_clicked
///
void QuantifySettingWindow::on_btnPath_clicked() {
    QDesktopServices::openUrl(QUrl("file:" + path, QUrl::TolerantMode));
}
///
/// \brief QuantifySettingWindow::on_btnConfig_clicked
///
void QuantifySettingWindow::on_btnConfig_clicked() {}
///
/// \brief QuantifySettingWindow::on_btnSelectPath_clicked
///
void QuantifySettingWindow::on_btnSelectPath_clicked() {}
