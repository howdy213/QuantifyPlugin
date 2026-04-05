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
#ifndef QUANTIFYDISPLAYVIEWDIALOG_H
#define QUANTIFYDISPLAYVIEWDIALOG_H

/**
 * @file classrecord.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-3-1
 * @version 1.3.0
 *
 * Copyright 2025-2026 howdy213
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QCloseEvent>
#include <QRegularExpression>

#include "studentrecordviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QuantifyDisplayViewDialog; }
QT_END_NAMESPACE

class QuantifyDisplayWindow;
class QuantifyDisplayViewDialogPrivate;
enum DisplayMode {
    Expand,      // 展开
    Summarize,   // 归纳（按规则类型）
    Collapse     // 折叠（按日期、成员）
};
class QuantifyDisplayViewDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuantifyDisplayViewDialog)
public:
    explicit QuantifyDisplayViewDialog(QWidget *parent = nullptr);
    ~QuantifyDisplayViewDialog();
    void setContent(const QVector<RecordInfo>& info);
    void setName(const QString& name);
    void setDialog(QuantifyDisplayWindow* dlg);
    void updateDisplay();
    void setGroupMode(bool isGroup);
    QString getName() const;
private slots:
    void on_btnExport_clicked();
    void on_btnFilter_clicked();
    void on_comboDisplayMode_currentIndexChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) override;
private:
    void createCol(int col, const QString& title, const QFont& font, const QColor& color);
    void createRow(int row, const RecordInfo &info);
    QScopedPointer<QuantifyDisplayViewDialogPrivate> d_ptr;
};

#endif // QUANTIFYDISPLAYVIEWDIALOG_H
