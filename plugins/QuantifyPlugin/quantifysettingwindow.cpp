/**
 * @file quantifysettingwindow.cpp
 * @brief 量化插件设置窗口实现
 * @author howdy213
 * @date 2026-4-12
 * @version 1.5.0
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
#include "quantifysettingwindow.h"
#include "quantify.h"
#include "quantifydisplaywindow.h"
#include "ui_quantifysettingwindow.h"

#include "QXlsx.h"
#include "WECore/file/wpath.h"
#include "WECore/file/wshellexecute.h"
#include "WECore/metadata/WMetaDocument.h"
#include "WECore/plugin/wplugin.h"
#include "WECore/plugin/wplugindata.h"
#include "logger.h"

#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QVBoxLayout>

using namespace we;
using namespace we::Consts;
using namespace Quantify;
using namespace Quantify::Consts;

QuantifySettingWindow::QuantifySettingWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::QuantifySettingWindow) {
    ui->setupUi(this);
    ui->comboEngine->addItems({EngineNative, EngineJS});
    ui->btnSaveSettings->setEnabled(false);
    ui->btnGenKeyPair->setEnabled(true);

    // 连接所有输入控件的修改信号到 onAnyInputChanged
    connect(ui->editPath, &QLineEdit::textChanged, this,
            &QuantifySettingWindow::onAnyInputChanged);
    connect(ui->editAddon, &QLineEdit::textChanged, this,
            &QuantifySettingWindow::onAnyInputChanged);
    connect(ui->editTemplate, &QLineEdit::textChanged, this,
            &QuantifySettingWindow::onAnyInputChanged);
    connect(ui->comboEngine, &QComboBox::currentTextChanged, this,
            &QuantifySettingWindow::onAnyInputChanged);
    connect(ui->checkEncrypt, &QCheckBox::checkStateChanged, this,
            &QuantifySettingWindow::onAnyInputChanged);

    ui->checkEncrypt->setEnabled(false);
}

QuantifySettingWindow::~QuantifySettingWindow() { delete ui; }

void QuantifySettingWindow::initialize(
    const Quantify::QuantifyComponents &components,
    const Quantify::QuantifyUI &ui) {
    m_displayWnd = ui.displayWindow;
    m_doc = components.config;
    loadSettings(); // 会重置未保存标志
    this->ui->btnSaveSettings->setEnabled(true);
}

void QuantifySettingWindow::onAnyInputChanged() {
    if (m_loading)
        return; // 加载过程中不触发
    if (!m_hasUnsavedChanges) {
        m_hasUnsavedChanges = true;
        ui->labelInfo->setText("保存设置后继续使用");
        emit unsavedChangesChanged(true);
    }
}

void QuantifySettingWindow::loadSettings() {
    if (!m_doc)
        return;

    m_loading = true; // 禁止触发 onAnyInputChanged

    ui->editPath->setText(m_doc->get(DirPath).toString());
    ui->editAddon->setText(m_doc->get(DirAddon).toString());
    ui->editTemplate->setText(m_doc->get(DirTemplate).toString());
    ui->comboEngine->setCurrentText(m_doc->get(VarEngine).toString());
    ui->checkEncrypt->setChecked(m_doc->get(VarEncryption).toBool());

    updatePrivateKeyStatus();
    updateEncryptionModeCheckbox();
    m_loading = false;

    // 重置未保存标志
    if (m_hasUnsavedChanges) {
        m_hasUnsavedChanges = false;
        emit unsavedChangesChanged(false);
    }
}

void QuantifySettingWindow::updatePrivateKeyStatus() {
    bool hasKey = Encryptor::hasPrivateKey();
    ui->labelPrivateKeyStatus->setText(
        hasKey ? tr("已找到私钥(%1)").arg(Encryptor::getPrivateKeyPath())
               : tr("未找到私钥 (无法加密保存)"));
    ui->labelPrivateKeyStatus->setStyleSheet(hasKey ? "color: green;"
                                                    : "color: red;");
    if (!hasKey && ui->checkEncrypt->isChecked()) {
        ui->labelPrivateKeyStatus->setToolTip(
            tr("加密已启用但私钥缺失，请插入U盘或生成密钥对"));
    }
    updateEncryptionModeCheckbox();
}

void QuantifySettingWindow::on_btnOpenDir_clicked() {
    WShellExecute::syncExecute(
        WPath().splitPath(PPlugin->getMetaData(Plugin::Path).toString()));
}

void QuantifySettingWindow::on_btnPath_clicked() {
    QString path = Quantify::resolvePath(m_doc, ui->editPath->text());
    QDesktopServices::openUrl(QUrl("file:" + path, QUrl::TolerantMode));
}

void QuantifySettingWindow::on_btnChangeConfig_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法获取配置文档，示例创建失败。"));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("新建示例"));
    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);

    QLabel *labelPath =
        new QLabel(tr("请输入子目录名称（仅允许字母、数字、下划线）："));
    QLineEdit *editPath = new QLineEdit;
    editPath->setPlaceholderText("例如 term1");

    QLabel *labelEngine = new QLabel(tr("请选择规则引擎："));
    QComboBox *comboEngine = new QComboBox;
    comboEngine->addItem(EngineNative);
    comboEngine->addItem(EngineJS);
    comboEngine->setCurrentIndex(0);

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    mainLayout->addWidget(labelPath);
    mainLayout->addWidget(editPath);
    mainLayout->addWidget(labelEngine);
    mainLayout->addWidget(comboEngine);
    mainLayout->addWidget(buttonBox);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString termDirName = editPath->text().trimmed();
    QString engineType = comboEngine->currentText();

    static QRegularExpression re("^[a-zA-Z0-9_]+$");
    if (termDirName.isEmpty() || !re.match(termDirName).hasMatch()) {
        QMessageBox::warning(
            this, tr("错误"),
            tr("目录名称只能包含字母、数字和下划线，且不能为空。"));
        return;
    }

    QString pluginPath =
        WPath().splitPath(PPlugin->getMetaData(Plugin::Path).toString());
    QString basePath = pluginPath + "Quantify/";
    QDir baseDir(basePath);

    if (baseDir.exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("目录已存在"));
        msgBox.setText(tr("目录 %1 已存在，请选择操作：").arg(basePath));
        QPushButton *deleteButton =
            msgBox.addButton(tr("删除并重建"), QMessageBox::YesRole);
        QPushButton *continueButton =
            msgBox.addButton(tr("继续(不清除)"), QMessageBox::NoRole);
        QPushButton *cancelButton =
            msgBox.addButton(tr("取消"), QMessageBox::RejectRole);
        msgBox.exec();

        if (msgBox.clickedButton() == cancelButton) {
            return;
        } else if (msgBox.clickedButton() == deleteButton) {
            if (!baseDir.removeRecursively()) {
                QMessageBox::critical(this, tr("错误"),
                                      tr("无法清空目录，请检查权限。"));
                return;
            }
            if (!QDir().mkpath(basePath)) {
                QMessageBox::critical(this, tr("错误"), tr("无法创建基础目录。"));
                return;
            }
            baseDir = QDir(basePath);
        }
    }

    QStringList subDirs = {DirAddon,
                           DirTemplate,
                           termDirName,
                           termDirName + "/rule",
                           termDirName + "/record",
                           termDirName + "/group"};
    for (const QString &sub : subDirs) {
        if (!baseDir.mkpath(sub)) {
            QMessageBox::critical(this, tr("错误"), tr("无法创建目录 %1").arg(sub));
            return;
        }
    }

    if (!createTemplateFile(baseDir.filePath("template/record.txt"),
                            "daily\n[late]\n\n"))
        return;
    if (!createTemplateFile(baseDir.filePath("template/rule-native.txt"),
                            "reason reason_ch\n-\n-\n-\n-\n"))
        return;
    if (!createTemplateFile(baseDir.filePath("template/rule-js.txt"),
                            "({ \n"
                            "    reason: 'assembly+',\n"
                            "    reason_ch: '集会',\n"
                            "    daily: function(ctx) {\n"
                            "        ctx.record.t += 1;\n"
                            "        ctx.record.s += 0.1;\n"
                            "        return ctx;\n"
                            "    },\n"
                            "    weekly: function(ctx) { return ctx; },\n"
                            "    termly: function(ctx) { return ctx; }\n"
                            "})\n"))
        return;

    QString excelPath = baseDir.filePath(termDirName + "/namelist.xlsx");
    if (!createNamelistExcel(excelPath))
        return;

    QString configPath = pluginPath + "Quantify/config.json";
    if (!createConfigFile(configPath, baseDir, termDirName, engineType))
        return;

    QMessageBox::information(
        this, tr("成功"),
        tr("示例文件夹及文件已创建在：%1\n\n配置文件已生成：%"
           "2\n请重启插件以应用新配置。")
            .arg(basePath, configPath));
}

bool QuantifySettingWindow::createTemplateFile(const QString &filePath,
                                               const QString &content) {
    QFile file(filePath);
    if (file.exists())
        return true;
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("错误"),
                              tr("无法创建模板文件：%1").arg(filePath));
        return false;
    }
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

bool QuantifySettingWindow::createNamelistExcel(const QString &filePath) {
    if (QFile::exists(filePath))
        return true;
    QXlsx::Document xlsx;
    xlsx.write(1, 1, "zs");
    xlsx.write(1, 2, "张三");
    xlsx.write(2, 1, "ls");
    xlsx.write(2, 2, "李四");
    xlsx.write(3, 1, "ww");
    xlsx.write(3, 2, "王五");
    if (!xlsx.saveAs(filePath)) {
        QMessageBox::critical(this, tr("错误"),
                              tr("无法创建 namelist.xlsx 文件。"));
        return false;
    }
    return true;
}

bool QuantifySettingWindow::createConfigFile(const QString &configPath,
                                             const QDir &baseDir,
                                             const QString &termDirName,
                                             const QString &engineType) {
    QFileInfo configInfo(configPath);
    QDir configDir = configInfo.absoluteDir();
    if (!configDir.exists() && !configDir.mkpath(".")) {
        QMessageBox::critical(this, tr("错误"), tr("无法创建配置目录。"));
        return false;
    }

    if (QFile::exists(configPath)) {
        int ret = QMessageBox::question(
            this, tr("配置文件已存在"),
            tr("配置文件 %1 已存在，是否覆盖？").arg(configPath),
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No)
            return false;
    }

    QJsonObject configObj;
    configObj[DirPath] = "./" + termDirName;
    configObj[DirAddon] =
        m_doc->hasArg(DirAddon) ? m_doc->get(DirAddon).toString() : "./addon";
    configObj[VarEngine] = engineType;
    configObj[DirTemplate] = m_doc->hasArg(DirTemplate)
                                 ? m_doc->get(DirTemplate).toString()
                                 : "./template";
    // configObj[VarEncryption] = false; // 已废弃，但保留

    QJsonDocument doc(configObj);
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("错误"), tr("无法写入配置文件。"));
        return false;
    }
    configFile.write(doc.toJson());
    configFile.close();
    return true;
}

void QuantifySettingWindow::on_btnSaveSettings_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置文档未初始化"));
        return;
    }

    QString newPath = ui->editPath->text().trimmed();
    QString newAddon = ui->editAddon->text().trimmed();
    QString newTemplate = ui->editTemplate->text().trimmed();
    QString newEngine = ui->comboEngine->currentText();
    bool newEncrypt = ui->checkEncrypt->isChecked();

    m_doc->set(DirPath, newPath);
    m_doc->set(DirAddon, newAddon);
    m_doc->set(DirTemplate, newTemplate);
    m_doc->set(VarEngine, newEngine);
    m_doc->set(VarEncryption, newEncrypt);

    QString configPath =
        WPath(PData).getModuleFolder(PPlugin->getId()) + "Quantify/config.json";
    if (!m_doc->save(configPath)) {
        QMessageBox::critical(this, tr("错误"), tr("保存配置文件失败"));
        return;
    }
    loadSettings(); // 重新加载（会重置未保存标志）
    ui->labelInfo->setText("已保存");
    QMessageBox::information(this, tr("成功"),
                             tr("设置已保存，部分更改需要重启插件后完全生效。"));
    emit settingsChanged();
}

void QuantifySettingWindow::on_btnGenKeyPair_clicked() {
    bool encryptEnabled = ui->checkEncrypt->isChecked();
    if (encryptEnabled) {
        QString recordDir = Quantify::resolvePathWithKey(m_doc, DirPath);
        if (Encryptor::hasEncryptedRecords(recordDir)) {
            QMessageBox::warning(
                this, tr("操作禁止"),
                tr("当前已启用加密且存在加密记录文件，无法生成新密钥对。\n"
                   "请先关闭“加密记录文件”选项，并解密所有记录文件后再试。"));
            return;
        }
    }

    QFileInfoList drives = QDir::drives();
    QString selectedDrive;
    if (drives.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("未找到任何磁盘驱动器。"));
        return;
    }
    if (drives.size() == 1) {
        selectedDrive = drives.first().absolutePath();
    } else {
        QStringList driveNames;
        for (const QFileInfo &drive : std::as_const(drives))
            driveNames << drive.absolutePath();
        bool ok;
        selectedDrive =
            QInputDialog::getItem(this, tr("选择U盘"), tr("请选择要保存私钥的U盘:"),
                                              driveNames, 0, false, &ok);
        if (!ok || selectedDrive.isEmpty())
            return;
    }

    const QString privateKeyPath = QDir(selectedDrive).filePath("Quantify.pem");
    const QString publicKeyOnDrive = QDir(selectedDrive).filePath("Quantify.pub");
    const QString publicKeyInConfig =
        QDir(getConfigDir(m_doc)).filePath("public.pem");

    const bool encryptionEnabled = ui->checkEncrypt->isChecked();
    if (encryptionEnabled && QFile::exists(publicKeyInConfig)) {
        QMessageBox::warning(this, tr("操作禁止"),
                             tr("当前已启用加密模式，无法重新生成密钥对。\n"
                                "请先关闭“加密记录文件”选项后再试。"));
        return;
    }

    QStringList existingFiles;
    if (QFile::exists(privateKeyPath))
        existingFiles << tr("U盘私钥: %1").arg(privateKeyPath);
    if (QFile::exists(publicKeyOnDrive))
        existingFiles << tr("U盘公钥: %1").arg(publicKeyOnDrive);
    if (QFile::exists(publicKeyInConfig))
        existingFiles << tr("配置公钥: %1").arg(publicKeyInConfig);

    if (!existingFiles.isEmpty()) {
        QString msg =
            tr("以下文件已存在，是否覆盖？\n\n%1").arg(existingFiles.join('\n'));
        int ret = QMessageBox::question(this, tr("确认覆盖"), msg,
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
            return;
    }

    if (!Encryptor::generateKeyPair(privateKeyPath, publicKeyInConfig)) {
        QMessageBox::critical(this, tr("错误"),
                              tr("生成密钥对失败，请检查U盘是否可写。"));
        return;
    }

    if (QFile::exists(publicKeyOnDrive)) {
        if (!QFile::remove(publicKeyOnDrive)) {
            QMessageBox::warning(this, tr("部分成功"),
                                 tr("无法删除 U 盘上的旧公钥（%1），复制公钥失败。\n"
                                    "请手动将 %2 复制到 U 盘根目录。")
                                     .arg(publicKeyOnDrive, publicKeyInConfig));
            return;
        }
    }
    if (!QFile::copy(publicKeyInConfig, publicKeyOnDrive)) {
        QMessageBox::warning(this, tr("部分成功"),
                             tr("密钥对已生成，但无法将公钥复制到 U 盘（%1）。\n"
                                "请手动将 %2 复制到 U 盘根目录。")
                                 .arg(publicKeyOnDrive, publicKeyInConfig));
    }

    QMessageBox::information(
        this, tr("成功"),
        tr("密钥对已生成！\n"
           "私钥: %1\n"
           "公钥(U盘备份): %2\n"
           "公钥(插件目录): %3\n\n"
           "请将公钥文件分发给需要使用解密功能的人员。")
            .arg(privateKeyPath, publicKeyOnDrive, publicKeyInConfig));

    updatePrivateKeyStatus();
}

void QuantifySettingWindow::on_btnMigrateRecords_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置文档未初始化"));
        return;
    }

    QString recordDir = Quantify::resolvePathWithKey(m_doc, DirPath) + "/record";
    if (!QDir(recordDir).exists()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("记录目录不存在: %1").arg(recordDir));
        return;
    }

    bool currentMode = Encryptor::isEncryptionModeActive(recordDir);
    QString targetDesc = currentMode ? tr("解密") : tr("加密");
    int ret = QMessageBox::question(this, tr("确认迁移"),
                                    tr("将把目录 %1 下的所有 .record 文件统一为 "
                                       "%2 格式。\n此操作不可逆，是否继续？")
                                        .arg(recordDir, targetDesc),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;

    if (!currentMode && !Encryptor::hasPrivateKey()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("当前需要加密模式，但未找到私钥，无法加密。"));
        return;
    }

    if (Encryptor::migrateRecordDirectory(recordDir, !currentMode)) {
        QMessageBox::information(this, tr("成功"), tr("迁移完成"));
        updateEncryptionModeCheckbox(); // 刷新复选框状态
        emit settingsChanged();
    } else {
        QMessageBox::critical(this, tr("错误"),
                              tr("迁移过程中发生错误，请检查日志。"));
    }
}

void QuantifySettingWindow::on_checkEncrypt_stateChanged(int state) {}

void QuantifySettingWindow::browseDirectory(QLineEdit *lineEdit,
                                            const QString &title) {
    QString currentText = lineEdit->text();
    QString configDir = getConfigDir(m_doc);
    QString initialDir;
    if (QDir(currentText).isAbsolute()) {
        initialDir = currentText;
    } else if (!currentText.isEmpty()) {
        initialDir = QDir(configDir).filePath(currentText);
    } else {
        initialDir = configDir;
    }
    QString dir = QFileDialog::getExistingDirectory(this, title, initialDir);
    if (!dir.isEmpty()) {
        QDir config(configDir);
        QString relative = config.relativeFilePath(dir);
        if (!relative.startsWith("..") && !QFileInfo(relative).isAbsolute()) {
            lineEdit->setText(relative);
        } else {
            lineEdit->setText(dir);
        }
    }
}

void QuantifySettingWindow::on_btnBrowsePath_clicked() {
    browseDirectory(ui->editPath, tr("选择数据目录"));
}

void QuantifySettingWindow::on_btnBrowseAddon_clicked() {
    browseDirectory(ui->editAddon, tr("选择附加程序目录"));
}

void QuantifySettingWindow::on_btnBrowseTemplate_clicked() {
    browseDirectory(ui->editTemplate, tr("选择模板目录"));
}

// 递归统计目录下所有文件的数量
static int countFilesOnly(const QString &path) {
    QDir dir(path);
    int total = 0;
    QFileInfoList entries =
        dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : std::as_const(entries)) {
        if (info.isDir())
            total += countFilesOnly(info.filePath());
        else
            ++total;
    }
    return total;
}

bool QuantifySettingWindow::copyDirectoryRecursively(
    const QString &srcPath, const QString &dstPath, bool overwrite,
    QProgressDialog *progress) {
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists() && !dstDir.mkpath("."))
        return false;

    bool success = true;
    QFileInfoList entries =
        srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    int total = entries.size();
    int current = 0;

    for (const QFileInfo &info : entries) {
        if (progress && progress->wasCanceled())
            return false;

        if (progress) {
            progress->setValue(current++);
            qApp->processEvents();
        }

        QString srcItem = info.filePath();
        QString dstItem = dstDir.filePath(info.fileName());
        if (info.isDir()) {
            if (!copyDirectoryRecursively(srcItem, dstItem, overwrite, progress))
                success = false;
            // 目录不增加进度值
        } else {
            if (QFile::exists(dstItem) && !overwrite)
                continue;
            if (!QFile::copy(srcItem, dstItem)) {
                success = false;
                Logger::instance().error(
                    QString("复制文件失败: %1 -> %2").arg(srcItem, dstItem));
            }
            // 仅文件增加进度
            if (progress) {
                progress->setValue(progress->value() + 1);
                qApp->processEvents();
            }
        }
    }

    if (progress)
        progress->setValue(total);
    return success;
}

void QuantifySettingWindow::showCopyProgressDialog(
    const QString &title, const QString &label, int maximum,
    std::function<bool(QProgressDialog *)> worker) {
    QProgressDialog progress(label, tr("取消"), 0, maximum, this);
    progress.setWindowTitle(title);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);
    progress.setValue(0);

    bool result = worker(&progress);
    if (progress.wasCanceled())
        QMessageBox::information(this, tr("已取消"), tr("操作已被用户取消。"));
    else if (!result)
        QMessageBox::critical(this, tr("错误"),
                              tr("操作过程中发生错误，请检查日志。"));
    else
        QMessageBox::information(this, tr("成功"), tr("操作完成。"));
}

void QuantifySettingWindow::on_btnBackup_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置文档未初始化。"));
        return;
    }

    QString backupRoot = QFileDialog::getExistingDirectory(
        this, tr("选择备份根目录"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (backupRoot.isEmpty())
        return;

    QString backupDir = performBackup(backupRoot);
    if (!backupDir.isEmpty()) {
        QMessageBox::information(this, tr("备份成功"),
                                 tr("数据已备份到：\n%1").arg(backupDir));
    }
}

void QuantifySettingWindow::on_btnRestore_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置文档未初始化。"));
        return;
    }
    QString backupDir = QFileDialog::getExistingDirectory(
        this, tr("选择备份文件夹"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (backupDir.isEmpty())
        return;

    performRestore(backupDir);
}

bool QuantifySettingWindow::removeDirectoryRecursively(const QString &path) {
    QDir dir(path);
    if (!dir.exists())
        return true;
    return dir.removeRecursively();
}

void QuantifySettingWindow::on_btnLogOpen_clicked() {
    QString logPath = QDir(WPath(PData).getModuleFolder(PPlugin->getId()))
                          .filePath("Quantify/logs/quantify.log");
    QDesktopServices::openUrl(QUrl("file:" + logPath, QUrl::TolerantMode));
}

void QuantifySettingWindow::on_btnLogClear_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("清空日志"), tr("确定要清空日志文件吗？此操作不可恢复。"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        Logger::instance().clear();
        QMessageBox::information(this, tr("成功"), tr("日志已清空。"));
    }
}

void QuantifySettingWindow::on_btnOpenNamelist_clicked() {
    QString namelistPath =
        QDir(resolvePathWithKey(m_doc, DirPath)).filePath("namelist.xlsx");
    QDesktopServices::openUrl(QUrl("file:" + namelistPath, QUrl::TolerantMode));
}

QStringList QuantifySettingWindow::getBackupSourceItems() const {
    QStringList items;
    QString configDir = getConfigDir(m_doc);
    QDir baseDir(configDir);
    if (!baseDir.exists())
        return items;

    items << "config.json";
    if (QFile::exists(baseDir.filePath("public.pem")))
        items << "public.pem";
    if (baseDir.exists(DirTemplate))
        items << DirTemplate;
    QString dataDirName = m_doc->get(DirPath).toString();
    if (!dataDirName.isEmpty()) {
        QDir dataDir = baseDir.filePath(dataDirName);
        if (dataDir.exists())
            items << dataDirName;
        else
            Logger::instance().warn("备份: 数据目录不存在: " + dataDirName);
    } else {
        Logger::instance().warn("备份: 配置中未指定数据目录 (path)");
    }

    return items;
}

QString QuantifySettingWindow::performBackup(const QString &backupRoot) {
    QString configDir = getConfigDir(m_doc);
    if (configDir.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法获取配置目录"));
        return QString();
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupDir =
        QDir(backupRoot).filePath(QString("QuantifyBackup_%1").arg(timestamp));
    QDir backupPath(backupDir);
    if (!backupPath.mkpath(".")) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法创建备份目录: %1").arg(backupDir));
        return QString();
    }

    QStringList items = getBackupSourceItems();
    int totalItems = items.size();
    int current = 0;

    QProgressDialog progress(tr("正在备份..."), tr("取消"), 0, totalItems, this);
    progress.setWindowTitle(tr("备份量化数据"));
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    bool canceled = false;
    bool failed = false;

    for (const QString &item : items) {
        if (progress.wasCanceled()) {
            canceled = true;
            break;
        }
        progress.setLabelText(tr("正在备份: %1").arg(item));
        progress.setValue(current++);
        qApp->processEvents();

        QString srcPath = QDir(configDir).filePath(item);
        QString dstPath = QDir(backupDir).filePath(item);
        if (QFileInfo(srcPath).isDir()) {
            if (!copyDirectoryRecursively(srcPath, dstPath, false)) {
                Logger::instance().error("备份目录失败: " + srcPath);
                failed = true;
                break;
            }
        } else {
            if (!QFile::copy(srcPath, dstPath)) {
                Logger::instance().error("备份文件失败: " + srcPath);
                failed = true;
                break;
            }
        }
    }
    progress.setValue(totalItems);

    if (canceled) {
        // 删除不完整的备份目录
        QDir(backupDir).removeRecursively();
        return QString();
    }
    if (failed) {
        QMessageBox::critical(this, tr("错误"),
                              tr("备份过程中发生错误，请查看日志。"));
        return QString();
    }

    return backupDir;
}

bool QuantifySettingWindow::performRestore(const QString &backupDir) {
    QString configDir = getConfigDir(m_doc);
    if (configDir.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法获取配置目录"));
        return false;
    }

    // 验证备份目录有效性：至少包含 config.json 或一个学期目录
    QDir backupPath(backupDir);
    if (!backupPath.exists("config.json") &&
        backupPath.entryList(QDir::Dirs).isEmpty()) {
        QMessageBox::warning(this, tr("无效备份"),
                             tr("所选备份文件夹不包含有效数据。"));
        return false;
    }

    int ret = QMessageBox::question(
        this, tr("确认恢复"),
        tr("恢复操作将用备份文件夹中的内容覆盖当前配置目录：\n%1\n\n"
           "当前目录中的同名文件将被替换，但不会删除备份中不存在的文件。\n是否继"
           "续？")
            .arg(configDir),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return false;

    // 获取备份中的所有项目（文件和目录）
    QFileInfoList entries =
        backupPath.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    int total = entries.size();
    int current = 0;
    bool failed = false;

    QProgressDialog progress(tr("正在恢复..."), tr("取消"), 0, total, this);
    progress.setWindowTitle(tr("恢复量化数据"));
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    for (const QFileInfo &entry : std::as_const(entries)) {
        if (progress.wasCanceled()) {
            return false;
        }
        progress.setLabelText(tr("正在恢复: %1").arg(entry.fileName()));
        progress.setValue(current++);
        qApp->processEvents();

        QString srcPath = entry.absoluteFilePath();
        QString dstPath = QDir(configDir).filePath(entry.fileName());

        if (entry.isDir()) {
            // 先删除目标目录（避免残留旧文件），再复制
            QDir dstDir(dstPath);
            if (dstDir.exists() && !dstDir.removeRecursively()) {
                Logger::instance().error("无法删除原有目录: " + dstPath);
                failed = true;
                break;
            }
            if (!copyDirectoryRecursively(srcPath, dstPath, true)) {
                Logger::instance().error("恢复目录失败: " + srcPath);
                failed = true;
                break;
            }
        } else {
            // 文件直接覆盖
            if (QFile::exists(dstPath) && !QFile::remove(dstPath)) {
                Logger::instance().error("无法删除原文件: " + dstPath);
                failed = true;
                break;
            }
            if (!QFile::copy(srcPath, dstPath)) {
                Logger::instance().error("恢复文件失败: " + srcPath);
                failed = true;
                break;
            }
        }
    }
    progress.setValue(total);

    if (failed) {
        QMessageBox::critical(this, tr("错误"),
                              tr("恢复过程中发生错误，请查看日志。"));
        return false;
    }

    emit requestDialogRestart();
    return true;
}

void QuantifySettingWindow::on_btnRestart_clicked() {

    if (QMessageBox::question(this, tr("提示"), tr("立即重启？")) ==
        QMessageBox::Yes)
    emit requestDialogRestart();
}

void QuantifySettingWindow::updateEncryptionModeCheckbox() {
    if (!m_doc)
        return;
    bool active = Encryptor::hasEncryptedRecords(
        Quantify::resolvePathWithKey(m_doc, DirPath));
    ui->checkEncrypt->setChecked(active);
    ui->checkEncrypt->setToolTip(active ? tr("当前为加密模式")
                                        : tr("当前为明文模式"));
}

void QuantifySettingWindow::on_btnImportPublicKey_clicked() {
    if (Encryptor::hasEncryptedRecords(
            Quantify::resolvePathWithKey(m_doc, DirPath))) {
        QMessageBox::information(this, "提示", "请在解除所有加密后再导入");
        return;
    }
    QString configDir = getConfigDir(m_doc);
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("导入公钥"), configDir, tr("PUB文件 (*.pub)"));
    if (filePath.isEmpty())
        return;

    QString destPath = QDir(configDir).filePath("public.pem");
    if(destPath==filePath)return;

    // 尝试加载用户选择的公钥文件
    if (!Encryptor::loadPublicKeyFromFile(filePath)) {
        QMessageBox::critical(this, tr("错误"),
                              tr("加载公钥失败，文件格式可能错误。"));
        return;
    }

    // 如果当前有私钥，检查是否匹配
    if (Encryptor::hasPrivateKey() && !Encryptor::keysMatch()) {
        QMessageBox::warning(
            this, tr("警告"),
            tr("导入的公钥与当前私钥不匹配，解密功能可能不可用。"));
        Encryptor::clearPrivateKey();
    }

    // 将导入的公钥保存到插件目录，以便下次启动自动加载
    if (QFile::exists(destPath) && !QFile::remove(destPath)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("无法替换原有公钥文件，请手动处理。"));
    } else {
        if (QFile::copy(filePath, destPath)) {
            Logger::instance().info("公钥已保存到 " + destPath);
        } else {
            QMessageBox::warning(this, tr("警告"),
                                 tr("无法保存公钥到插件目录，下次启动可能失效。"));
        }
    }

    updatePrivateKeyStatus();
    updateEncryptionModeCheckbox();
    QMessageBox::information(this, tr("成功"), tr("公钥导入成功。"));
}

void QuantifySettingWindow::on_btnRefreshKeys_clicked() {
    // 清除内存中的密钥
    Encryptor::clearPublicKey();
    Encryptor::clearPrivateKey();

    QString configDir = getConfigDir(m_doc);

    // 重新扫描U盘加载私钥
    QFileInfoList drives = QDir::drives();
    QString privateKeyPath;
    for (const QFileInfo &drive : std::as_const(drives)) {
        QString path = drive.absolutePath() + "/Quantify.pem";
        if (QFile::exists(path)) {
            privateKeyPath = path;
            break;
        }
    }
    if (!privateKeyPath.isEmpty()) {
        Encryptor::loadPrivateKey(privateKeyPath);
    } else {
        Logger::instance().info("刷新密钥：未找到U盘私钥");
    }

    // 重新加载公钥（优先配置目录，否则内置）
    Encryptor::loadPublicKeyWithFallback(configDir);

    // 检查匹配性
    if (Encryptor::hasPrivateKey() && Encryptor::hasPublicKey() &&
        !Encryptor::keysMatch()) {
        QMessageBox::warning(
            this, tr("警告"),
            tr("当前加载的私钥与公钥不匹配，功能可能不可用。"));
        Encryptor::clearPrivateKey();
    }

    updatePrivateKeyStatus();
    updateEncryptionModeCheckbox();
    QMessageBox::information(this, tr("完成"), tr("已刷新密钥状态。"));
}
