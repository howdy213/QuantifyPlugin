/**
 * @file nativerule.h
 * @brief 自定义规则文件引擎
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

class NativeRule final : public RuleBase {
public:
    NativeRule() = default;

    QString reason() const override { return m_reason; }
    QString reasonCh() const override { return m_reason_ch; }
    ExecuteResult result(RecordType rt, const Record& rec) const override;
    bool loadFromFile(const QString& content) override;
    static CheckResult isRuleValid(const QString& rule) ;

    static Record getFinalResult(const Record &rec,
                                 const QString &completeRule);

private:
    static bool isVariable(const QString& var);
    static bool isSingleRuleTrue(const Record& rec, const QString& rule);
    static bool isRuleTrue(const Record& rec, const QString& rules);
    static Record getSingleResult(const Record& rec, const QString& func);
    static Record getResult(const Record& rec, const QString& funcs);

    QString m_reason;
    QString m_reason_ch;
    QVector<QString> m_rules[3];
};
