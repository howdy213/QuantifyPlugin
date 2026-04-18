/**
 * @file quantifysettingwindow.h
 * @brief 量化插件设置窗口
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
#ifndef QUANTIFYSETTINGWINDOW_H
#define QUANTIFYSETTINGWINDOW_H

#include <QWidget>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "WConfig/wconfigdocument.h"
#include "encryptor.h"
#include "quantify.h"

namespace Ui {
class QuantifySettingWindow;
}

class QuantifySettingWindow : public QWidget {
    Q_OBJECT
public:
    explicit QuantifySettingWindow(QWidget *parent = nullptr);
    ~QuantifySettingWindow();
    void initialize(const Quantify::QuantifyComponents& components,
                    const Quantify::QuantifyUI& ui);

    void loadSettings();                    // 从 doc 加载当前设置到界面
    void updatePrivateKeyStatus();          // 更新私钥状态显示

private slots:
    void on_btnOpenDir_clicked();           // 打开插件目录
    void on_btnPath_clicked();              // 打开数据目录
    void on_btnChangeConfig_clicked();      // 新建示例
    void on_btnSaveSettings_clicked();      // 保存所有设置
    void on_btnGenKeyPair_clicked();        // 生成密钥对到U盘
    void on_btnMigrateRecords_clicked();    // 迁移记录文件
    void on_checkEncrypt_stateChanged(int state); // 加密开关变化

    void on_btnBrowsePath_clicked();        // 浏览数据目录
    void on_btnBrowseAddon_clicked();       // 浏览附加程序目录
    void on_btnBrowseTemplate_clicked();    // 浏览模板目录

    void on_btnLogOpen_clicked();

    void on_btnLogClear_clicked();

    void on_btnOpenNamelist_clicked();

private:
    void browseDirectory(QLineEdit* lineEdit, const QString& title);
    bool createTemplateFile(const QString &filePath, const QString &content);
    bool createNamelistExcel(const QString &filePath);
    bool createConfigFile(const QString &configPath, const QDir &baseDir,
                          const QString &termDirName, const QString &engineType);

    WConfigDocument *m_doc = nullptr;
    Ui::QuantifySettingWindow *ui;
};

#endif // QUANTIFYSETTINGWINDOW_H