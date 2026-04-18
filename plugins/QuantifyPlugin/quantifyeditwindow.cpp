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
#include "encryptor.h"
#include "qcryptographichash.h"
#include "qmessagebox.h"
#include "quantify.h"
#include "virtualkeyboard.h"

#include "quantifydisplaywindow.h"
#include "quantifyeditwindow.h"
#include "ui_quantifyeditwindow.h"

using namespace we::Consts;
using namespace Quantify;
using namespace Quantify::Consts;
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

    connect(ui->calendarWidget, &QCalendarWidget::clicked, this,
            &QuantifyEditWindow::on_calendarWidget_clicked);
    connect(ui->calendarWidget, &QCalendarWidget::activated, this,
            &QuantifyEditWindow::on_calendarWidget_activated);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this,
            &QuantifyEditWindow::on_tabWidget_currentChanged);
}
///
/// \brief QuantifyEditWindow::~QuantifyEditWindow
///
QuantifyEditWindow::~QuantifyEditWindow() { delete ui; }
///
/// \brief QuantifyEditWindow::initialize
/// \param components
/// \param ui
///
void QuantifyEditWindow::initialize(
    const Quantify::QuantifyComponents &components,
    const Quantify::QuantifyUI &ui) {
    cr = components.classRecord;
    doc = components.config;
    displayWnd = ui.displayWindow;

    if (doc) {
        updateCalendarColors();
    }

    if (displayWnd) {
        connect(displayWnd, &QuantifyDisplayWindow::recordRefresh, this,
                &QuantifyEditWindow::onUpdateSecurityInfo);
    }
}
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
    QString tempPath = Quantify::resolvePathWithKey(doc, DirTemplate);
    QString engine = doc->get(VarEngine).toString();
    QString fileName;
    QString currentType = ui->comboBox->currentText();

    if (currentType == DirGroup) {
        ui->textEdit->setPlainText("name name_ch\nmember");
        return;
    }
    if (currentType == DirRecord)
        fileName = "record.txt";
    else if (engine == EngineNative)
        fileName = "rule-native.txt";
    else if (engine == EngineJS)
        fileName = "rule-js.txt";
    else
        return;

    QString fullPath = tempPath + '/' + fileName;
    QFile file(fullPath);
    QString content;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    } else {
        content = QString("无法打开文件：%1").arg(fullPath);
    }
    ui->textEdit->setPlainText(content);
}
///
/// \brief QuantifyEditWindow::on_btnCheck_clicked
///
void QuantifyEditWindow::on_btnCheck_clicked() {
    if (cr != nullptr) {
        CheckResult isValid;
        if (ui->comboBox->currentText() == "record")
            isValid = cr->isRecordValid(ui->textEdit->toPlainText());
        else if (ui->comboBox->currentText() == DirGroup)
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
    QString ext = getCurrentFileExtension();
    if (ext.isEmpty())
        return;

    QString dirPath = getCurrentDirectoryPath();
    QString defaultName =
        ui->nameEdit->text().isEmpty() ? "file" : ui->nameEdit->text();
    QString defaultPath = dirPath + defaultName + ext;

    QString savePath = QFileDialog::getSaveFileName(
        nullptr, "保存文件", defaultPath, getCurrentFileFilter());
    if (savePath.isEmpty())
        return;

    bool isRecord = (ui->comboBox->currentText() == "record");
    if (!writeFileWithEncryption(savePath, ui->textEdit->toPlainText(),
                                 isRecord)) {
        return;
    }
}
///
/// \brief QuantifyEditWindow::on_btnOpen_clicked
///
void QuantifyEditWindow::on_btnOpen_clicked() {
    QString path = getCurrentDirectoryPath();
    QString filter = getCurrentFileFilter();
    if (filter.isEmpty())
        return;

    QString savePath =
        QFileDialog::getOpenFileName(nullptr, "打开文件", path, filter);
    if (savePath.isEmpty())
        return;

    bool isRecord = (ui->comboBox->currentText() == "record");
    QString content = readFileWithDecryption(savePath, isRecord);
    if (content.isNull())
        return; // 错误已提示

    ui->textEdit->setPlainText(content);
    QFileInfo info(savePath);
    ui->nameEdit->setText(info.baseName());
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
        QString addonPath = Quantify::resolvePathWithKey(doc, DirAddon);
        WShellExecute::syncExecute(addonPath);
    }
}
///
/// \brief QuantifyEditWindow::on_keyboardButton_clicked
///
void QuantifyEditWindow::on_keyboardButton_clicked() {
    VirtualKeyboard().OpenScreenKeyboard();
}
QString QuantifyEditWindow::getCurrentFileExtension() const {
    QString type = ui->comboBox->currentText();
    if (type == "rule")
        return ".rule";
    if (type == "record")
        return ".record";
    if (type == DirGroup)
        return ".group";
    return "";
}

QString QuantifyEditWindow::getCurrentFileFilter() const {
    QString ext = getCurrentFileExtension();
    if (ext == ".rule")
        return "Rule File (*.rule)";
    if (ext == ".record")
        return "Record File (*.record)";
    if (ext == ".group")
        return "Group File (*.group)";
    return "";
}

QString QuantifyEditWindow::getCurrentDirectoryPath() const {
    if (!doc)
        return "";
    QString base = Quantify::resolvePathWithKey(doc, DirPath);
    QString ext = getCurrentFileExtension();
    if (ext.isEmpty())
        return base;
    // 去掉开头的点号作为子目录名
    QString subDir = ext.mid(1);
    return base + "/" + subDir + "/";
}

