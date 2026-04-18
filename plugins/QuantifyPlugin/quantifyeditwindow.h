/**
 * @file quantifyeditwindow.h
 * @brief 考勤记录/规则/组文件编辑
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
#ifndef QUANTIFYEDITWINDOW_H
#define QUANTIFYEDITWINDOW_H
#include <QWidget>
#include <QFileDialog>
#include <QDesktopServices>
#include <QStandardPaths>

#include "WConfig/wconfigdocument.h"
#include "classrecord.h"
#include "qcalendarwidget.h"
#include "quantify.h"

enum InputType {TYPE_RULE,TYPE_RECORD};
namespace Ui {
class QuantifyEditWindow;
}

class QuantifyDisplayWindow;
class QuantifyEditWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuantifyEditWindow(QWidget *parent = nullptr);
    ~QuantifyEditWindow();
    void initialize(const Quantify::QuantifyComponents& components,
                    const Quantify::QuantifyUI& ui);
private slots:
    void on_comboBox_editTextChanged(const QString &arg1);
    void on_btnTemplate_clicked();
    void on_btnCheck_clicked();
    void on_textEdit_textChanged();
    void on_btnSave_clicked();
    void on_btnOpen_clicked();
    void on_btnClear_clicked();
    void on_addonButton_clicked();
    void on_keyboardButton_clicked();
    void on_calendarWidget_clicked(const QDate &date);
    void on_tabWidget_currentChanged(int index);
    void onNamelistButtonClicked();
    void on_calendarWidget_activated(const QDate &date);   // 双击/回车触发

public slots:
    void onUpdateSecurityInfo();

private:
    QString getCurrentFileExtension() const;
    QString getCurrentFileFilter() const;
    QString getCurrentDirectoryPath() const;
    QString readFileWithDecryption(const QString &filePath, bool isRecord) const;
    bool writeFileWithEncryption(const QString &filePath, const QString &content, bool isRecord) const;

    void loadNamelistButtons();          // 从 cr 加载姓名按钮
    void updateCalendarColors();         // 更新日历颜色
    QMap<QDate, int> countRecordFiles();   // 统计规则文件数量
    void updateSecurityInfo();
    InputType inType = TYPE_RULE;
    bool isChecked = false;

    ClassRecord* cr = nullptr;
    WConfigDocument* doc = nullptr;
    QuantifyDisplayWindow* displayWnd=nullptr;
    Ui::QuantifyEditWindow *ui;
};

#endif // QUANTIFYEDITWINDOW_H
