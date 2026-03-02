/**
 * @file nativerule.h
 * @brief 自定义规则引擎
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
#include <QStringList>

#include "nativerule.h"

///
/// \brief NativeRule::loadFromFile
/// \param content
/// \return
///
bool NativeRule::loadFromFile(const QString &content) {
    const auto lines = QStringView(content).split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return false;

    const auto first = lines[0].split(' ', Qt::SkipEmptyParts);
    if (first.isEmpty())
        return false;
    m_reason = first[0].toString();
    m_reason_ch = (first.size() > 1) ? first[1].toString() : m_reason;

    int idx = 1;
    if (idx < lines.size() && lines[idx] == "-")
        ++idx;

    int group = 0;
    while (idx < lines.size() && group < 3) {
        if (lines[idx] == "-") {
            ++group;
        } else {
            m_rules[group].append(lines[idx].toString());
        }
        ++idx;
    }
    return true;
}
///
/// \brief NativeRule::isRuleValid
/// \param rule
/// \return
///
CheckResult NativeRule::isRuleValid(const QString &rule) {
    // 按行分割，忽略空行
    QStringList lines = rule.split('\n', Qt::SkipEmptyParts);
    if (lines.size() < 4) {
        return {false, "规则文件至少需要4行（标题行 + 三个分隔符行）"};
    }

    // 检查第一行格式：英文名 中文名（至少一个空格分隔）
    QString firstLine = lines[0].trimmed();
    if (!firstLine.contains(' ')) {
        return {false, "第一行必须包含空格，格式：英文名 中文名"};
    }

    // 统计分隔符 '-' 的数量
    int dashCount = 0;
    for (int i = 1; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "-") {
            ++dashCount;
        }
    }
    if (dashCount < 3) {
        return {false, "规则文件缺少必要的分隔符 '-'，至少需要3个"};
    }

    // 基本检查通过，更详细的语法错误将在加载时检测
    return {true, "规则文件格式基本正确"};
}
///
/// \brief NativeRule::result
/// \param rt
/// \param rec
/// \return
///
Record NativeRule::result(RecordType rt, const Record &rec) const {
    int idx = -1;
    switch (rt) {
    case DAILY:
        idx = 0;
        break;
    case WEEKLY:
        idx = 1;
        break;
    case TERMLY:
        idx = 2;
        break;
    default:
        return rec;
    }

    Record res = rec;
    for (const auto &rule : m_rules[idx])
        res = getFinalResult(res, rule);
    return res;
}

// -------- static helpers ----------
///
/// \brief NativeRule::isVariable
/// \param var
/// \return
///
bool NativeRule::isVariable(const QString &var) {
    if (var.length() == 1)
        return var[0] == 't' || var[0] == 's';
    if (var.length() != 2)
        return false;
    return (var[0] == 't' || var[0] == 's') &&
           (var[1] == '1' || var[1] == '2' || var[1] == '3');
}
///
/// \brief NativeRule::isSingleRuleTrue
/// \param rec
/// \param rule
/// \return
///
bool NativeRule::isSingleRuleTrue(const Record &rec, const QString &rule) {
    const QString varStr = rule.left(2);
    if (!isVariable(varStr))
        return true;

    const double var = getVariable(rec, varStr);
    if (rule.size() < 4)
        return true;

    const QChar op = rule[2];
    bool ok;
    const double val = QStringView(rule).mid(3).toDouble(&ok);
    if (!ok)
        return true;

    if (op == '>')
        return var > val;
    if (op == '=')
        return var == val;
    if (op == '<')
        return var < val;
    return true;
}
///
/// \brief NativeRule::isRuleTrue
/// \param rec
/// \param rules
/// \return
///
bool NativeRule::isRuleTrue(const Record &rec, const QString &rules) {
    bool result = true;
    int start = 0;
    for (int i = 0; i <= rules.size(); ++i) {
        if (i == rules.size() || rules[i] == '&' || rules[i] == '|') {
            const bool sub = isSingleRuleTrue(rec, rules.mid(start, i - start));
            if (i == rules.size() || rules[i] == '&')
                result = result && sub;
            else // '|'
                result = result || sub;
            start = i + 1;
        }
    }
    return result;
}
///
/// \brief NativeRule::getSingleResult
/// \param rec
/// \param func
/// \return
///
Record NativeRule::getSingleResult(const Record &rec, const QString &func) {
    const int vpos = (func.size() > 1 && func[1].isDigit()) ? 2 : 1;
    const QString varStr = func.left(vpos);
    if (!isVariable(varStr))
        return rec;

    const double var = getVariable(rec, varStr);
    if (func.size() <= vpos)
        return rec;
    const QChar op = func[vpos];
    if (op != '+' && op != '-' && op != '=')
        return rec;

    // parse delta: [+-]?[0-9.]+[a-z][0-9]?
    int i = vpos + 1;
    while (i < func.size() && (func[i].isDigit() || func[i] == '.'))
        ++i;
    const QString valStr = func.mid(vpos + 1, i - vpos - 1);
    bool ok;
    const double val = valStr.toDouble(&ok);
    if (!ok)
        return rec;

    const QString rest = func.mid(i);
    double var2 = 1.0;
    if (!rest.isEmpty()) {
        if (!isVariable(rest))
            return rec;
        var2 = getVariable(rec, rest);
    }

    double delta = 0;
    if (op == '+')
        delta = val * var2;
    else if (op == '-')
        delta = -val * var2;
    else if (op == '=')
        delta = val * var2 - var;

    return setVariable(rec, varStr, delta);
}
///
/// \brief NativeRule::getResult
/// \param rec
/// \param funcs
/// \return
///
Record NativeRule::getResult(const Record &rec, const QString &funcs) {
    Record r = rec;
    int start = 0;
    for (int i = 0; i <= funcs.size(); ++i) {
        if (i == funcs.size() || funcs[i] == ':') {
            r = getSingleResult(r, funcs.mid(start, i - start));
            start = i + 1;
        }
    }
    return r;
}
///
/// \brief NativeRule::getFinalResult
/// \param rec
/// \param completeRule
/// \return
///
Record NativeRule::getFinalResult(const Record &rec,
                                  const QString &completeRule) {
    const int colon = completeRule.indexOf(':');
    if (colon < 0)
        return rec;
    const QString cond = completeRule.left(colon);
    const QString acts = completeRule.mid(colon + 1);
    return isRuleTrue(rec, cond) ? getResult(rec, acts) : rec;
}
