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
#include "quantify.h"
#include "ui_quantifysettingwindow.h"

#include "QXlsx.h"
#include "WConfig/wconfigdocument.h"
#include "WFile/wpath.h"
#include "WFile/wshellexecute.h"
#include "WPlugin/wplugin.h"
#include "WPlugin/wplugindata.h"
#include "logger.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QVBoxLayout>

using namespace Consts;
using namespace Quantify;
using namespace Quantify::Consts;

QuantifySettingWindow::QuantifySettingWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::QuantifySettingWindow) {
    ui->setupUi(this);
    ui->comboEngine->addItems({EngineNative, EngineJS});

    ui->btnSaveSettings->setEnabled(false);
    ui->btnGenKeyPair->setEnabled(true);
}

QuantifySettingWindow::~QuantifySettingWindow() { delete ui; }

void QuantifySettingWindow::initialize(
    const Quantify::QuantifyComponents &components,
    const Quantify::QuantifyUI &ui) {
    m_doc = components.config;
    loadSettings();
    this->ui->btnSaveSettings->setEnabled(true);
}

void QuantifySettingWindow::loadSettings() {
    if (!m_doc)
        return;

    ui->editPath->setText(m_doc->get(DirPath).toString());
    ui->editAddon->setText(m_doc->get(DirAddon).toString());
    ui->editTemplate->setText(m_doc->get(DirTemplate).toString());
    ui->comboEngine->setCurrentText(m_doc->get(VarEngine).toString());
    ui->checkEncrypt->setChecked(m_doc->get(VarEncryption).toBool());
    updatePrivateKeyStatus();
}

void QuantifySettingWindow::updatePrivateKeyStatus() {
    bool hasKey = Encryptor::hasPrivateKey();
    ui->labelPrivateKeyStatus->setText(
        hasKey
            ? tr(("已找到私钥(" + Encryptor::getPrivateKeyPath() + ")").toUtf8())
            : tr("未找到私钥 (无法加密)"));
    ui->labelPrivateKeyStatus->setStyleSheet(hasKey ? "color: green;"
                                                    : "color: red;");

    // 如果未找到私钥，禁止启用加密复选框
    ui->checkEncrypt->setEnabled(hasKey);
    ui->btnMigrateRecords->setEnabled(hasKey);
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
    configObj[VarEncryption] = false; // 新示例默认不加密

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

    // 保存到配置文档
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
    loadSettings();
    QMessageBox::information(this, tr("成功"),
                             tr("设置已保存，部分更改需要重启插件后完全生效。"));
}

void QuantifySettingWindow::on_btnGenKeyPair_clicked() {
    // 查找可用的U盘（或让用户选择路径）
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

    QString privateKeyPath = QDir(selectedDrive).filePath("Quantify.pem");
    QString publicKeyPath = QDir(getConfigDir(m_doc)).filePath("public.pem");

    if (QFile::exists(privateKeyPath)) {
        int ret = QMessageBox::question(
            this, tr("私钥已存在"), tr("U盘根目录已存在 Quantify.pem，是否覆盖？"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
            return;
    }

    if (!Encryptor::generateKeyPair(privateKeyPath, publicKeyPath)) {
        QMessageBox::critical(this, tr("错误"),
                              tr("生成密钥对失败，请检查U盘是否可写。"));
        return;
    }

    QMessageBox::information(
        this, tr("成功"),
        tr("密钥对已生成！\n私钥保存至：%1\n公钥保存至：%2\n\n"
           "请将公钥文件分发给需要使用解密功能的人员。")
            .arg(privateKeyPath, publicKeyPath));
    updatePrivateKeyStatus();
}

void QuantifySettingWindow::on_btnMigrateRecords_clicked() {
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置文档未初始化"));
        return;
    }

    QDir dir(resolvePathWithKey(m_doc, DirPath));
    bool ok = dir.cd("record");
    if (!dir.exists() || !ok) {
        QMessageBox::warning(this, tr("错误"),
                             tr("记录目录不存在: %1").arg(dir.absolutePath()));
        return;
    }

    bool enableEncryption = ui->checkEncrypt->isChecked();
    if (!enableEncryption && !Encryptor::hasPrivateKey()) {
        QMessageBox::warning(this, tr("警告"),
                             tr("未找到私钥，无法解密现有加密文件。"));
        return;
    }

    int ret = QMessageBox::question(
        this, tr("确认迁移"),
        tr("将把目录 %1 下的所有 .record 文件%2。\n此操作不可逆，是否继续？")
            .arg(dir.absolutePath(), enableEncryption ? tr("加密") : tr("解密")),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;

    if (Encryptor::migrateRecordDirectory(dir.absolutePath(), enableEncryption)) {
        QMessageBox::information(this, tr("成功"), tr("迁移完成"));
    } else {
        QMessageBox::critical(this, tr("错误"),
                              tr("迁移过程中发生错误，请检查日志。"));
    }
}

void QuantifySettingWindow::on_checkEncrypt_stateChanged(int state) {}

void QuantifySettingWindow::browseDirectory(QLineEdit* lineEdit, const QString& title)
{
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

void QuantifySettingWindow::on_btnOpenNamelist_clicked()
{
    QString namelistPath = QDir(resolvePathWithKey(m_doc,DirPath)).filePath("namelist.xlsx");
    QDesktopServices::openUrl(QUrl("file:" + namelistPath, QUrl::TolerantMode));
}

