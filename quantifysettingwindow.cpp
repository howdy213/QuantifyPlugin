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
#include "WFile/wpath.h"
#include "WFile/wshellexecute.h"
#include "WPlugin/wplugin.h"
#include "WPlugin/wplugindata.h"
#include "qdialogbuttonbox.h"
#include "qlineedit.h"
#include "qmessagebox.h"
#include "ui_quantifysettingwindow.h"
#include "xlsxdocument.h"
using namespace Consts;

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
///
/// \brief QuantifySettingWindow::setDoc
/// \param doc
///
void QuantifySettingWindow::setDoc(WConfigDocument *doc) {
    this->m_doc = doc;
    path = doc->get("path").toString();
    ui->labelPathContent->setText(path);
    ui->comboBox->setCurrentText(doc->get("engine").toString());
}
///
/// \brief QuantifySettingWindow::on_btnOpenDir_clicked
///
void QuantifySettingWindow::on_btnOpenDir_clicked() {
    WShellExecute::syncExecute(
        WPath().splitPath(PPlugin->getMetaData(Plugin::Path).toString()));
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
///
/// \brief QuantifySettingWindow::on_btnChangeConfig_clicked
///
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
    comboEngine->addItem("native");
    comboEngine->addItem("js");
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

    QStringList subDirs = {"addon",
                           "template",
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
                            "({\n"
                            "    reason: 'late',\n"
                            "    reason_ch: '迟到',\n"
                            "    daily: function(record) { return record; },\n"
                            "    weekly: function(record) { return record; },\n"
                            "    termly: function(record) { return record; }\n"
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
///
/// \brief QuantifySettingWindow::createTemplateFile
/// \param filePath
/// \param content
/// \return
///
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
///
/// \brief QuantifySettingWindow::createNamelistExcel
/// \param filePath
/// \return
///
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
///
/// \brief QuantifyHelpDialog::createConfigFile
/// \param configPath
/// \param baseDir
/// \param termDirName
/// \param engineType
/// \return
///
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
    // 路径使用相对路径（相对于配置文件所在目录），例如 "./term1"
    configObj["path"] = "./" + termDirName;
    configObj["addon"] =
        m_doc->hasArg("addon") ? m_doc->get("addon").toString() : "./addon";
    configObj["engine"] = engineType;
    configObj["template"] = m_doc->hasArg("template")
                                ? m_doc->get("template").toString()
                                : "./template";

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
