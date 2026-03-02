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
#include <QDebug>
#include <QSettings>
#include <QStringView>

#include "classrecord.h"
#include "classrule.h"
#include "jsrule.h"
#include "nativerule.h"

struct TargetInfo {
    QString groupName;
    QStringList exclude;
};
///
/// \brief parseTarget
/// \param target
/// \return
///
static TargetInfo parseTarget(const QString &target) {
    TargetInfo info;
    if (target.contains('-')) {
        QStringList parts = target.split('-', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            info.groupName = parts[0];
            info.exclude = parts.mid(1);
        }
    } else {
        info.groupName = target;
    }
    return info;
}
///
/// \brief ClassRule::ClassRule
/// \param rulePath
/// \param parent
/// \param engine
///
ClassRule::ClassRule(QString rulePath, ClassRecord *parent, RuleEngine engine)
    : m_cr(parent), m_rulePath(std::move(rulePath)), m_engine(engine) {}
///
/// \brief ClassRule::~ClassRule
///
ClassRule::~ClassRule() { clear(); }
///
/// \brief ClassRule::analyzeRuleFile
/// \param file
///
void ClassRule::analyzeRuleFile(const QString &file) {
    std::unique_ptr<RuleBase> newRule;
    if (m_engine == RuleEngine::Native)
        newRule.reset(new NativeRule);
    else
        newRule.reset(new JSRule);

    if (!newRule->loadFromFile(file))
        return;

    for (auto &stu : m_cr->students)
        stu.records.insert(newRule->reason(), Record{});

    m_rules.push_back(newRule.release());
}
///
/// \brief ClassRule::analyzeRecordFile
/// \param file 记录文件内容
/// \param date 日期字符串（如 "2024xxxx"）
/// \return 是否成功处理
///
bool ClassRule::analyzeRecordFile(const QString &file, const QString &date) {
    auto lines = file.split('\n', Qt::SkipEmptyParts);
    // 处理行内注释
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        QStringList list = it->split("//");
        *it = list[0];
    }
    if (lines.isEmpty())
        return false;

    // 解析记录类型
    RecordType rt = NONE;
    if (lines[0] == "daily")
        rt = DAILY;
    else if (lines[0] == "weekly")
        rt = WEEKLY;
    else if (lines[0] == "termly")
        rt = TERMLY;
    if (rt == NONE)
        return false;

    RuleBase *currentRule = nullptr;
    QString extra;
    int startIdx = 1;

    for (int i = startIdx; i < lines.size(); ++i) {
        const auto &line = lines[i];
        if (line.isEmpty())
            continue;

        // 新规则块开始，格式如 "[reason][ extra extra]"
        if (line[0] == '[') {
            extra.clear();
            startIdx = i;
            const auto parts = line.split(']'); // ["[reason", "[ extra extra", ""]
            const QString reason = parts[0].mid(1); // "reason"
            if (parts.size() >= 2)
                extra = parts[1].mid(1); // " extra extra"

            // 查找匹配的规则
            currentRule = nullptr;
            foreach (auto *r, m_rules) {
                if (r->reason() == reason) {
                    currentRule = r;
                    break;
                }
            }
        }
        // 普通数据行
        else if (currentRule) {
            QString processed = line + extra; // 追加额外后缀
            const auto tokens = processed.split(' ', Qt::SkipEmptyParts);
            if (tokens.isEmpty())
                continue;

            double customScore = 0.0;
            // 处理自定义规则的特殊逻辑
            if (currentRule->reason() == "custom") {
                if (tokens.size() < 2)
                    continue;
                const QString numPart = tokens[1].mid(1); // 去掉符号位
                // notePart = tokens[2].mid(1);
                bool ok = false;
                double delta = numPart.toDouble(&ok);
                if (!ok)
                    continue;
                customScore = delta * (tokens[1][0] == '+' ? 1 : -1);
                // 可能发生11->-1，必须有符号
            }

            const QString target = tokens[0];
            TargetInfo targetInfo = parseTarget(target);

            // 查找组
            auto groupIt = m_cr->groups.constFind(targetInfo.groupName);
            if (groupIt == m_cr->groups.constEnd()) { // 说明是学生
                auto stuIt = m_cr->students.find(target);
                if (stuIt != m_cr->students.end())
                    changeRecord(tokens, date, currentRule, rt, stuIt, "", customScore);
                continue;
            }

            QStringList members = groupIt->members;      // 组内所有成员
            QString groupDisplayName = groupIt->name_ch; // 用于显示的前缀

            // 排除指定的成员
            foreach (const QString &ex, targetInfo.exclude) {
                members.removeAll(ex);
            }

            // 遍历每个成员，调用 changeRecord
            for (const QString &member : members) {
                auto stuIt = m_cr->students.find(member);
                if (stuIt != m_cr->students.end()) {
                    changeRecord(tokens, date, currentRule, rt, stuIt,
                                 '(' + groupDisplayName + ')', customScore);
                }
            }
        }
    }

    // 每周处理：累计周记录
    if (rt == WEEKLY) {
        for (auto &stu : m_cr->students) {
            Record weeklySum{};
            foreach (const auto &rec, stu.records)
                weeklySum.s2 += rec.s2;
            stu.weekly.push_back(weeklySum);
        }
        ++m_cr->weekCount;
    }

    // 清除临时记录（由调用方负责）
    clear(rt);
    return true;
}
///
/// \brief ClassRule::changeRecord
/// \param param
/// \param date
/// \param rule
/// \param rt
/// \param stuIt
/// \param prefixReason
/// \param customScore
///
void ClassRule::changeRecord(const QStringList &param, const QString &date,
                             RuleBase *rule, RecordType rt,
                             QMap<QString, StudentRecord>::iterator stuIt,
                             const QString &prefixReason, double customScore) {
    const QString reason = rule->reason();
    auto &records = stuIt->records;
    auto it = records.find(reason);
    if (it == records.end())
        it = records.insert(reason, Record{});

    const Record old = it.value();
    if (reason != "custom") {
        it.value() = rule->result(rt, old);
    } else {
        it.value() = setVariable(old, "s", customScore);
    }

    QString note = (param.size() > 1) ? param[1] : "无";
    if (reason == "custom")
        note = param.length() >= 3 ? param[2] : "无";
    const QString reasonAdd = prefixReason + rule->reasonCh() + '(' +
                              (rt == DAILY    ? "每日"
                                                                       : rt == WEEKLY ? "每周"
                                                                                                           : "每学期") +
                              ')';
    const double delta = (reason == "custom") ? customScore : (it->s3 - old.s3);
    stuIt->addCRInfo(date, note, reasonAdd, delta, rt, "", m_cr->weekCount);
}
///
/// \brief ClassRule::isRecordValid
/// \param file 记录文件内容
/// \return 校验结果，包含是否有效及错误信息
///
CheckResult ClassRule::isRecordValid(const QString &file) {
    auto lines = file.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return {false, "文件内容为空"};
    }

    // 处理行内注释
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        QStringList list = it->split("//");
        *it = list[0];
    }

    // 校验记录类型
    RecordType rt = NONE;
    if (lines[0] == "daily")
        rt = DAILY;
    else if (lines[0] == "weekly")
        rt = WEEKLY;
    else if (lines[0] == "termly")
        rt = TERMLY;
    if (rt == NONE) {
        return {false, "首行必须是 'daily'、'weekly' 或 'termly'"};
    }

    QString extra;
    int startIdx = 1;
    for (int i = startIdx; i < lines.size(); ++i) {
        const auto &line = lines[i];
        if (line.isEmpty())
            continue; // 理论上已被 split 跳过，但保留安全

        // 新规则块开始
        if (line[0] == '[') {
            startIdx = i;
            const auto parts = line.split(']');
            if (parts.isEmpty()) {
                return {false, QString("第 %1 行: 格式错误，缺少 ']'").arg(i + 1)};
            }
            const QString reason = parts[0].mid(1); // 去掉 '['
            extra = (parts.size() >= 2) ? parts[1].mid(1) : QString();

            // 查找对应的规则是否存在
            bool found = false;
            foreach (const auto *r, m_rules) {
                if (r->reason() == reason) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return {false,
                        QString("第 %1 行: 未找到规则 '%2'").arg(i + 1).arg(reason)};
            }
        }
        // 普通数据行
        else {
            QString processed = line + extra; // 添加额外后缀
            const auto tokens = processed.split(' ', Qt::SkipEmptyParts);
            if (tokens.isEmpty()) {
                return {false, QString("第 %1 行: 缺少操作对象").arg(i + 1)};
            }

            // 检查 custom 规则的特殊格式
            if (tokens[0] == "custom") {
                if (tokens.size() < 2) {
                    return {false,
                            QString("第 %1 行: custom 规则必须指定分数").arg(i + 1)};
                }
                bool ok;
                tokens[1].mid(1).toDouble(&ok); // 检查分数部分是否为有效数字
                if (!ok) {
                    return {false, QString("第 %1 行: 分数格式错误").arg(i + 1)};
                }
            }

            // 校验目标（组排除语法）
            const QString target = tokens[0];
            if (target.contains('-')) {
                // 解析组名和排除列表
                QStringList parts = target.split('-', Qt::SkipEmptyParts);
                if (parts.isEmpty()) {
                    return {false, QString("第 %1 行: 格式错误").arg(i + 1)};
                }
                QString groupName = parts[0];
                QStringList excludeList = parts.mid(1);

                // 检查组是否存在
                auto git = m_cr->groups.constFind(groupName);
                if (git == m_cr->groups.constEnd()) {
                    return {
                            false,
                        QString("第 %1 行: 组 '%2' 不存在").arg(i + 1).arg(groupName)};
                }
                const QStringList &groupMembers = git->members;
                // 检查排除成员是否都在组内
                foreach (const QString &ex, excludeList) {
                    if (!groupMembers.contains(ex)) {
                        return {false, QString("第 %1 行: 排除成员 '%2' 不在组 '%3' 中")
                                           .arg(i + 1)
                                           .arg(ex, groupName)};
                    }
                }
            } else {
                // 普通目标：单个学生或组（不含排除）
                if (!m_cr->groups.contains(target) &&
                    !m_cr->students.contains(target)) {
                    return {false,
                            QString("第 %1 行: 目标 '%2' 不存在").arg(i + 1).arg(target)};
                }
            }
        }
    }
    return {true, "记录文件有效"};
}
///
/// \brief ClassRule::isRuleValid
/// \param rule
/// \return
///
CheckResult ClassRule::isRuleValid(const QString &rule) {
    switch (m_engine) {
    case RuleEngine::Native:
        return NativeRule::isRuleValid(rule);
    case RuleEngine::JS:
        return JSRule::isRuleValid(rule);
    default:
        break;
    }
}
///
/// \brief ClassRule::clear
/// \param rt
///
void ClassRule::clear(RecordType rt) {
    for (auto &stu : m_cr->students) {
        for (auto &rec : stu.records) {
            switch (rt) {
            case DAILY:
                rec.t1 = 0;
                rec.s1 = 0.0;
                break;
            case WEEKLY:
                rec.t2 = 0;
                rec.s2 = 0.0;
                break;
            case TERMLY:
                rec.t3 = 0;
                rec.s3 = 0.0;
                break;
            default:
                break;
            }
        }
    }
}
///
/// \brief ClassRule::clear
///
void ClassRule::clear() {
    qDeleteAll(m_rules);
    m_rules.clear();
}
