/**
 * @file quantifyeditwindow.cpp
 * @brief 编辑窗口实现
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
#include <Windows.h>

#include <TlHelp32.h>
#include <guiddef.h>
#include <shlobj.h>

#include "WECore/file/wshellexecute.h"
#include "WECore/metadata/WMetaDocument.h"
#include "encryptor.h"
#include "qmessagebox.h"
#include "quantify.h"
#include "virtualkeyboard.h"
#include <QCryptoGraphicHash>

#include "quantifydisplaywindow.h"
#include "quantifyeditwindow.h"
#include "ui_quantifyeditwindow.h"

using namespace we;
using namespace we::Consts;
using namespace Quantify;
using namespace Quantify::Consts;

///
/// \brief 构造函数，初始化界面、字体、信号连接等
/// \param parent 父窗口
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
/// \brief 析构函数
///
QuantifyEditWindow::~QuantifyEditWindow() { delete ui; }

///
/// \brief 初始化窗口，设置核心组件与UI引用
/// \param components 量化组件集合
/// \param ui 量化UI接口
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

    if (this->ui->textEdit->toPlainText().isEmpty()) {
        QString defaultPath =
            Quantify::resolvePathWithKey(doc, DirTemplate).filePath("default.txt");
        QFile file(defaultPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QString content = stream.readAll();
            if (!content.isEmpty()) {
                replaceTextEditContent(content);
            }
            file.close();
        }
    }
    navigateToLatestRecord();
}

void QuantifyEditWindow::setClassRecord(ClassRecord *cr) {
    this->cr = cr;
    updateCalendarColors(); // 重新标记日历中各日期有无记录
    loadNamelistButtons();  // 重新加载学生姓名按钮（成员可能增减）
}

///
/// \brief 组合框文本变化事件（当前未使用）
/// \param arg1 新文本
///
void QuantifyEditWindow::on_comboBox_editTextChanged(const QString &arg1) {
    Q_UNUSED(arg1);
}

///
/// \brief 模板按钮点击槽：根据当前类型和引擎加载对应模板文件
///
void QuantifyEditWindow::on_btnTemplate_clicked() {
    QDir tempPath = Quantify::resolvePathWithKey(doc, DirTemplate);
    QString engine = doc->get(VarEngine).toString();
    QString fileName;
    QString currentType = ui->comboBox->currentText();

    if (currentType == DirGroup) {
        // 分组模板直接写入固定内容
        replaceTextEditContent("name name_ch\nmember");
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

    QString fullPath = tempPath.absoluteFilePath(fileName);
    QFile file(fullPath);
    QString content;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    } else {
        content = QString("无法打开文件：%1").arg(fullPath);
    }
    // 使用支持撤销的替换函数
    replaceTextEditContent(content);
}

///
/// \brief 检查按钮点击槽：调用 ClassRecord 的校验接口并更新UI状态
///
void QuantifyEditWindow::on_btnCheck_clicked() {
    if (cr != nullptr) {
        CheckResult isValid;
        if (ui->comboBox->currentText() == "record") {
            isValid = cr->isRecordValid(ui->textEdit->toPlainText());
            QRegularExpression fileNameRegex(R"(^(\d{8})-(\d+)$)");
            QString baseName = QFileInfo(ui->nameEdit->text()).baseName();
            if (!fileNameRegex.match(baseName).hasMatch()) {
                isValid.success = false;
                isValid.info += (isValid.info.isEmpty() ? "" : "\n");
                isValid.info += "文件名不是规范格式";
            }
        } else if (ui->comboBox->currentText() == DirGroup)
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
/// \brief 文本编辑框内容变化时重置检查状态
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
/// \brief 保存按钮槽：弹出保存对话框，根据配置决定是否加密
///
void QuantifyEditWindow::on_btnSave_clicked() {
    QString ext = getCurrentFileExtension();
    if (ext.isEmpty())
        return;

    QDir dirPath = getCurrentDirectoryPath();
    QString defaultName =
        ui->nameEdit->text().isEmpty() ? "file" : ui->nameEdit->text();
    QString defaultPath = dirPath.absoluteFilePath(defaultName + ext);

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
/// \brief 打开按钮槽：弹出打开对话框，读取并解密文件
///
void QuantifyEditWindow::on_btnOpen_clicked() {
    QDir path = getCurrentDirectoryPath();
    QString filter = getCurrentFileFilter();
    if (filter.isEmpty())
        return;

    QString savePath = QFileDialog::getOpenFileName(nullptr, "打开文件",
                                                    path.absolutePath(), filter);
    if (savePath.isEmpty())
        return;

    bool isRecord = (ui->comboBox->currentText() == "record");
    QString content = readFileWithDecryption(savePath, isRecord);
    if (content.isNull())
        return; // 错误已提示

    // 使用支持撤销的替换函数加载内容
    replaceTextEditContent(content);
    QFileInfo info(savePath);
    ui->nameEdit->setText(info.baseName());
}

///
/// \brief 清空按钮槽：删除 textEdit 全部文本（已支持撤销）
///
void QuantifyEditWindow::on_btnClear_clicked() {
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.deleteChar();
    cursor.endEditBlock();
}

///
/// \brief 打开插件目录按钮
///
void QuantifyEditWindow::on_addonButton_clicked() {
    if (doc) {
        QDir addonPath = resolvePathWithKey(doc, DirAddon);
        WShellExecute::syncExecute(addonPath.absolutePath());
    }
}

///
/// \brief 打开软键盘按钮
///
void QuantifyEditWindow::on_keyboardButton_clicked() {
    VirtualKeyboard().OpenScreenKeyboard();
}

///
/// \brief 获取当前类型对应的文件扩展名（含点）
/// \return 扩展名字符串，如 ".rule"
///
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

///
/// \brief 获取当前类型的文件过滤器字符串
/// \return 用于 QFileDialog 的过滤器
///
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

///
/// \brief 获取当前类型的默认目录路径
/// \return 例如 "basepath/record/"
///
QDir QuantifyEditWindow::getCurrentDirectoryPath() const {
    if (!doc)
        return QDir();
    QDir base = Quantify::getTermDir(doc);
    QString ext = getCurrentFileExtension();
    if (ext.isEmpty())
        return base;
    // 去掉开头的点号作为子目录名
    QString subDir = ext.mid(1);
    base.cd(subDir);
    return base;
}

///
/// \brief 读取文件内容，若为记录文件且已加密则自动解密
/// \param filePath 文件完整路径
/// \param isRecord 是否为记录文件
/// \return 文件内容的 UTF-8 字符串，失败时返回空 QString
///
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

///
/// \brief 保存文件，根据配置和类型决定是否加密
/// \param filePath 保存路径
/// \param content 文本内容
/// \param isRecord 是否为记录文件
/// \return 保存成功返回 true
///
bool QuantifyEditWindow::writeFileWithEncryption(const QString &filePath,
                                                 const QString &content,
                                                 bool isRecord) const {
    QByteArray plainData = content.toUtf8();
    QByteArray outData;

    // 自动判断加密模式：只要记录目录中存在任意加密文件，则新文件也加密
    bool encryptEnabled = false;
    if (isRecord && doc) {
        encryptEnabled = Encryptor::hasEncryptedRecords(
            Quantify::getTermDir(doc).absolutePath());
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

///
/// \brief 从 ClassRecord 加载学生姓名列表并生成按钮网格
///
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

///
/// \brief 姓名按钮点击槽：在光标位置插入学生英文名并换行
///
void QuantifyEditWindow::onNamelistButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;

    QString engName = btn->property("englishName").toString();
    if (engName.isEmpty())
        return;

    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock(); // 将整次插入封装为一个撤销单元
    cursor.insertText(engName + "\n");
    cursor.endEditBlock();
    ui->textEdit->setTextCursor(cursor);
}

///
/// \brief 日历点击槽：将选中日期格式化为名称建议
/// \param date 选中的日期
///
void QuantifyEditWindow::on_calendarWidget_clicked(const QDate &date) {
    QString dateStr = date.toString("yyyyMMdd") + "-";
    ui->nameEdit->setText(dateStr);
}

///
/// \brief 统计各日期下的记录文件数量
/// \return 日期到文件计数的映射
///
QMap<QDate, int> QuantifyEditWindow::countRecordFiles() {
    QMap<QDate, int> countMap;
    if (!doc)
        return countMap;
    QDir recordDir = Quantify::getTermDir(doc).filePath("record");
    if (!recordDir.exists())
        return countMap;

    QStringList filters;
    filters << "*.record";
    recordDir.setNameFilters(filters);
    QFileInfoList files = recordDir.entryInfoList();

    for (const QFileInfo &fi : std::as_const(files)) {
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

///
/// \brief 更新安全信息标签：显示文件哈希、最后修改时间、加密比例
///
void QuantifyEditWindow::updateSecurityInfo() {
    QDir dir = Quantify::getTermDir(doc).filePath("record");
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
                               .arg(hashHex, timeStr, lastModifiedFile.split('/').last())
                               .arg(encryptedFiles)
                               .arg(totalFiles);
    if (ui->labelSecurity) {
        ui->labelSecurity->setText(securityText);
    }
}

///
/// \brief 更新日历颜色：根据记录文件数量标记背景色
///
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

///
/// \brief 日历激活槽：双击日期时加载当天第一个记录文件内容
/// \param date 激活的日期
///
void QuantifyEditWindow::on_calendarWidget_activated(const QDate &date) {
    if (!doc)
        return;
    QDir dir = getTermDir(doc).filePath("record");
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

    // 支持撤销的文本替换
    replaceTextEditContent(content);
    ui->nameEdit->setText(files.first().baseName());
}

///
/// \brief 外部通知的安全信息更新槽
///
void QuantifyEditWindow::onUpdateSecurityInfo() { updateSecurityInfo(); }

///
/// \brief 标签页切换槽：根据页面更新日历颜色或姓名列表
/// \param index 当前页索引
///
void QuantifyEditWindow::on_tabWidget_currentChanged(int index) {
    QWidget *current = ui->tabWidget->widget(index);
    if (current == ui->tabCalendar) {
        updateCalendarColors();
    } else if (current == ui->tabNameList) {
        loadNamelistButtons();
    }
}
///
/// \brief QuantifyEditWindow::navigateToLatestRecord
///
void QuantifyEditWindow::navigateToLatestRecord() {
    if (!doc)
        return;
    QMap<QDate, int> counts = countRecordFiles();
    if (counts.isEmpty())
        return;
    QDate latest = counts.lastKey(); // QMap 按日期升序，lastKey 即最大日期
    ui->calendarWidget->setSelectedDate(latest);
}
///
/// \brief 替换 textEdit 全部内容，并保留撤销/重做历史
/// \param text 要设置的新文本
///
void QuantifyEditWindow::replaceTextEditContent(const QString &text) {
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock();              // 开始一个编辑块
    cursor.select(QTextCursor::Document); // 选中全部文本
    cursor.insertText(text);              // 插入新文本，自动覆盖选区
    cursor.endEditBlock();                // 结束编辑块，形成单次撤销操作
    cursor.movePosition(QTextCursor::Start);
    ui->textEdit->setTextCursor(cursor);
}

void QuantifyEditWindow::loadRecordFileByIndex(int index) {
    if (!doc) {
        QMessageBox::warning(this, "错误", "配置未加载");
        return;
    }

    // 获取当前日期前缀（例如 "20260606-"）
    QString prefix = ui->nameEdit->text();
    if (prefix.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先点击日历选择一个日期");
        return;
    }
    // 确保前缀以短横线结尾，如果不是则自动补全
    if (prefix.contains('-')) {
        // 可能是 "20260606" 这种情况
        prefix = prefix.left(prefix.lastIndexOf('-')) + "-";
    }

    QString fileName = prefix + QString::number(index) + ".record";
    QString recordDir = getTermDir(doc).filePath(DirRecord);
    QString filePath = QDir(recordDir).filePath(fileName);

    if (!QFile::exists(filePath)) {
        ui->labelCheck->setText("读取文件失败");
        ui->nameEdit->setText(fileName.left(fileName.lastIndexOf('.')));
        // QMessageBox::information(this, "提示",
        //                          QString("文件不存在：%1").arg(fileName));
        return;
    }

    QString content = readFileWithDecryption(filePath, true);
    if (content.isNull())
        return; // readFileWithDecryption 已弹出错误提示

    replaceTextEditContent(content);
    ui->nameEdit->setText(
        fileName.left(fileName.lastIndexOf('.'))); // 去掉 .record 后缀
    ui->labelCheck->setText("读取文件成功");
}

void QuantifyEditWindow::on_btnFile1_clicked() { loadRecordFileByIndex(1); }

void QuantifyEditWindow::on_btnFile2_clicked() { loadRecordFileByIndex(2); }

void QuantifyEditWindow::on_btnFile3_clicked() { loadRecordFileByIndex(3); }
