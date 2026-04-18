/**
 * @file classrecord.h
 * @brief 班级记录类
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
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QStandardPaths>

#include "QXlsx.h"
#include "WECore/WConfig/wconfigdocument.h"
#include "quantify.h"
#include "quantifydisplaywindow.h"
#include "quantifydisplayviewdialog.h"
#include "quantifyeditwindow.h"
#include "ui_quantifydisplaywindow.h"

const double SCORE_SCALE = 10000.0;
///
/// \brief QuantifyDisplayWindow::QuantifyDisplayWindow
/// \param doc
/// \param parent
///
QuantifyDisplayWindow::QuantifyDisplayWindow(QWidget *parent)
    : QWidget(parent),ui(new Ui::QuantifyDisplayWindow) {
    ui->setupUi(this);
    ui->quantifyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->quantifyTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->quantifyTable->setSelectionMode(QAbstractItemView::NoSelection);
    connect(ui->quantifyTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &QuantifyDisplayWindow::on_CustomSort);
}
///
/// \brief QuantifyDisplayWindow::initialize
/// \param components
/// \param ui
///
void QuantifyDisplayWindow::initialize(const Quantify::QuantifyComponents& components,
                                       const Quantify::QuantifyUI& ui)
{
    m_cr = components.classRecord;
    m_doc = components.config;
    m_editWnd = ui.editWindow;
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
        return m_cr->index("", A) > m_cr->index("", B);
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
void QuantifyDisplayWindow::refreshData() { m_cr->refresh(); }
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

    const int weekCount = m_cr->week();
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

    int currentMode = ui->comboType->currentIndex(); // 0:个人, 1:小组
    int rowCount = (currentMode == 0) ? m_cr->students.size() : m_cr->groups.size();
    ui->quantifyTable->setRowCount(rowCount);

    QHeaderView *header = ui->quantifyTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->quantifyTable->setColumnWidth(0, 100);
    for (int i = 1; i < colCount; ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    int row = 0;
    bool useAverage = ui->checkAverage->isChecked(); // 读取复选框状态
    if (currentMode == 0) {
        // 个人模式
        for (auto it = m_cr->students.constBegin(); it != m_cr->students.constEnd();
             ++it, ++row) {
            const StudentRecord &student = it.value();
            if (!useAverage) {
                // 原始分数
                createRow(row, student.name_ch, student.weekly, student.getScore().s3);
            } else {
                // 平均分：每周除以7，总分除以周数
                QVector<Record> avgWeekly = student.weekly;
                for (int w = 0; w < avgWeekly.size(); ++w) {
                    avgWeekly[w].s2 /= 7.0; // 周总分 -> 日均
                }
                double avgTotal =
                    (weekCount > 0) ? (student.getScore().s3 / weekCount) : 0.0;
                createRow(row, student.name_ch, avgWeekly, avgTotal);
            }
        }
    } else {
        // 小组模式

        for (auto git = m_cr->groups.constBegin(); git != m_cr->groups.constEnd();
             ++git, ++row) {
            const GroupRecord &group = git.value();
            double groupTotal = 0.0;
            int memberCount = 0;
            QVector<Record> groupWeekly(weekCount, Record{}); // 每周记录，仅 s2 有效

            for (const QString &member : group.members) {
                auto sit = m_cr->students.constFind(member);
                if (sit == m_cr->students.constEnd())
                    continue;
                const StudentRecord &student = sit.value();
                groupTotal += student.getScore().s3;
                memberCount++;
                for (int w = 0; w < weekCount && w < student.weekly.size(); ++w) {
                    groupWeekly[w].s2 += student.weekly[w].s2;
                }
            }

            if (useAverage && memberCount > 0) {
                groupTotal /= memberCount;
                for (int w = 0; w < weekCount; ++w) {
                    groupWeekly[w].s2 /= memberCount;
                }
            }

            createRow(row, group.name_ch, groupWeekly, groupTotal);
        }
    }
    emit recordRefresh();
}
///
/// \brief QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked
/// \param row
/// \param column
///// quantifydisplaywindow.cpp
void QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked(int row,
                                                               int column) {
    if (column == 0)
        return; // 姓名列不处理

    // 获取当前模式：0=个人，1=小组
    int mode = ui->comboType->currentIndex();

    // 准备要显示的记录列表
    QVector<RecordInfo> displayInfo;
    QString displayName;
    bool isGroupMode = (mode == 1);

    if (mode == 0) {
        // 个人模式：查找对应学生
        QString name = ui->quantifyTable->item(row, 0)->text();
        auto it = std::find_if(
            m_cr->students.constBegin(), m_cr->students.constEnd(),
            [&name](const StudentRecord &sr) { return sr.name_ch == name; });
        if (it == m_cr->students.constEnd())
            return;

        displayName = it->name_ch;
        if (column == ui->quantifyTable->columnCount() - 1) {
            displayInfo = it->info; // 所有记录
        } else {
            displayInfo = it->getWeeklyInfo(column - 1); // 某周记录
        }
    } else {
        // 小组模式：查找对应小组
        QString groupName = ui->quantifyTable->item(row, 0)->text();
        auto git = std::find_if(m_cr->groups.constBegin(), m_cr->groups.constEnd(),
                                [&groupName](const GroupRecord &gr) {
                                    return gr.name_ch == groupName;
        });
        if (git == m_cr->groups.constEnd())
            return;

        displayName = git->name_ch;
        // 收集该小组所有成员的记录，并为每条记录添加 memberName
        for (const QString &member : git->members) {
            auto sit = m_cr->students.constFind(member);
            if (sit == m_cr->students.constEnd())
                continue;
            QVector<RecordInfo> memberInfo;
            if (column == ui->quantifyTable->columnCount() - 1) {
                memberInfo = sit->info; // 所有记录
            } else {
                memberInfo = sit->getWeeklyInfo(column - 1); // 某周记录
            }
            for (RecordInfo &info : memberInfo) {
                info.memberName = sit->name_ch; // 标记成员姓名
                displayInfo.append(info);
            }
        }
    }

    if (displayInfo.isEmpty())
        return;

    // 创建或重用查看对话框
    if (!viewDlg) {
        viewDlg = new QuantifyDisplayViewDialog(this);
        viewDlg->setDialog(this);
    }

    viewDlg->setGroupMode(isGroupMode); // 设置是否为小组模式
    viewDlg->setContent(displayInfo);
    viewDlg->setName(displayName);

    if (viewDlg->isHidden())
        viewDlg->show();
    else
        viewDlg->activateWindow();
}
///
/// \brief QuantifyDisplayWindow::on_comboType_currentIndexChanged
/// \param index
///
void QuantifyDisplayWindow::on_comboType_currentIndexChanged(int index) {
    on_btnRefresh_clicked();
}

void QuantifyDisplayWindow::on_checkAverage_stateChanged(int arg1) {
    on_btnRefresh_clicked();
}
