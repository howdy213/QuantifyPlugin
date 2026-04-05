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
#ifndef QUANTIFYSETTINGWINDOW_H
#define QUANTIFYSETTINGWINDOW_H
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>

#include "WConfig/wconfigdocument.h"

namespace Ui {
class QuantifySettingWindow;
}

class QuantifySettingWindow : public QWidget {
    Q_OBJECT
public:
    explicit QuantifySettingWindow(QWidget *parent = nullptr);
    ~QuantifySettingWindow();
    void setDoc(WConfigDocument *doc);
private slots:
    void on_btnOpenDir_clicked();
    void on_btnPath_clicked();
    void on_btnConfig_clicked();
    void on_btnSelectPath_clicked();
    void on_btnChangeConfig_clicked();

private:
    bool createTemplateFile(const QString &filePath, const QString &content);
    bool createNamelistExcel(const QString &filePath);
    bool createConfigFile(const QString &configPath,
                          const QDir &baseDir,
                          const QString &termDirName,
                          const QString &engineType);
    WConfigDocument *m_doc = nullptr;
    QString path = "";
    Ui::QuantifySettingWindow *ui;
};

#endif // QUANTIFYSETTINGWINDOW_H
