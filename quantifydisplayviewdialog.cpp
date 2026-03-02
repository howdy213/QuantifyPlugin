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
#include "quantifydisplayviewdialog.h"
#include "QXlsx.h"
#include "quantifydisplaywindow.h"
#include "ui_quantifydisplayviewdialog.h"

class QuantifyDisplayViewDialogPrivate {
public:
    QuantifyDisplayViewDialogPrivate() = default;
    Ui::QuantifyDisplayViewDialog *ui;
    QVector<RecordInfo> m_info;
    QString m_name;
    QuantifyDisplayWindow *m_dlg = nullptr;
};
///
/// \brief QuantifyDisplayViewDialog::QuantifyDisplayViewDialog
/// \param parent
///
QuantifyDisplayViewDialog::QuantifyDisplayViewDialog(QWidget *parent)
    : QDialog(parent), d_ptr(new QuantifyDisplayViewDialogPrivate) {
    Q_D(QuantifyDisplayViewDialog);
    d->ui = new Ui::QuantifyDisplayViewDialog;
    d->ui->setupUi(this);
    d->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    QFont font;
    font.setPointSize(12);
    font.setFamily("黑体");

    setMinimumHeight(450);
    setMinimumWidth(750);

    QColor color(0, 120, 240);
    const int colCount = 4;
    d->ui->tableWidget->setColumnCount(colCount);

    createCol(0, "日期", font, color);
    createCol(1, "分数", font, color);
    createCol(2, "原因", font, color);
    createCol(3, "备注", font, color);
}
///
/// \brief QuantifyDisplayViewDialog::~QuantifyDisplayViewDialog
///
QuantifyDisplayViewDialog::~QuantifyDisplayViewDialog() {

    Q_D(QuantifyDisplayViewDialog);
    delete d->ui;
}
///
/// \brief QuantifyDisplayViewDialog::createCol
/// \param col
/// \param title
/// \param font
/// \param color
///
void QuantifyDisplayViewDialog::createCol(int col, const QString &title,
                                          const QFont &font,
                                          const QColor &color) {
    Q_D(QuantifyDisplayViewDialog);
    auto *item = new QTableWidgetItem(title);
    item->setFont(font);
    item->setForeground(QBrush(color));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->ui->tableWidget->setHorizontalHeaderItem(col, item);
}
///
/// \brief QuantifyDisplayViewDialog::createRow
/// \param row
/// \param info
///
void QuantifyDisplayViewDialog::createRow(int row, const RecordInfo &info) {
    Q_D(QuantifyDisplayViewDialog);
    auto createItem = [](const QString &text) {
        auto *item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        return item;
    };

    d->ui->tableWidget->setItem(row, 0, createItem(info.date));
    d->ui->tableWidget->setItem(row, 1, createItem(QString::number(info.delta)));
    d->ui->tableWidget->setItem(row, 2, createItem(info.reason));
    d->ui->tableWidget->setItem(row, 3, createItem(info.note));
}
///
/// \brief QuantifyDisplayViewDialog::setName
/// \param name
///
void QuantifyDisplayViewDialog::setName(const QString &name) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_name = name;
    d->ui->labelNameDisplay->setText(name);
}
///
/// \brief QuantifyDisplayViewDialog::getName
/// \return
///
QString QuantifyDisplayViewDialog::getName() const {
    const Q_D(QuantifyDisplayViewDialog);
    return d->m_name;
}
///
/// \brief QuantifyDisplayViewDialog::setDialog
/// \param dlg
///
void QuantifyDisplayViewDialog::setDialog(QuantifyDisplayWindow *dlg) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_dlg = dlg;
}
///
/// \brief QuantifyDisplayViewDialog::setContent
/// \param info
///
void QuantifyDisplayViewDialog::setContent(const QVector<RecordInfo> &info) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_info = info;

    const int rows = info.size();
    d->ui->tableWidget->setRowCount(rows);

    QHeaderView *header = d->ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    d->ui->tableWidget->setColumnWidth(0, 100);
    for (int i = 1; i < d->ui->tableWidget->columnCount(); ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    for (int row = 0; row < rows; ++row) {
        createRow(row, info[row]);
    }
}
///
/// \brief QuantifyDisplayViewDialog::closeEvent
/// \param event
///
void QuantifyDisplayViewDialog::closeEvent(QCloseEvent *event) {
    Q_D(QuantifyDisplayViewDialog);
    if (d->m_dlg) {
        d->m_dlg->setViewDlg(nullptr);
    }
    event->accept();
    deleteLater();
}
///
/// \brief QuantifyDisplayViewDialog::on_btnExport_clicked
///
void QuantifyDisplayViewDialog::on_btnExport_clicked() {
    Q_D(QuantifyDisplayViewDialog);
    QString desktopPath =
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString savePath = QFileDialog::getSaveFileName(
        nullptr, "保存文件",
        desktopPath + "/" + d->ui->labelNameDisplay->text() + ".xlsx",
        "Table Files (*.xlsx)");
    if (savePath.isEmpty())
        return;

    QXlsx::Document xlsx;
    const int rowCount = d->ui->tableWidget->rowCount();
    const int colCount = d->ui->tableWidget->columnCount();

    for (int row = 1; row <= rowCount; ++row) {
        for (int col = 1; col <= colCount; ++col) {
            QTableWidgetItem *item = d->ui->tableWidget->item(row - 1, col - 1);
            if (!item)
                continue;
            bool ok = false;
            double num = item->text().toDouble(&ok);
            if (ok)
                xlsx.write(row, col, num);
            else
                xlsx.write(row, col, item->text());
        }
    }
    xlsx.saveAs(savePath);
}
///
/// \brief QuantifyDisplayViewDialog::on_btnFilter_clicked
///
void QuantifyDisplayViewDialog::on_btnFilter_clicked() {
    Q_D(QuantifyDisplayViewDialog);
    d->ui->tableWidget->clearContents();
    QString filterText = d->ui->editFilter->text();
    if (filterText.isEmpty()) {
        setContent(d->m_info);
        return;
    }

    QStringList filters = filterText.split('|', Qt::SkipEmptyParts);
    auto matches = [&filters](const RecordInfo &info) {
        foreach (const QString &f, filters) {
            if (info.note.contains(f) || info.reason.contains(f))
                return true;
        }
        return false;
    };

    // 计算匹配行数
    int matchedRows = 0;
    foreach (const RecordInfo &info, d->m_info) {
        if (matches(info))
            ++matchedRows;
    }

    d->ui->tableWidget->setRowCount(matchedRows + 1);
    QHeaderView *header = d->ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    d->ui->tableWidget->setColumnWidth(0, 100);
    for (int i = 1; i < d->ui->tableWidget->columnCount(); ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    int row = 0;
    double total = 0;
    QString date;
    foreach (const RecordInfo &info, d->m_info) {
        if (matches(info)) {
            createRow(row, info);
            ++row;
            total += info.delta;
            date = info.date;
        }
    }
    RecordInfo info;
    info.reason = "总计";
    info.date = date;
    info.delta = total;
    createRow(row, info);
}
