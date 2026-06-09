/**
 * @file quantifysettingwindow.h
 * @brief 量化插件设置窗口
 * @author howdy213
 * @date 2026-06-09
 * @version 2.1.1
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
#ifndef QUANTIFYSETTINGWINDOW_H
#define QUANTIFYSETTINGWINDOW_H

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QWidget>
#include <QProgressDialog>

#include "WECore/metadata/WMetaDocument.h"
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

    void initialize(const Quantify::QuantifyComponents &components,
                    const Quantify::QuantifyUI &ui);
    void loadSettings();           // 从 doc 加载当前设置到界面
    void updatePrivateKeyStatus(); // 更新私钥状态显示

signals:
    void settingsChanged();
    void requestDialogRestart();   // 请求重启主对话框
    void unsavedChangesChanged(bool hasUnsaved);

private slots:
    void onAnyInputChanged();

    // 基础操作
    void on_btnOpenDir_clicked();      // 打开插件目录
    void on_btnPath_clicked();         // 打开数据目录
    void on_btnChangeConfig_clicked(); // 新建示例
    void on_btnSaveSettings_clicked(); // 保存所有设置

    // 密钥与加密
    void on_btnGenKeyPair_clicked();          // 生成密钥对到U盘
    void on_btnMigrateRecords_clicked();      // 迁移记录文件
    void on_btnImportPublicKey_clicked();     // 导入公钥
    void on_btnRefreshKeys_clicked();         // 刷新密钥状态

    // 目录浏览
    void on_btnBrowsePath_clicked();     // 浏览数据目录
    void on_btnBrowseAddon_clicked();    // 浏览附加程序目录
    void on_btnBrowseTemplate_clicked(); // 浏览模板目录

    // 备份/恢复
    void on_btnBackup_clicked();  // 备份
    void on_btnRestore_clicked(); // 还原

    // 日志与名单
    void on_btnLogOpen_clicked();       // 打开日志文件
    void on_btnLogClear_clicked();      // 清空日志
    void on_btnOpenNamelist_clicked();  // 打开名单Excel

    // 其他
    void on_btnRestart_clicked();          // 重启插件
    void on_btnCopySemester_clicked();     // 复制学期目录

    void updateEncryptionModeCheckbox();   // 更新加密模式显示

private:
    // 备份恢复辅助
    QString performBackup(const QString &backupRoot);
    bool performRestore(const QString &backupDir);
    QStringList getBackupSourceItems() const;
    static bool copyDirectoryRecursively(const QString &srcPath,
                                         const QString &dstPath,
                                         bool overwrite = false,
                                         QProgressDialog *progress = nullptr);
    static bool removeDirectoryRecursively(const QString &path);
    void showCopyProgressDialog(const QString &title,
                                const QString &label,
                                int maximum,
                                std::function<bool(QProgressDialog*)> worker);

    // 配置与文件辅助
    void browseDirectory(QLineEdit *lineEdit, const QString &title);
    bool createNamelistExcel(const QString &filePath);
    bool createConfigFile(const QString &configPath, const QDir &baseDir,
                          const QString &termDirName, const QString &engineType);

    we::WMetaDocument *m_doc = nullptr;
    Ui::QuantifySettingWindow *ui;
    QuantifyDisplayWindow *m_displayWnd = nullptr;
    bool m_hasUnsavedChanges = false;
    bool m_loading = false;
};

#endif // QUANTIFYSETTINGWINDOW_H