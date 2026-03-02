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
#include <Windows.h>

#include <TlHelp32.h>
#include <guiddef.h>
#include <shlobj.h>

#include "WECore/WConfig/wconfigdocument.h"
#include "WECore/WFile/wshellexecute.h"
#include "virtualkeyboard.h"

#include "quantifyeditwindow.h"
#include "ui_quantifyeditwindow.h"

using namespace we::Consts;
///
/// \brief QuantifyEditWindow::QuantifyEditWindow
/// \param parent
///
QuantifyEditWindow::QuantifyEditWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::QuantifyEditWindow) {
    ui->setupUi(this);
    ui->comboBox->addItem("record");
    ui->comboBox->addItem("rule");
    ui->comboBox->addItem("group");
    ui->comboBox->setCurrentIndex(0);
    QFont font;
    font.setPointSize(10);
    font.setFamily("宋体");
    ui->textEdit->setFont(font);
}
///
/// \brief QuantifyEditWindow::~QuantifyEditWindow
///
QuantifyEditWindow::~QuantifyEditWindow() { delete ui; }
///
/// \brief QuantifyEditWindow::on_comboBox_editTextChanged
/// \param arg1
///
void QuantifyEditWindow::on_comboBox_editTextChanged(const QString &arg1) {
    Q_UNUSED(arg1);
}
///
/// \brief QuantifyEditWindow::on_btnTemplate_clicked
///
void QuantifyEditWindow::on_btnTemplate_clicked() {
    QString tempPath = doc->get("template").toString();
    QString engine = doc->get("engine").toString();
    QString fileName;
    if (ui->comboBox->currentText() == "group") {
        ui->textEdit->setPlainText("name name_ch\nmember");
        return;
    }
    if (ui->comboBox->currentText() == "record")
        fileName = "record.txt";
    else if (engine == "native")
        fileName = "rule-native.txt";
    else if (engine == "js")
        fileName = "rule-js.txt";
    QString fullPath = tempPath + '/' + fileName;

    // 读取文件内容
    QFile file(fullPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString content = stream.readAll();
        ui->textEdit->setPlainText(content);
        file.close();
    } else {
        ui->textEdit->setPlainText(QString("无法打开文件：%1").arg(fullPath));
    }
}
///
/// \brief QuantifyEditWindow::on_btnCheck_clicked
///
void QuantifyEditWindow::on_btnCheck_clicked() {
    if (cr != nullptr) {
        CheckResult isValid;
        if (ui->comboBox->currentText() == "record")
            isValid = cr->isRecordValid(ui->textEdit->toPlainText());
        else if (ui->comboBox->currentText() == "group")
            isValid = cr->isGroupFileValid(ui->textEdit->toPlainText());
        else
            isValid = cr->isRuleValid(ui->textEdit->toPlainText());
        ui->labelCheck->setText(isValid.success ? "有效" : "无效");
        QPalette palette;
        palette.setColor(QPalette::WindowText,
                         isValid.success ? Qt::green : Qt::red);
        ui->labelCheck->setPalette(palette);
        this->isChecked = true;
        ui->logEdit->append(isValid.info);
    } else {
        this->isChecked = true;
        ui->labelCheck->setText("未刷新");
    }
}
///
/// \brief QuantifyEditWindow::on_textEdit_textChanged
///
void QuantifyEditWindow::on_textEdit_textChanged() {
    if (isChecked) {
        QPalette palette;
        palette.setColor(QPalette::WindowText, Qt::black);
        ui->labelCheck->setPalette(palette);
        ui->labelCheck->setText("未检查");
        isChecked = false;
    }
}
///
/// \brief QuantifyEditWindow::on_btnSave_clicked
///
void QuantifyEditWindow::on_btnSave_clicked() {
    QString currentPath = qvariant_cast<QString>(doc->get("path"));
    QString end;

    if (ui->comboBox->currentText() == "rule")
        end = ".rule";
    else if (ui->comboBox->currentText() == "record")
        end = ".record";
    else if (ui->comboBox->currentText() == "group")
        end = ".group";
    QString path = currentPath + "/" + end.mid(1) + "/";
    QString filePath =
        path + (ui->nameEdit->text() == "" ? "file" : ui->nameEdit->text()) + end;
    QString savePath = QFileDialog::getSaveFileName(
        nullptr, "保存文件", filePath, "Quantify File (*" + end + ")");
    if (savePath == "")
        return;
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->textEdit->toPlainText();
    }
}
///
/// \brief QuantifyEditWindow::on_btnOpen_clicked
///
void QuantifyEditWindow::on_btnOpen_clicked() {
    QString currentPath = qvariant_cast<QString>(doc->get("path"));
    QString end;
    if (ui->comboBox->currentText() == "rule")
        end = ".rule";
    else if (ui->comboBox->currentText() == "record")
        end = ".record";
    else if (ui->comboBox->currentText() == "group")
        end = ".group";
    QString path = currentPath + "/" + end.mid(1) + "/";
    QString fstr;
    if (ui->comboBox->currentText() == "rule")
        fstr = "Rule File (*.rule)";
    else if (ui->comboBox->currentText() == "record")
        fstr = "Record File (*.record)";
    else if (ui->comboBox->currentText() == "group")
        fstr = "Group File (*.group)";
    QStringList filter = {fstr};
    QString savePath = QFileDialog::getOpenFileName(nullptr, "打开文件", path,
                                                    filter.join(";;"));
    if (savePath == "")
        return;
    QFile file(savePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QFileInfo info(file);
        QTextStream in(&file);
        ui->textEdit->setPlainText(in.readAll());
        ui->nameEdit->setText(info.fileName().split(".")[0]);
    }
}
///
/// \brief QuantifyEditWindow::on_btnClear_clicked
///
void QuantifyEditWindow::on_btnClear_clicked() { ui->textEdit->clear(); }
///
/// \brief QuantifyEditWindow::on_addonButton_clicked
///
void QuantifyEditWindow::on_addonButton_clicked() {
    if (doc) {
        WShellExecute::syncExecute(doc->get("addon").toString());
    }
}
///
/// \brief QuantifyEditWindow::on_keyboardButton_clicked
///
void QuantifyEditWindow::on_keyboardButton_clicked() {
    VirtualKeyboard().OpenScreenKeyboard();
}
