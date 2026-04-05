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
#ifndef CLASSRECORD_H
#define CLASSRECORD_H
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "classrule.h"
#include "grouprecord.h"

class ClassRecord
{
public:
    ClassRecord(QString folder, RuleEngine engine = RuleEngine::Native);
    ~ClassRecord();
public:
    bool addRecord(QString student,QString reason,Record rec);
    CheckResult isRecordValid(const QString& record);
    CheckResult isRuleValid(const QString& rule);
    CheckResult isGroupFileValid(const QString& file);
    int index(QString abbr, QString ch);
    unsigned int week();
    Record getRecord(QString name,QString rule);
    Record getScore(QString name);
    void clear();
    void refresh();
    QMap<QString,StudentRecord>students;
    QMap<QString,GroupRecord>groups;
private:
    void readStudentFile();
    void readGroupFile();
    void readRuleFile();
    void readRecordFile();
private:
    friend ClassRule;
    unsigned int weekCount=0;
    QString folder="";
    ClassRule rules;
    RuleEngine engine;
};

#endif // CLASSRECORD_H
