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

#include "jsrule.h"

///
/// \brief JSRule::JSRule
/// \param parent
///
JSRule::JSRule(QObject *parent) : QObject(parent) {
    m_engine = new QJSEngine(this);
}
///
/// \brief JSRule::~JSRule
///
JSRule::~JSRule() {}
///
/// \brief JSRule::loadFromFile
/// \param content
/// \return
///
bool JSRule::loadFromFile(const QString &content) {
    QJSValue obj = m_engine->evaluate(content);
    if (!obj.isObject()) {
        qWarning() << "JS rule must return an object";
        return false;
    }

    QJSValue reasonVal = obj.property("reason");
    if (!reasonVal.isString()) {
        qWarning() << "Missing 'reason' property";
        return false;
    }
    m_reason = reasonVal.toString();

    QJSValue chVal = obj.property("reason_ch");
    m_reason_ch = chVal.isString() ? chVal.toString() : m_reason;

    m_dailyFunc = obj.property("daily");
    m_weeklyFunc = obj.property("weekly");
    m_termlyFunc = obj.property("termly");

    if (!m_dailyFunc.isCallable() && !m_weeklyFunc.isCallable() &&
        !m_termlyFunc.isCallable()) {
        qWarning() << "No callable function found";
        return false;
    }
    return true;
}
///
/// \brief JSRule::result
/// \param rt
/// \param rec
/// \return
///
Record JSRule::result(RecordType rt, const Record &rec) const {
    QJSValue func;
    switch (rt) {
    case DAILY:
        func = m_dailyFunc;
        break;
    case WEEKLY:
        func = m_weeklyFunc;
        break;
    case TERMLY:
        func = m_termlyFunc;
        break;
    default:
        return rec;
    }
    if (!func.isCallable())
        return rec;

    QJSValue jsRec = recordToJS(rec);
    QJSValue res = func.call({jsRec});
    if (res.isError()) {
        qWarning() << "JS error:" << res.toString();
        return rec;
    }
    return recordFromJS(res.isObject() ? res : jsRec);
}
///
/// \brief JSRule::recordToJS
/// \param rec
/// \return
///
QJSValue JSRule::recordToJS(const Record &rec) const {
    QJSValue obj = m_engine->newObject();
    obj.setProperty("t1", rec.t1);
    obj.setProperty("t2", rec.t2);
    obj.setProperty("t3", rec.t3);
    obj.setProperty("t", 0);
    obj.setProperty("s1", rec.s1);
    obj.setProperty("s2", rec.s2);
    obj.setProperty("s3", rec.s3);
    obj.setProperty("s", 0);
    return obj;
}
///
/// \brief JSRule::recordFromJS
/// \param obj
/// \return
///
Record JSRule::recordFromJS(const QJSValue &obj) const {
    Record r;
    r.t1 = obj.property("t1").toInt();
    r.t2 = obj.property("t2").toInt();
    r.t3 = obj.property("t3").toInt();

    r.s1 = obj.property("s1").toNumber();
    r.s2 = obj.property("s2").toNumber();
    r.s3 = obj.property("s3").toNumber();

    r = setVariable(r, "t", obj.property("t").toInt());
    r = setVariable(r, "s", obj.property("s").toNumber());
    return r;
}
///
/// \brief JSRule::isRuleValid
/// \param rule
/// \return
///
CheckResult JSRule::isRuleValid(const QString &rule) {
    QJSEngine engine;
    QJSValue result = engine.evaluate(rule);

    if (result.isError()) {
        QString errorMsg = result.toString();
        int line = -1;

        // 错误对象通常包含 lineNumber 和 columnNumber 属性
        if (result.hasProperty("lineNumber")) {
            line = result.property("lineNumber").toInt();
            return {
                    false,
                QString("JavaScript 语法错误 (行 %1): %2").arg(line).arg(errorMsg)};
        } else {
            return {false, QString("JavaScript 语法错误: %1").arg(errorMsg)};
        }
    }

    if (!result.isObject())
        return {false, "规则必须返回一个对象 (使用 ({ ... }) 包裹)"};

    QJSValue reason = result.property("reason");
    if (!reason.isString())
        return {false, "缺少必需的字符串属性 'reason' 或类型错误"};

    QJSValue reasonCh = result.property("reason_ch");
    if (!reasonCh.isUndefined() && !reasonCh.isString())
        return {false, "属性 'reason_ch' 如果存在，必须是字符串"};

    QJSValue daily = result.property("daily");
    QJSValue weekly = result.property("weekly");
    QJSValue termly = result.property("termly");

    if (!daily.isCallable() && !weekly.isCallable() && !termly.isCallable())
        return {false, "至少需要提供一个可调用的函数：daily、weekly 或 termly"};

    return {true, "JavaScript 规则有效"};
}
