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
    QVector<RecordInfo> m_originalInfo;  // 保存原始所有记录
    QString m_filterText;                 // 当前过滤文本
    DisplayMode m_displayMode = Expand;   // 当前显示模式
    bool m_isGroupMode = false;
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

    QString firstColText = info.date;
    if (!info.memberName.isEmpty()) {
        firstColText = info.date + " " + info.memberName;
    }

    d->ui->tableWidget->setItem(row, 0, createItem(firstColText));
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
/// \brief QuantifyDisplayViewDialog::updateDisplay
///
void QuantifyDisplayViewDialog::updateDisplay()
{
    Q_D(QuantifyDisplayViewDialog);

    // 1. 获取过滤文本
    d->m_filterText = d->ui->editFilter->text();

    // 2. 过滤原始数据
    QVector<RecordInfo> filtered = d->m_originalInfo;
    if (!d->m_filterText.isEmpty()) {
        QStringList keywords = d->m_filterText.split('|', Qt::SkipEmptyParts);
        filtered.clear();
        for (const RecordInfo &info : d->m_originalInfo) {
            bool match = false;
            for (const QString &kw : keywords) {
                if (info.note.contains(kw, Qt::CaseInsensitive) ||
                    info.reason.contains(kw, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            if (match) filtered.append(info);
        }
    }

    // 3. 清空表格
    d->ui->tableWidget->clearContents();
    d->ui->tableWidget->setRowCount(0);

    // 4. 根据显示模式填充数据
    if (d->m_displayMode == Expand) {
        // 展开模式：每条记录一行
        d->ui->tableWidget->setRowCount(filtered.size());
        for (int i = 0; i < filtered.size(); ++i)
            createRow(i, filtered[i]);
    }
    else if (d->m_displayMode == Summarize) {
        // 归纳模式：按 reason 分组
        QMap<QString, QVector<RecordInfo>> groups;
        for (const RecordInfo &info : filtered)
            groups[info.reason].append(info);
        d->ui->tableWidget->setRowCount(groups.size());
        int row = 0;
        for (auto it = groups.begin(); it != groups.end(); ++it, ++row) {
            double total = 0;
            for (const RecordInfo &info : it.value()) total += info.delta;
            // 创建分组行：日期列留空或显示“合计”，分数列显示总分，原因列显示规则名，备注列显示记录条数
            QTableWidgetItem *dateItem = new QTableWidgetItem("合计");
            dateItem->setTextAlignment(Qt::AlignCenter);
            d->ui->tableWidget->setItem(row, 0, dateItem);

            QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(total));
            scoreItem->setTextAlignment(Qt::AlignCenter);
            d->ui->tableWidget->setItem(row, 1, scoreItem);

            QTableWidgetItem *reasonItem = new QTableWidgetItem(it.key());
            reasonItem->setTextAlignment(Qt::AlignCenter);
            d->ui->tableWidget->setItem(row, 2, reasonItem);

            QTableWidgetItem *noteItem = new QTableWidgetItem(QString("共 %1 条").arg(it.value().size()));
            noteItem->setTextAlignment(Qt::AlignCenter);
            d->ui->tableWidget->setItem(row, 3, noteItem);
        }
    }
    else if (d->m_displayMode == Collapse) {
        // 折叠模式：根据小组模式决定分组依据
        if (d->m_isGroupMode) {
            // 小组模式：按 memberName 分组
            QMap<QString, QVector<RecordInfo>> groups;
            for (const RecordInfo &info : filtered)
                groups[info.memberName].append(info);
            d->ui->tableWidget->setRowCount(groups.size());
            int row = 0;
            for (auto it = groups.begin(); it != groups.end(); ++it, ++row) {
                double total = 0;
                for (const RecordInfo &info : it.value()) total += info.delta;
                QTableWidgetItem *memberItem = new QTableWidgetItem(it.key());
                memberItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 0, memberItem);

                QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(total));
                scoreItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 1, scoreItem);

                QTableWidgetItem *reasonItem = new QTableWidgetItem("成员合计");
                reasonItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 2, reasonItem);

                QTableWidgetItem *noteItem = new QTableWidgetItem(QString("共 %1 条").arg(it.value().size()));
                noteItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 3, noteItem);
            }
        } else {
            // 个人模式：按 date 分组
            QMap<QString, QVector<RecordInfo>> groups;
            for (const RecordInfo &info : filtered)
                groups[info.date].append(info);
            d->ui->tableWidget->setRowCount(groups.size());
            int row = 0;
            for (auto it = groups.begin(); it != groups.end(); ++it, ++row) {
                double total = 0;
                for (const RecordInfo &info : it.value()) total += info.delta;
                QTableWidgetItem *dateItem = new QTableWidgetItem(it.key());
                dateItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 0, dateItem);

                QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(total));
                scoreItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 1, scoreItem);

                QTableWidgetItem *reasonItem = new QTableWidgetItem("本日合计");
                reasonItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 2, reasonItem);

                QTableWidgetItem *noteItem = new QTableWidgetItem(QString("共 %1 条").arg(it.value().size()));
                noteItem->setTextAlignment(Qt::AlignCenter);
                d->ui->tableWidget->setItem(row, 3, noteItem);
            }
        }
    }

    // 5. 添加总计行
    int lastRow = d->ui->tableWidget->rowCount();
    d->ui->tableWidget->setRowCount(lastRow + 1);

    double totalDelta = 0;
    QString lastDateOrMember;
    for (const RecordInfo &info : filtered) {
        totalDelta += info.delta;
        lastDateOrMember = d->m_isGroupMode ? info.memberName : info.date;
    }

    QTableWidgetItem *totalFirstItem = new QTableWidgetItem("   ");
    totalFirstItem->setTextAlignment(Qt::AlignCenter);
    d->ui->tableWidget->setItem(lastRow, 0, totalFirstItem);

    QTableWidgetItem *totalScoreItem = new QTableWidgetItem(QString::number(totalDelta));
    totalScoreItem->setTextAlignment(Qt::AlignCenter);
    d->ui->tableWidget->setItem(lastRow, 1, totalScoreItem);

    QTableWidgetItem *totalReasonItem = new QTableWidgetItem("总计");
    totalReasonItem->setTextAlignment(Qt::AlignCenter);
    d->ui->tableWidget->setItem(lastRow, 2, totalReasonItem);

    QTableWidgetItem *totalNoteItem = new QTableWidgetItem("");
    totalNoteItem->setTextAlignment(Qt::AlignCenter);
    d->ui->tableWidget->setItem(lastRow, 3, totalNoteItem);

    // 6. 调整列标题和列宽
    // 根据模式动态修改第一列标题
    if (d->m_isGroupMode && d->m_displayMode == Collapse) {
        d->ui->tableWidget->horizontalHeaderItem(0)->setText("成员");
    } else {
        d->ui->tableWidget->horizontalHeaderItem(0)->setText("日期");
    }

    QHeaderView *header = d->ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < d->ui->tableWidget->columnCount(); ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }
}
///
/// \brief QuantifyDisplayViewDialog::setGroupMode
/// \param isGroup
///
void QuantifyDisplayViewDialog::setGroupMode(bool isGroup) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_isGroupMode = isGroup;
}
///
/// \brief QuantifyDisplayViewDialog::setContent
/// \param info
///
void QuantifyDisplayViewDialog::setContent(const QVector<RecordInfo> &info) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_originalInfo = info;
    d->m_filterText.clear();                // 重置过滤
    d->ui->editFilter->clear();          // 清空过滤输入框
    updateDisplay();
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
    updateDisplay();
}

void QuantifyDisplayViewDialog::on_comboDisplayMode_currentIndexChanged(int index) {
    Q_D(QuantifyDisplayViewDialog);
    d->m_displayMode = static_cast<DisplayMode>(index);
    updateDisplay();
}
