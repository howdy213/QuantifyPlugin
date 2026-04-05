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
#include <Windows.h>

#include <TlHelp32.h>
#include <guiddef.h>
#include <shlobj.h>

#include "WECore/WConfig/wconfigdocument.h"
#include "WECore/WFile/wshellexecute.h"
#include "qcryptographichash.h"
#include "virtualkeyboard.h"

#include "quantifyeditwindow.h"
#include "quantifydisplaywindow.h"
#include "ui_quantifyeditwindow.h"

using namespace we::Consts;
///
/// \brief QuantifyEditWindow::QuantifyEditWindow
/// \param parent
///
QuantifyEditWindow::QuantifyEditWindow(QuantifyDisplayWindow* displayWnd,QWidget *parent)
    : QWidget(parent), ui(new Ui::QuantifyEditWindow) {
    this->displayWnd=displayWnd;
    ui->setupUi(this);
    ui->comboBox->addItem("record");
    ui->comboBox->addItem("rule");
    ui->comboBox->addItem("group");
    ui->comboBox->setCurrentIndex(0);
    QFont font;
    font.setPointSize(10);
    font.setFamily("宋体");
    ui->textEdit->setFont(font);
    connect(ui->calendarWidget, &QCalendarWidget::clicked, this,
            &QuantifyEditWindow::on_calendarWidget_clicked); // 单击填充文件名
    connect(ui->calendarWidget, &QCalendarWidget::activated, this,
            &QuantifyEditWindow::on_calendarWidget_activated); // 双击打开文件
    connect(ui->tabWidget, &QTabWidget::currentChanged, this,
            &QuantifyEditWindow::on_tabWidget_currentChanged);
    if (doc) {
        updateCalendarColors();
    }

    connect(displayWnd,&QuantifyDisplayWindow::recordRefresh,this,&QuantifyEditWindow::onUpdateSecurityInfo);
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
    QString content;
    QFile file(fullPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    } else
        content = QString("无法打开文件：%1").arg(fullPath);
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.deleteChar();
    cursor.insertText(content);
    cursor.endEditBlock();
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
void QuantifyEditWindow::on_btnClear_clicked() {
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.deleteChar();
    cursor.endEditBlock();
}
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
void QuantifyEditWindow::loadNamelistButtons() {
    // 清除旧布局
    QLayout *oldLayout = ui->namelistWidget->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    if (!cr) {
        ui->logEdit->append("学生数据未加载，无法显示姓名列表。");
        return;
    }

    QGridLayout *grid = new QGridLayout(ui->namelistWidget);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    const int COLUMNS = 5; // 每行显示5个按钮
    int row = 0, col = 0;

    for (auto it = cr->students.constBegin(); it != cr->students.constEnd();
         ++it) {
        QString engName = it.key();          // 英文缩写
        QString chName = it.value().name_ch; // 中文名
        if (chName.isEmpty())
            continue;

        QPushButton *btn = new QPushButton(chName+"\n"+engName);
        btn->setProperty("englishName", engName);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(btn, &QPushButton::clicked, this,
                &QuantifyEditWindow::onNamelistButtonClicked);
        grid->addWidget(btn, row, col);

        if (++col >= COLUMNS) {
            col = 0;
            ++row;
        }
    }
}

void QuantifyEditWindow::onNamelistButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;

    QString engName = btn->property("englishName").toString();
    if (engName.isEmpty())
        return;

    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.insertText(engName + "\n");
    ui->textEdit->setTextCursor(cursor);
}

void QuantifyEditWindow::on_calendarWidget_clicked(const QDate &date) {
    QString dateStr = date.toString("yyyyMMdd") + "-";
    ui->nameEdit->setText(dateStr);
}

QMap<QDate, int> QuantifyEditWindow::countRuleFiles() {
    QMap<QDate, int> countMap;
    if (!doc)
        return countMap;

    QString path = doc->get("path").toString();
    QDir ruleDir(path + "/record");
    if (!ruleDir.exists())
        return countMap;

    QStringList filters;
    filters << "*.record";
    ruleDir.setNameFilters(filters);
    QFileInfoList files = ruleDir.entryInfoList();

    for (const QFileInfo &fi : files) {
        QString baseName = fi.baseName(); // 例如 "20260314-1"
        if (baseName.length() < 8)
            continue;
        QString dateStr = baseName.left(8);
        QDate date = QDate::fromString(dateStr, "yyyyMMdd");
        if (date.isValid()) {
            countMap[date]++;
        }
    }
    return countMap;
}

void QuantifyEditWindow::updateSecurityInfo()
{
    QString recordDir = doc->get("path").toString() + "/record";
    QDir dir(recordDir);
    QStringList recordFiles = dir.entryList(QStringList() << "*.record", QDir::Files);

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QDateTime lastModified;
    QString lastModifiedFile;

    for (const QString& fileName : std::as_const(recordFiles)) {
        QFile file(dir.absoluteFilePath(fileName));
        if (file.open(QIODevice::ReadOnly)) {
            hash.addData(&file);
            QDateTime fileTime = QFileInfo(file).lastModified();
            if (fileTime > lastModified){
                lastModifiedFile=file.fileName();
                lastModified = fileTime;
            }
            file.close();
        }
    }

    QByteArray hashResult = hash.result();
    QString hashHex = QString::number(static_cast<uchar>(hashResult[0]), 16).rightJustified(2, '0');
    QString timeStr = lastModified.toString("yyyy-M-d h:mm:ss:zzz");

    QString securityText = QString("%1 %2 %3").arg(hashHex, timeStr,lastModifiedFile.split('/').last());
    if (ui->labelSecurity) {
        ui->labelSecurity->setText(securityText);
    }
}

void QuantifyEditWindow::updateCalendarColors() {
    QMap<QDate, int> counts = countRuleFiles();

    // 清除之前所有日期的特殊格式
    QTextCharFormat defaultFormat;
    ui->calendarWidget->setDateTextFormat(QDate(), defaultFormat);

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        QTextCharFormat fmt;
        int cnt = it.value();
        if (cnt == 1)
            fmt.setBackground(Qt::green);
        else if (cnt == 2)
            fmt.setBackground(Qt::yellow);
        else if (cnt >= 3)
            fmt.setBackground(Qt::red);

        fmt.setForeground(Qt::black);
        ui->calendarWidget->setDateTextFormat(it.key(), fmt);
    }
}

void QuantifyEditWindow::on_calendarWidget_activated(const QDate &date) {
    if (!doc)
        return;

    QString path = doc->get("path").toString();
    QDir recordDir(path + "/record");
    if (!recordDir.exists())
        return;

    QString datePrefix = date.toString("yyyyMMdd");
    QStringList filters;
    filters << datePrefix + "*.record";
    recordDir.setNameFilters(filters);
    QFileInfoList files = recordDir.entryInfoList();
    if (files.isEmpty())
        return;

    QString filePath = files.first().absoluteFilePath();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->textEdit->setPlainText(in.readAll());
        ui->nameEdit->setText(files.first().baseName()); // 填充不带后缀的文件名
        file.close();
    }
}

void QuantifyEditWindow::onUpdateSecurityInfo()
{
    updateSecurityInfo();
}
void QuantifyEditWindow::on_tabWidget_currentChanged(int index) {
    QWidget *current = ui->tabWidget->widget(index);
    if (current == ui->tabCalendar) {
        updateCalendarColors();
    } else if (current == ui->tabNameList) {
        loadNamelistButtons();
    }
}
