/**
 * @file quantifydialog.h
 * @brief 主对话框
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
#ifndef QUANTIFYDIALOG_H
#define QUANTIFYDIALOG_H
#include <QWidget>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class QuantifyDialog; }
QT_END_NAMESPACE

class QuantifyPlugin;
class QuantifyDialogPrivate;

class QuantifyDialog : public QWidget {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuantifyDialog)
public:
    explicit QuantifyDialog(QWidget *parent = nullptr);
    ~QuantifyDialog();
    bool readConfig();
    void setPlugin(QuantifyPlugin *plugin);
private slots:
    void onRequestDialogRestart();
    void onSettingsChanged();
    void onSettingUnsavedChanged(bool hasUnsaved);
private:
    void rebuildClassRecord();
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;   // 新增：用于延迟显示错误
private:
    QScopedPointer<QuantifyDialogPrivate> d_ptr;
    QString m_keyMismatchError;                   // 存储密钥不匹配错误信息
};

#endif // QUANTIFYDIALOG_H