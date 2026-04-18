/**
 * @file classrule.h
 * @brief 班级规则类
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
#pragma once
#include <QVector>

#include "rulebase.h"
#include "studentrecord.h"

class ClassRecord;

enum class RuleEngine { Native, JS };

class ClassRule {
public:
    explicit ClassRule(QString rulePath, ClassRecord* parent, RuleEngine engine = RuleEngine::Native);
    ~ClassRule();
    ClassRule(const ClassRule&) = delete;
    ClassRule& operator=(const ClassRule&) = delete;
    CheckResult isRecordValid(const QString& file);
    CheckResult isRuleValid(const QString& rule);
    void clear(RecordType rt);
    void clear();
private:
    friend class ClassRecord;
    void analyzeRuleFile(const QString& file);
    bool analyzeRecordFile(const QString& file, const QString& date = "2024xxxx");
    void changeRecord(const QStringList &param, const QString &date,
                      RuleBase *rule, RecordType rt,
                      QMap<QString, StudentRecord>::iterator studentIt,
                      const QString &prefixReason, double scoreForCustom);
    ClassRecord* m_cr;
    QVector<RuleBase*> m_rules;
    QString m_rulePath;
    RuleEngine m_engine;
};
