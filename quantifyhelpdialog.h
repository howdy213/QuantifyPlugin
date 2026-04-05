/**
 * @file quantifyhelptwindow.h
 * @brief 帮助对话框
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
#ifndef QUANTIFYHELPDIALOG_H
#define QUANTIFYHELPDIALOG_H

#include <QDialog>
#include <QString>
#include "WECore/WDef/wedef.h"
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class QuantifyHelpDialog; }
QT_END_NAMESPACE

class QuantifyHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuantifyHelpDialog(WConfigDocument *doc, QWidget *parent = nullptr);
    ~QuantifyHelpDialog();

private:
    // 初始化各个标签页
    void setupIntroTab();
    void setupQuantifyTab();
    void setupRecordTab();
    void setupRuleTab();
    void setupGroupTab();
    void setupConfigTab();
    void setupNamelistTab();
    void setupSecurityTab();
    void setupAboutTab();

    Ui::QuantifyHelpDialog *ui;
    WConfigDocument *m_doc;
};

#endif // QUANTIFYHELPDIALOG_H
