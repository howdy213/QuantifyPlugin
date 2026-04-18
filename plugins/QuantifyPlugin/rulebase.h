/**
 * @file rulebase.h
 * @brief 规则引擎基类
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
#ifndef RULEBASE_H
#define RULEBASE_H
#include <QString>

struct CheckResult {
    bool success = false;
    QString info;
};

struct LogResult {
    QString level;
    QString message;
};

struct Record {
    int t1 = 0;
    int t2 = 0;
    int t3 = 0;
    double s1 = 0;
    double s2 = 0;
    double s3 = 0;
};

struct ExecuteResult {
    Record record;
    LogResult log;
};

enum RecordType { DAILY, WEEKLY, TERMLY, NONE };

double getVariable(const Record &rec, const QString &var);

Record setVariable(const Record &rec, const QString &var, double delta);

class RuleBase {
public:
    virtual ~RuleBase() = default;
    virtual QString reason() const = 0;
    virtual QString reasonCh() const = 0;
    // 根据记录类型和输入记录，应用规则返回新记录
    virtual ExecuteResult result(RecordType rt, const Record &rec) const = 0;
    // 从文件内容加载规则定义，成功返回 true
    virtual bool loadFromFile(const QString &fileContent) = 0;
    static const inline Record empty{-1, -1, -1, 0.0, 0.0, 0.0};
};

#endif // RULEBASE_H
