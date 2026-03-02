#ifndef QUANTIFYDISPLAYWINDOW_H
#define QUANTIFYDISPLAYWINDOW_H

#include <QtWidgets/QWidget>
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
#include "WConfig/wconfigdocument.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QTableWidget>
#include "classrecord.h"
#include "quantifydisplayviewdialog.h"
#include "quantifyeditwindow.h"

namespace Ui {
class QuantifyDisplayWindow;
}

class QuantifyDisplayWindow : public QWidget
{
    Q_OBJECT
public:
    ClassRecord cr;
    QuantifyEditWindow* editWnd = nullptr;
    explicit QuantifyDisplayWindow(WConfigDocument* doc, QWidget *parent = nullptr);
    ~QuantifyDisplayWindow();
public:
    void refreshData();
    void createCol(int col, const QString& title, const QFont& font, const QColor& color);
    void createRow(int row, const QString& name, const QVector<Record>& rec, float total);
    bool tableTextRank(int logicalIndex, const QString& A, const QString& B);
    void setViewDlg(QuantifyDisplayViewDialog *dlg) { viewDlg = dlg; };
private slots:
    void on_btnExport_clicked();
    void on_CustomSort(int logicalIndex);
    void on_quantifyTable_cellDoubleClicked(int row, int column);
    void on_btnRefresh_clicked();

private:
    QuantifyDisplayViewDialog *viewDlg = nullptr;
    Ui::QuantifyDisplayWindow *ui;
};

#endif // QUANTIFYDISPLAYWINDOW_H
