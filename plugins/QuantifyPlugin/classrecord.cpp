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
#include "classrecord.h"
#include "quantify.h"
#include "QXlsx.h"
#include "logger.h"

using namespace QXlsx;
using namespace Quantify;
using namespace Quantify::Consts;
///
/// \brief ClassRecord::ClassRecord
/// \param folder
/// \param engine
///
ClassRecord::ClassRecord(QString folder, RuleEngine engine)
    : rules(folder, this, engine) {
    this->engine = engine;
    this->folder = folder;
}
///
/// \brief ClassRecord::~ClassRecord
///
ClassRecord::~ClassRecord() {}
bool ClassRecord::addRecord(QString student, QString reason, Record rec) {
    auto it = this->students.find(student);
    if (it == this->students.end())
        return false;
    else {
        it->records.insert(reason, rec);
        return true;
    }
    return true;
}
///
/// \brief ClassRecord::isRecordValid
/// \param file
/// \return
///
CheckResult ClassRecord::isRecordValid(const QString &file) {
    return rules.isRecordValid(file);
}
///
/// \brief ClassRecord::isRuleValid
/// \param rule
/// \return
///
CheckResult ClassRecord::isRuleValid(const QString &rule) {
    return rules.isRuleValid(rule);
}
///
/// \brief ClassRecord::isGroupFileValid
/// \param file
/// \return
///
CheckResult ClassRecord::isGroupFileValid(const QString &file) {
    QString data = file;
    QTextStream in(&data);
    QString firstLine = in.readLine();
    CheckResult res;
    res.success = false;
    if (firstLine.isNull()) {
        res.info = "文件为空";
        return res;
    }

    QStringList parts = firstLine.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 2) {
        res.info = "第一行格式错误，应为 '组名 中文组名' 两个字段";
        return res;
    } else {
        if (groups.contains(parts[0]))
            res.info += "组已经定义，";
    }
    QString groupKey = parts[0];
    if (groupKey.isEmpty()) {
        res.info += "组名不能为空";
        return res;
    }
    // 组名不能包含 '-'（避免与排除语法冲突）
    if (groupKey.contains('-')) {
        return {false, "组名不能包含 '-' 字符"};
        return res;
    }

    int lineNum = 2;
    while (!in.atEnd()) {
        QString member = in.readLine().trimmed();
        if (member.isEmpty()) {
            res.info += QString("第 %1 行: 存在空行").arg(lineNum);
            return res;
        }
        if (!students.contains(member)) {
            res.info += QString("第 %1 行: 成员 '%2' 不存在于学生名单中")
                            .arg(lineNum)
                            .arg(member);
            return res;
        }
        lineNum++;
    }
    res.info += "组文件有效";
    res.success = true;
    return res;
}
///
/// \brief ClassRecord::index
/// \param abbr
/// \param name
/// \return
///
int ClassRecord::index(QString abbr, QString name) {
    int index = 0;
    for (auto it = this->students.begin(); it != students.end(); it++, index++) {
        bool ab = it.key() == abbr || abbr.isEmpty();
        bool ch = it->name_ch == name || name.isEmpty();
        if (ab && ch)
            return index;
    }
    return -1;
}
///
/// \brief ClassRecord::week
/// \return
///
unsigned int ClassRecord::week() { return weekCount; }
///
/// \brief ClassRecord::getRecord
/// \param name
/// \param rule
/// \return
///
Record ClassRecord::getRecord(QString name, QString rule) {
    auto it = students.find(name);
    if (it == students.end())
        return RuleBase::empty;
    else {
        auto it2 = it.value().records.find(rule);
        if (it2 == it.value().records.end())
            return RuleBase::empty;
        else
            return it2.value();
    }
}
///
/// \brief ClassRecord::getScore
/// \param name
/// \return
///
Record ClassRecord::getScore(QString name) {
    auto it = students.find(name);
    if (it == students.end())
        return RuleBase::empty;
    return it->getScore();
}
///
/// \brief ClassRecord::clear
///
void ClassRecord::clear() {
    weekCount = 0;
    groups.clear();
    students.clear();
    rules.clear();
}
///
/// \brief ClassRecord::refresh
///
void ClassRecord::refresh() {
    clear();
    readStudentFile();
    readGroupFile();
    readRuleFile();
    readRecordFile();
}
///
/// \brief ClassRecord::readStudentFile
///
void ClassRecord::readStudentFile() {
    QString str = QDir(folder).filePath("namelist.xlsx");
    Document xlsxR(str);
    if (xlsxR.load()) {
        for (int row = 1;; row++) {
            QString name = xlsxR.read(row, 1).toString();
            if (name == "")
                break;
            QString name_ch = xlsxR.read(row, 2).toString();
            StudentRecord sr;
            sr.name_ch = name_ch;
            this->students.insert(name, sr);
        }
    }
    GroupRecord gr;
    gr.name_ch = "全部";
    for (auto it = this->students.begin(); it != this->students.end(); it++)
        gr.members.push_back(it.key());
    this->groups.insert("ALL", gr);
}
///
/// \brief ClassRecord::readGroupFile
///
void ClassRecord::readGroupFile() {
    QDir dir = QDir(folder).filePath(DirGroup);
    QStringList filter("*.group");
    dir.setNameFilters(filter);
    QList<QFileInfo> fileInfo(dir.entryInfoList(filter));
    for (int i = 0; i <= fileInfo.size() - 1; i++) {
        QString str = fileInfo.at(i).filePath();
        QFile file(str);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            GroupRecord gr;
            QString str = in.readLine();
            auto list = str.split(' ');
            if (list.length() <= 1)
                continue;
            gr.name_ch = list[1];
            while (!in.atEnd()) {
                gr.members.push_back(in.readLine());
            }
            this->groups.insert(list[0], gr);
            file.close();
        }
    }
}
///
/// \brief ClassRecord::readRuleFile
///
void ClassRecord::readRuleFile() {
    QDir dir = QDir(folder).filePath(DirRule);
    QStringList filter("*.rule");
    dir.setNameFilters(filter);
    QList<QFileInfo> fileInfo(dir.entryInfoList(filter));
    for (int i = 0; i <= fileInfo.size() - 1; i++) {
        QString str = fileInfo.at(i).filePath();
        QFile file(str);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content;
            while (!in.atEnd()) {
                content += in.readLine() + '\n';
            }
            file.close();
            rules.analyzeRuleFile(content);
        }
    }
}
///
/// \brief ClassRecord::readRecordFile
///
void ClassRecord::readRecordFile() {
    QDir dir = QDir(folder).filePath(DirRecord);
    QStringList filter("*.record");
    dir.setNameFilters(filter);
    QList<QFileInfo> fileInfo(dir.entryInfoList(filter));
    for (int i = 0; i < fileInfo.size(); i++) {
        QString str = fileInfo.at(i).filePath();
        QFile file(str);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content;
            while (!in.atEnd()) {
                content += in.readLine() + '\n';
            }
            file.close();
            rules.analyzeRecordFile(content,
                                    fileInfo.at(i).fileName().split(".").first());
        }
    }
}
