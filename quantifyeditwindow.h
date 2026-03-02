/**
 * @file quantifyeditwindow.h
 * @brief 考勤记录/规则编辑
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
#ifndef QUANTIFYEDITWINDOW_H
#define QUANTIFYEDITWINDOW_H
#include <QWidget>
#include <QFileDialog>
#include <QDesktopServices>
#include <QStandardPaths>

#include "WConfig/wconfigdocument.h"
#include "classrecord.h"

enum InputType {TYPE_RULE,TYPE_RECORD};
namespace Ui {
class QuantifyEditWindow;
}

class QuantifyEditWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuantifyEditWindow(QWidget *parent = nullptr);
    ~QuantifyEditWindow();
    ClassRecord* cr=nullptr;
    WConfigDocument* doc=nullptr;
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

private:
    InputType inType=TYPE_RULE;
    bool isChecked=false;
private:
    Ui::QuantifyEditWindow *ui;
};

#endif // QUANTIFYEDITWINDOW_H