// 读取文件（自动解密记录文件）
QString QuantifyEditWindow::readFileWithDecryption(const QString &filePath,
                                                   bool isRecord) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(const_cast<QuantifyEditWindow *>(this), "错误",
                             "无法打开文件：" + file.errorString());
        return QString();
    }
    QByteArray fileData = file.readAll();
    file.close();

    if (isRecord && Encryptor::isEncrypted(fileData)) {
        QByteArray plainData = Encryptor::decryptData(fileData);
        if (plainData.isEmpty()) {
            QMessageBox::warning(const_cast<QuantifyEditWindow *>(this), "错误",
                                 "解密文件失败，文件可能已损坏或公钥无效");
            return QString();
        }
        QString text = QString::fromUtf8(plainData);
        if (text.isEmpty() && !plainData.isEmpty()) {
            QMessageBox::warning(const_cast<QuantifyEditWindow *>(this), "错误",
                                 "文件编码不是UTF-8，可能已损坏");
            return QString();
        }
        ui->labelEncryption->setText("已加密");
        return text;
    } else {
        ui->labelEncryption->setText("未加密");
        return QString::fromUtf8(fileData);
    }
}

// 保存文件（记录文件按配置决定是否加密）
bool QuantifyEditWindow::writeFileWithEncryption(const QString &filePath,
                                                 const QString &content,
                                                 bool isRecord) const {
    QByteArray plainData = content.toUtf8();
    QByteArray outData;

    bool encryptEnabled = false;
    if (doc && doc->hasArg(VarEncryption)) {
        encryptEnabled = doc->get(VarEncryption).toBool();
    }

    if (isRecord && encryptEnabled) {
        outData = Encryptor::encryptData(plainData);
        if (outData.isEmpty()) {
            QMessageBox::warning(const_cast<QuantifyEditWindow *>(this), "错误",
                                 "加密保存失败，请检查U盘私钥是否可用");
            return false;
        }
    } else {
        outData = plainData;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(const_cast<QuantifyEditWindow *>(this), "错误",
                             "无法写入文件");
        return false;
    }
    file.write(outData);
    file.close();
    return true;
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

        QPushButton *btn = new QPushButton(chName + "\n" + engName);
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

QMap<QDate, int> QuantifyEditWindow::countRecordFiles() {
    QMap<QDate, int> countMap;
    if (!doc)
        return countMap;
    QDir recordDir(Quantify::resolvePathWithKey(doc, DirPath) + "/record");
    if (!recordDir.exists())
        return countMap;

    QStringList filters;
    filters << "*.record";
    recordDir.setNameFilters(filters);
    QFileInfoList files = recordDir.entryInfoList();

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

void QuantifyEditWindow::updateSecurityInfo() {
    QString recordDir =
        Quantify::resolvePathWithKey(doc, DirPath) + "/record";
    QDir dir(recordDir);
    QStringList recordFiles =
        dir.entryList(QStringList() << "*.record", QDir::Files);

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QDateTime lastModified;
    QString lastModifiedFile;
    int encryptedFiles = 0;
    int totalFiles = 0;
    for (const QString &fileName : std::as_const(recordFiles)) {
        QFile file(dir.absoluteFilePath(fileName));
        if (file.open(QIODevice::ReadOnly)) {
            encryptedFiles += Encryptor::isEncrypted(file.read(4));
            hash.addData(fileName.toUtf8());
            QDateTime fileTime = QFileInfo(file).lastModified();
            if (fileTime > lastModified) {
                lastModifiedFile = file.fileName();
                lastModified = fileTime;
            }
            file.close();
        }
        totalFiles++;
    }

    QByteArray hashResult = hash.result();
    QString hashHex = QString::number(static_cast<uchar>(hashResult[0]), 16)
                          .rightJustified(2, '0');
    QString timeStr = lastModified.toString("yyyy-M-d h:mm:ss:zzz");

    QString securityText =
        QString("%1 %2 %3 %4/%5 ")
                               .arg(hashHex, timeStr, lastModifiedFile.split('/').last()).arg(encryptedFiles).arg(totalFiles);
    if (ui->labelSecurity) {
        ui->labelSecurity->setText(securityText);
    }
}

void QuantifyEditWindow::updateCalendarColors() {
    QMap<QDate, int> counts = countRecordFiles();

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
    QString recordDir = Quantify::resolvePathWithKey(doc, "path") + "/record";
    QDir dir(recordDir);
    if (!dir.exists())
        return;

    QString datePrefix = date.toString("yyyyMMdd");
    QStringList filters;
    filters << datePrefix + "*.record";
    dir.setNameFilters(filters);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    if (files.isEmpty())
        return;

    QString filePath = files.first().absoluteFilePath();

    // 使用统一解密函数读取记录文件
    QString content = readFileWithDecryption(filePath, true);
    if (content.isNull())
        return;

    ui->textEdit->setPlainText(content);
    ui->nameEdit->setText(files.first().baseName());
}

void QuantifyEditWindow::onUpdateSecurityInfo() { updateSecurityInfo(); }
void QuantifyEditWindow::on_tabWidget_currentChanged(int index) {
    QWidget *current = ui->tabWidget->widget(index);
    if (current == ui->tabCalendar) {
        updateCalendarColors();
    } else if (current == ui->tabNameList) {
        loadNamelistButtons();
    }
}
