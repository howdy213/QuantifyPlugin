/**
 * @file quantifydisplaywindow.cpp
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
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextEdit>

#include "QXlsx.h"
#include "WECore/metadata/WMetaDocument.h"
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
/// @brief QuantifyDisplayWindow::refresh
///
void QuantifyDisplayWindow::refresh() {
    refreshData();

    const int weekCount = m_cr->week();
    const auto &ranges = m_cr->getSummaryRanges();
    const int summaryCount = ranges.size();

    // 列数：姓名 + 每周列 + 汇总列 + 总分列
    const int colCount = 1 + weekCount + summaryCount + 1;
    ui->quantifyTable->setColumnCount(colCount);

    // 创建表头
    QFont headerFont;
    headerFont.setPointSize(12);
    headerFont.setFamily("黑体");
    QColor headerColor(0, 120, 240);

    createCol(0, "姓名", headerFont, headerColor);
    int col = 1;
    for (int i = 1; i <= weekCount; ++i) {
        createCol(col++, QString("第%1周").arg(i), headerFont, headerColor);
    }
    for (int idx = 0; idx < summaryCount; ++idx) {
        int start = ranges[idx].first;
        int end   = ranges[idx].second;
        createCol(col++, QString("%1-%2").arg(start).arg(end), headerFont, headerColor);
    }
    createCol(col, "总分", headerFont, headerColor);

    // 设置列宽
    QHeaderView *header = ui->quantifyTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->quantifyTable->setColumnWidth(0, 100);
    for (int i = 1; i < colCount; ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    int currentMode = ui->comboType->currentIndex(); // 0:个人, 1:小组
    bool useAverage = ui->checkAverage->isChecked();

    int rowCount = (currentMode == 0) ? m_cr->students.size() : m_cr->groups.size();
    ui->quantifyTable->setRowCount(rowCount);

    int row = 0;
    if (currentMode == 0) {
        // 个人模式
        for (auto it = m_cr->students.constBegin(); it != m_cr->students.constEnd(); ++it, ++row) {
            const StudentRecord &student = it.value();
            QVector<Record> weekly = student.weekly;
            double total = student.getScore().s3;

            if (useAverage) {
                // 每周分数转为日均
                for (int w = 0; w < weekly.size(); ++w) {
                    weekly[w].s2 /= 7.0;
                }
                if (weekCount > 0) {
                    total /= weekCount;
                }
            }

            QVector<double> summaryValues = computeSummaryValues(weekly);
            // 调用带汇总列的重载
            createRow(row, student.name_ch, weekly, total, summaryValues);
        }
    } else {
        // 小组模式
        for (auto git = m_cr->groups.constBegin(); git != m_cr->groups.constEnd(); ++git, ++row) {
            const GroupRecord &group = git.value();
            int memberCount = 0;
            double groupTotal = 0.0;
            QVector<Record> groupWeekly(weekCount, Record{}); // 每周累计 s2

            for (const QString &member : group.members) {
                auto sit = m_cr->students.constFind(member);
                if (sit == m_cr->students.constEnd()) continue;
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

            QVector<double> summaryValues = computeSummaryValues(groupWeekly);
            createRow(row, group.name_ch, groupWeekly, groupTotal, summaryValues);
        }
    }

    emit recordRefresh();
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
/// \param summaryValues
///
void QuantifyDisplayWindow::createRow(int row, const QString &name,
                                      const QVector<Record> &rec,
                                      float total,
                                      const QVector<double> &summaryValues) {
    // 姓名列
    auto *nameItem = new QTableWidgetItem(name);
    nameItem->setData(Qt::UserRole, row);
    nameItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->quantifyTable->setItem(row, 0, nameItem);

    int col = 1;

    // 每周列
    for (int i = 0; i < rec.size(); ++i) {
        double scaled = qRound(rec[i].s2 * SCORE_SCALE) / SCORE_SCALE;
        auto *cellItem = new QTableWidgetItem(QString::number(scaled));
        cellItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->quantifyTable->setItem(row, col++, cellItem);
    }

    // 汇总列
    for (double val : summaryValues) {
        double scaled = qRound(val * SCORE_SCALE) / SCORE_SCALE;
        auto *cellItem = new QTableWidgetItem(QString::number(scaled));
        cellItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->quantifyTable->setItem(row, col++, cellItem);
    }

    // 总分列
    double totalScaled = qRound(total * SCORE_SCALE) / SCORE_SCALE;
    auto *totalItem = new QTableWidgetItem(QString::number(totalScaled));
    totalItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->quantifyTable->setItem(row, col, totalItem);
}

QVector<double> QuantifyDisplayWindow::computeSummaryValues(const QVector<Record> &weekly) const
{
    QVector<double> values;
    const auto& ranges = m_cr->getSummaryRanges();
    for (const auto& range : ranges) {
        double sum = 0.0;
        int start = range.first - 1;   // 转换为0基索引
        int end   = range.second - 1;
        for (int w = start; w <= end && w < weekly.size(); ++w) {
            sum += weekly[w].s2;
        }
        values.append(sum);
    }
    return values;
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
    refresh();
}
///
/// \brief QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked
/// \param row
/// \param column
void QuantifyDisplayWindow::on_quantifyTable_cellDoubleClicked(int row, int column) {
    int mode = ui->comboType->currentIndex();                 // 0=个人，1=小组
    int weekCount = m_cr->week();
    const auto& ranges = m_cr->getSummaryRanges();
    int totalCols = 1 + weekCount + ranges.size() + 1;        // 姓名 + 每周列 + 汇总列 + 总分列

    // 姓名列：显示组信息或成员列表
    if (column == 0) {
        QString displayName = ui->quantifyTable->item(row, 0)->text();  // 界面显示的中文名或组名
        if (mode == 0) {
            // 个人模式：显示该学生所属的组（中文组名）
            // 1. 根据中文名查找学生英文缩写
            QString studentAbbr;
            for (auto it = m_cr->students.constBegin(); it != m_cr->students.constEnd(); ++it) {
                if (it.value().name_ch == displayName) {
                    studentAbbr = it.key();
                    break;
                }
            }
            if (studentAbbr.isEmpty()) {
                QMessageBox::information(this, tr("组信息"), tr("未找到该学生"));
                return;
            }
            // 2. 获取所属组（排除 ALL）
            QStringList groups;
            for (auto git = m_cr->groups.constBegin(); git != m_cr->groups.constEnd(); ++git) {
                if (git.key() == "ALL") continue;
                if (git.value().members.contains(studentAbbr))
                    groups.append(git.value().name_ch);  // 中文组名
            }
            QString msg = groups.isEmpty() ? tr("该学生未加入任何组") : tr("所属组: ") + groups.join("、");
            QMessageBox::information(this, tr("组信息"), msg);
        } else {
            // 小组模式：显示成员列表
            QString groupKey;
            for (auto git = m_cr->groups.constBegin(); git != m_cr->groups.constEnd(); ++git) {
                if (git.value().name_ch == displayName) {
                    groupKey = git.key();
                    break;
                }
            }
            if (groupKey.isEmpty()) {
                QMessageBox::information(this, tr("成员信息"), tr("未找到该小组"));
                return;
            }
            const GroupRecord &group = m_cr->groups[groupKey];
            QStringList memberChineseNames;
            for (const QString &abbr : group.members) {
                auto sit = m_cr->students.constFind(abbr);
                if (sit != m_cr->students.constEnd())
                    memberChineseNames.append(sit->name_ch);
                else
                    memberChineseNames.append(abbr);  // 降级显示英文
            }
            QString msg = tr("成员列表 (%1 人):\n").arg(memberChineseNames.size()) +
                          memberChineseNames.join("、");
            QMessageBox::information(this, tr("成员信息"), msg);
        }
        return;
    }

    // 收集要显示的记录
    QVector<RecordInfo> displayInfo = collectRecordsForColumn(mode, row, column, weekCount, ranges);
    QString displayName = getDisplayNameForDoubleClick(mode, row);

    if (displayInfo.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("该列没有详细记录。"));
        return;
    }

    // 创建或重用查看对话框
    if (!viewDlg) {
        viewDlg = new QuantifyDisplayViewDialog(this);
        viewDlg->setDialog(this);
    }

    bool isGroupMode = (mode == 1);
    viewDlg->setGroupMode(isGroupMode);
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

void QuantifyDisplayWindow::setClassRecord(ClassRecord *cr) {
    m_cr = cr;
    refresh(); // 重新加载数据并刷新表格
}

QStringList QuantifyDisplayWindow::getGroupsOfStudent(const QString &studentAbbr) const {
    QStringList groups;
    for (auto it = m_cr->groups.constBegin(); it != m_cr->groups.constEnd(); ++it) {
        if (it.value().members.contains(studentAbbr))
            groups.append(it.value().name_ch);
    }
    return groups;
}

void QuantifyDisplayWindow::on_btnModifySummaryFile_clicked()
{
    if (!m_doc) {
        QMessageBox::warning(this, tr("错误"), tr("配置未加载"));
        return;
    }

    QString recordDir = Quantify::resolvePathWithKey(m_doc, Quantify::Consts::DirPath) + "/" + Quantify::Consts::DirRecord;
    QString summaryPath = QDir(recordDir).filePath("summary.txt");

    // 读取现有内容
    QString content;
    QFile file(summaryPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        content = in.readAll();
        file.close();
    } else {
        // 文件不存在，则内容为空，稍后保存时会创建
        content = "";
    }

    // 创建编辑对话框
    QDialog dlg(this);
    dlg.setWindowTitle(tr("编辑周段汇总文件 - summary.txt"));
    dlg.resize(500, 400);

    QTextEdit *textEdit = new QTextEdit(&dlg);
    textEdit->setPlainText(content);
    textEdit->setFont(QFont("Consolas", 10));

    QPushButton *btnSave = new QPushButton(tr("保存"), &dlg);
    QPushButton *btnCancel = new QPushButton(tr("取消"), &dlg);
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnSave);
    buttonLayout->addWidget(btnCancel);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->addWidget(textEdit);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(btnSave, &QPushButton::clicked, &dlg, [&]() {
        QString newContent = textEdit->toPlainText();
        QFile outFile(summaryPath);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(&dlg, tr("错误"), tr("无法写入文件: %1").arg(summaryPath));
            return;
        }
        QTextStream out(&outFile);
        out << newContent;
        outFile.close();
        dlg.accept();
    });
    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        // 刷新显示，重新加载汇总范围并刷新表格
        refresh();
        QMessageBox::information(this, tr("成功"), tr("汇总文件已更新，表格已刷新。"));
    }
}
QString QuantifyDisplayWindow::getDisplayNameForDoubleClick(int mode, int row) const {
    // 直接获取表格中显示的名称即可
    return ui->quantifyTable->item(row, 0)->text();
}

QVector<RecordInfo> QuantifyDisplayWindow::collectRecordsForWeekOrTotal(int mode, int row, int column, int weekCount) const {
    QVector<RecordInfo> result;
    bool isTotalColumn = (column == weekCount + 1 + m_cr->getSummaryRanges().size()); // 总分列索引

    if (mode == 0) { // 个人模式
        QString name = ui->quantifyTable->item(row, 0)->text();
        auto it = std::find_if(m_cr->students.constBegin(), m_cr->students.constEnd(),
                               [&name](const StudentRecord& sr) { return sr.name_ch == name; });
        if (it == m_cr->students.constEnd()) return result;
        if (isTotalColumn) {
            result = it->info;
        } else {
            int weekIdx = column - 1; // 普通周列（从0基索引）
            result = it->getWeeklyInfo(weekIdx);
        }
    } else { // 小组模式
        QString groupName = ui->quantifyTable->item(row, 0)->text();
        auto git = std::find_if(m_cr->groups.constBegin(), m_cr->groups.constEnd(),
                                [&groupName](const GroupRecord& gr) { return gr.name_ch == groupName; });
        if (git == m_cr->groups.constEnd()) return result;
        for (const QString& member : git->members) {
            auto sit = m_cr->students.constFind(member);
            if (sit == m_cr->students.constEnd()) continue;
            QVector<RecordInfo> memberInfo;
            if (isTotalColumn) {
                memberInfo = sit->info;
            } else {
                int weekIdx = column - 1;
                memberInfo = sit->getWeeklyInfo(weekIdx);
            }
            for (RecordInfo& info : memberInfo) {
                info.memberName = sit->name_ch; // 标记成员姓名
                result.append(info);
            }
        }
    }
    return result;
}

QVector<RecordInfo> QuantifyDisplayWindow::collectRecordsForSummary(int mode, int row, int summaryIndex,
                                                                    const QVector<QPair<int,int>>& ranges) const {
    QVector<RecordInfo> result;
    if (summaryIndex < 0 || summaryIndex >= ranges.size()) return result;
    int startWeek = ranges[summaryIndex].first -1;
    int endWeek   = ranges[summaryIndex].second -1;

    if (mode == 0) { // 个人模式
        QString name = ui->quantifyTable->item(row, 0)->text();
        auto it = std::find_if(m_cr->students.constBegin(), m_cr->students.constEnd(),
                               [&name](const StudentRecord& sr) { return sr.name_ch == name; });
        if (it == m_cr->students.constEnd()) return result;
        for (const RecordInfo& info : it->info) {
            if (info.week >= startWeek && info.week <= endWeek) {
                result.append(info);
            }
        }
    } else { // 小组模式
        QString groupName = ui->quantifyTable->item(row, 0)->text();
        auto git = std::find_if(m_cr->groups.constBegin(), m_cr->groups.constEnd(),
                                [&groupName](const GroupRecord& gr) { return gr.name_ch == groupName; });
        if (git == m_cr->groups.constEnd()) return result;
        for (const QString& member : git->members) {
            auto sit = m_cr->students.constFind(member);
            if (sit == m_cr->students.constEnd()) continue;
            for (const RecordInfo& info : sit->info) {
                if (info.week >= startWeek && info.week <= endWeek) {
                    RecordInfo copy = info;
                    copy.memberName = sit->name_ch;
                    result.append(copy);
                }
            }
        }
    }
    return result;
}

QVector<RecordInfo> QuantifyDisplayWindow::collectRecordsForColumn(int mode, int row, int column,
                                                                   int weekCount, const QVector<QPair<int,int>>& ranges) const {
    int totalCols = 1 + weekCount + ranges.size() + 1; // 姓名 + 每周 + 汇总 + 总分
    if (column == 0) return {}; // 姓名列由调用方单独处理
    if (column == totalCols - 1) {
        // 总分列
        return collectRecordsForWeekOrTotal(mode, row, column, weekCount);
    }
    // 判断是普通周列还是汇总列
    if (column >= 1 && column <= weekCount) {
        // 普通周列
        return collectRecordsForWeekOrTotal(mode, row, column, weekCount);
    } else if (column > weekCount && column < totalCols - 1) {
        // 汇总列
        int summaryIndex = column - weekCount - 1;
        return collectRecordsForSummary(mode, row, summaryIndex, ranges);
    }
    return {};
}