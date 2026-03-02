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
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QStandardPaths>

#include "QXlsx.h"
#include "WECore/WConfig/wconfigdocument.h"
#include "quantifydisplaywindow.h"
#include "ui_quantifydisplaywindow.h"

const double SCORE_SCALE = 10000.0;
///
/// \brief QuantifyDisplayWindow::QuantifyDisplayWindow
/// \param doc
/// \param parent
///
QuantifyDisplayWindow::QuantifyDisplayWindow(WConfigDocument *doc,
                                             QWidget *parent)
    : QWidget(parent),
    cr(qvariant_cast<QString>(doc->get("path")),
         doc->get("engine").toString() == "js" ? RuleEngine::JS
                                               : RuleEngine::Native),
    editWnd(nullptr), viewDlg(nullptr), ui(new Ui::QuantifyDisplayWindow) {
    ui->setupUi(this);
    ui->quantifyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->quantifyTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->quantifyTable->setSelectionMode(QAbstractItemView::NoSelection);
    connect(ui->quantifyTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &QuantifyDisplayWindow::on_CustomSort);
}
///
/// \brief QuantifyDisplayWindow::~QuantifyDisplayWindow
///
QuantifyDisplayWindow::~QuantifyDisplayWindow() { delete ui; }
///
/// \brief QuantifyDisplayWindow::tableTextRank
/// \param logicalIndex
/// \param A
/// \param B
/// \return
///
bool QuantifyDisplayWindow::tableTextRank(int logicalIndex, const QString &A,
                                          const QString &B) {
    if (logicalIndex == 0) {
        return cr.index("", A) > cr.index("", B);
    }
    bool ok1, ok2;
    float valA = A.toFloat(&ok1);
    float valB = B.toFloat(&ok2);
    if (ok1 && ok2) {
        return valA > valB;
    }
    return A > B;
}
///
/// \brief QuantifyDisplayWindow::on_CustomSort
/// \param logicalIndex
///
void QuantifyDisplayWindow::on_CustomSort(int logicalIndex) {
    const int rowCount = ui->quantifyTable->rowCount();
    const int colCount = ui->quantifyTable->columnCount();

    for (int i = 0; i < rowCount - 1; ++i) {
        for (int j = 0; j < rowCount - i - 1; ++j) {
            QString textJ = ui->quantifyTable->item(j, logicalIndex)->text();
            QString textJ1 = ui->quantifyTable->item(j + 1, logicalIndex)->text();
            if (tableTextRank(logicalIndex, textJ, textJ1)) {
                for (int k = 0; k < colCount; ++k) {
                    QTableWidgetItem *item1 = ui->quantifyTable->takeItem(j, k);
                    QTableWidgetItem *item2 = ui->quantifyTable->takeItem(j + 1, k);
                    ui->quantifyTable->setItem(j, k, item2);
                    ui->quantifyTable->setItem(j + 1, k, item1);
                }
            }
        }
    }
}
///
/// \brief QuantifyDisplayWindow::refreshData
///
void QuantifyDisplayWindow::refreshData() { cr.refresh(); }
///
/// \brief QuantifyDisplayWindow::createCol
/// \param col
/// \param title
/// \param font
/// \param color
///
void QuantifyDisplayWindow::createCol(int col, const QString &title,
                                      const QFont &font, const QColor &color) {
    auto *item = new QTableWidgetItem(title);
    item->setFont(font);
    item->setForeground(QBrush(color));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->quantifyTable->setHorizontalHeaderItem(col, item);
}
///
/// \brief QuantifyDisplayWindow::createRow
/// \param row
/// \param name
/// \param rec
/// \param total
///
void QuantifyDisplayWindow::createRow(int row, const QString &name,
                                      const QVector<Record> &rec, float total) {
    auto *item = new QTableWidgetItem(name);
    item->setData(Qt::UserRole, QVariant(row));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->quantifyTable->setItem(row, 0, item);

    int count = 0;
    for (auto it = rec.begin(); it != rec.end(); ++it, ++count) {
        double scaled = qRound(it->s2 * SCORE_SCALE) / SCORE_SCALE;
        auto *cellItem = new QTableWidgetItem(QString::number(scaled));
        cellItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->quantifyTable->setItem(row, count + 1, cellItem);
    }

    double totalScaled = qRound(total * SCORE_SCALE) / SCORE_SCALE;
    auto *totalItem = new QTableWidgetItem(QString::number(totalScaled));
    totalItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->quantifyTable->setItem(row, count + 1, totalItem);
}
///
/// \brief QuantifyDisplayWindow::on_btnExport_clicked
///
void QuantifyDisplayWindow::on_btnExport_clicked() {
    QString desktopPath =
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString savePath = QFileDialog::getSaveFileName(nullptr, "保存文件",
                                                    desktopPath + "/result.xlsx",
                                                    "Table Files (*.xlsx)");
    if (savePath.isEmpty())
        return;

    QXlsx::Document xlsx;
    const int rowCount = ui->quantifyTable->rowCount();
    const int colCount = ui->quantifyTable->columnCount();

    for (int row = 1; row <= rowCount; ++row) {
        for (int col = 1; col <= colCount; ++col) {
            QTableWidgetItem *cell = ui->quantifyTable->item(row - 1, col - 1);
            if (!cell)
                continue;

            bool ok = false;
            float num = cell->text().toFloat(&ok);
            if (ok)
                xlsx.write(row, col, num);
            else
                xlsx.write(row, col, cell->text());
        }
    }
    xlsx.saveAs(savePath);
}
///
/// \brief QuantifyDisplayWindow::on_btnRefresh_clicked
///
void QuantifyDisplayWindow::on_btnRefresh_clicked() {
    refreshData();

    const int weekCount = cr.week();
    const int colCount = weekCount + 2; // 姓名 + 每周 + 总分
    ui->quantifyTable->setColumnCount(colCount);

    QFont headerFont;
    headerFont.setPointSize(12);
    headerFont.setFamily("黑体");
    QColor headerColor(0, 120, 240);

    createCol(0, "姓名", headerFont, headerColor);
    for (int i = 1; i <= weekCount; ++i) {
        createCol(i, QString("第%1周").arg(i), headerFont, headerColor);
    }
    createCol(colCount - 1, "总分", headerFont, headerColor);

    const int rowCount = cr.students.size();
    ui->quantifyTable->setRowCount(rowCount);

    QHeaderView *header = ui->quantifyTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->quantifyTable->setColumnWidth(0, 100);
    for (int i = 1; i < colCount; ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    int row = 0;
    for (auto it = cr.students.constBegin(); it != cr.students.constEnd();
         ++it, ++row) {
        createRow(row, it.value().name_ch, it->weekly, it.value().getScore().s3);
    }

    if (editWnd) {
        editWnd->cr = &cr;
    }
}
///
/// \brief QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked
/// \param row
/// \param column
///
void QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked(int row,
                                                               int column) {
    // 查找对应的学生记录
    QString name = ui->quantifyTable->item(row, 0)->text();
    auto it = std::find_if(
        cr.students.constBegin(), cr.students.constEnd(),
        [&name](const StudentRecord &sr) { return sr.name_ch == name; });
    if (it == cr.students.constEnd())
        return;

    if (column == 0)
        return;

    if (!viewDlg) {
        viewDlg = new QuantifyDisplayViewDialog(this);
        viewDlg->setDialog(this);
    }

    if (column == ui->quantifyTable->columnCount() - 1) {
        viewDlg->setContent(it->info);
    } else {
        viewDlg->setContent(it->getWeeklyInfo(column - 1));
    }
    viewDlg->setName(it->name_ch);

    if (viewDlg->isHidden())
        viewDlg->show();
    else
        viewDlg->activateWindow();
}
