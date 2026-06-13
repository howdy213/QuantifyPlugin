/**
 * @file quantifydisplaywindow.h
 * @brief 展示窗口
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
#ifndef QUANTIFYDISPLAYWINDOW_H
#define QUANTIFYDISPLAYWINDOW_H

#include "classrecord.h"
#include "quantify.h"
#include <QWidget>

class QuantifyDisplayViewDialog;

namespace Ui {
class QuantifyDisplayWindow;
}

namespace Quantify {
class QuantifyComponents;
class QuantifyUI;
} // namespace Quantify

class QuantifyDisplayWindow : public QWidget {
    Q_OBJECT
public:
    explicit QuantifyDisplayWindow(QWidget *parent = nullptr);
    ~QuantifyDisplayWindow();

    void initialize(const Quantify::QuantifyComponents &components,
                    const Quantify::QuantifyUI &ui);

    void refreshData();
    void createCol(int col, const QString &title, const QFont &font,
                   const QColor &color);
    void createRow(int row, const QString &name, const QVector<Record> &rec,
                   float total, const QVector<double> &summaryValues = {});
    QVector<double> computeSummaryValues(const QVector<Record> &weekly) const;
    bool tableTextRank(int logicalIndex, const QString &A, const QString &B);
    void setViewDlg(QuantifyDisplayViewDialog *dlg) { viewDlg = dlg; }
    void setClassRecord(ClassRecord *cr);
signals:
    void recordRefresh();

public slots:
    void refresh();
private slots:
    void on_btnExport_clicked();
    void on_CustomSort(int logicalIndex);
    void on_quantifyTable_cellDoubleClicked(int row, int column);
    void on_btnRefresh_clicked();
    void on_comboType_currentIndexChanged(int index);
    void on_checkAverage_stateChanged(int arg1);

    void on_btnModifySummaryFile_clicked();

private:
    QString getDisplayNameForDoubleClick(int mode, int row) const;
    QVector<RecordInfo>
    collectRecordsForColumn(int mode, int row, int column, int weekCount,
                            const QVector<QPair<int, int>> &ranges) const;
    QVector<RecordInfo>
    collectRecordsForSummary(int mode, int row, int summaryIndex,
                             const QVector<QPair<int, int>> &ranges) const;
    QVector<RecordInfo> collectRecordsForWeekOrTotal(int mode, int row,
                                                     int column,
                                                     int weekCount) const;

private:
    QStringList getGroupsOfStudent(const QString &studentAbbr) const;

private:
    Ui::QuantifyDisplayWindow *ui;
    QuantifyDisplayViewDialog *viewDlg = nullptr;
    ClassRecord *m_cr = nullptr;
    we::WMetaDocument *m_doc = nullptr;
    QuantifyEditWindow *m_editWnd = nullptr;
};
#endif // QUANTIFYDISPLAYWINDOW_H