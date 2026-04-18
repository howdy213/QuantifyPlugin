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
#include "logger.h"
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
        QString err = "JS 规则必须返回一个对象";
        Logger::instance().error(err);
        return false;
    }
    QJSValue reasonVal = obj.property("reason");
    if (!reasonVal.isString()) {
        Logger::instance().error("JS 规则缺少 'reason' 属性");
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
        Logger::instance().error("JS 规则至少需要一个可调用的函数");
        return false;
    }
    return true;
}
///
/// \brief JSRule::createCallObject
/// \param rec
/// \return
///
QJSValue JSRule::createCallObject(const Record &rec) const {
    QJSValue obj = m_engine->newObject();
    QJSValue recordObj = recordToJS(rec);
    obj.setProperty("record", recordObj);
    QJSValue logObj = m_engine->newObject();
    logObj.setProperty("message", "");
    logObj.setProperty("level", "info");
    obj.setProperty("log", logObj);
    return obj;
}
///
/// \brief JSRule::extractLogFromJS
/// \param logObj
/// \param message
/// \param level
///
void JSRule::extractLogFromJS(const QJSValue &logObj, QString &message,
                              QString &level) const {
    if (!logObj.isObject())
        return;
    QJSValue msgVal = logObj.property("message");
    if (msgVal.isString())
        message = msgVal.toString();
    QJSValue lvlVal = logObj.property("level");
    if (lvlVal.isString())
        level = lvlVal.toString();
}
// 递归打印 QVariant 的辅助函数
void printVariant(const QString &prefix, const QVariant &value) {
    if (value.typeId() == QMetaType::Type::QVariantMap) {
        QVariantMap map = value.toMap();
        qDebug().noquote() << prefix << "{";
        for (auto it = map.begin(); it != map.end(); ++it) {
            printVariant(prefix + "  " + it.key() + ": ", it.value());
        }
        qDebug().noquote() << prefix << "}";
    } else if (value.typeId() == QMetaType::Type::QVariantList) {
        QVariantList list = value.toList();
        qDebug().noquote() << prefix << "[";
        for (int i = 0; i < list.size(); ++i) {
            printVariant(prefix + "  ", list[i]);
        }
        qDebug().noquote() << prefix << "]";
    } else {
        qDebug().noquote() << prefix << value.toString();
    }
}
///
/// \brief JSRule::result
/// \param rt
/// \param rec
/// \return
///
ExecuteResult JSRule::result(RecordType rt, const Record &rec) const {
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
        return {rec,{"error","未知的日期类型"}};
    }
    if (!func.isCallable())
        return {rec,{"error","JS规则不为可调用对象"}};

    // 创建调用参数对象 { record, log }
    QJSValue callObj = createCallObject(rec);

    QVariant callVar = callObj.toVariant();
    QJSValue res = func.call({callObj});
    if (res.isError()) {
        QString err =
            QString("JS 规则 '%1' 执行错误: %2").arg(m_reason, res.toString());
        ExecuteResult result;
        result.record = rec;
        result.log = {"error", err};
        return result;
    }

    if (!res.isObject() || !res.hasProperty("record")) {
        QString err =
            QString(
                          "JS 规则 '%1' 返回值无效：必须返回 { record: {...}, log: {...} }")
                          .arg(m_reason);
        ExecuteResult result;
        result.record = rec;
        result.log = {"error", err};
        return result;
    }

    QJSValue recordVal = res.property("record");
    if (!recordVal.isObject()) {
        QString err =
            QString("JS 规则 '%1' 返回值中 'record' 不是对象").arg(m_reason);
        ExecuteResult result;
        result.record = rec;
        result.log = {"error", err};
        return result;
    }

    Record newRec = recordFromJS(recordVal);

    ExecuteResult result;
    result.record = newRec;

    QJSValue logVal = res.property("log");
    if (logVal.isObject()) {
        QString logMsg, logLevel;
        extractLogFromJS(logVal, logMsg, logLevel);
        if (!logMsg.isEmpty()) {
            if (logLevel == "warning"||logLevel == "error"||logLevel == "info")
                result.log.level=logLevel;
            else result.log.level="info";
            result.log.message=logMsg;
        }
    }
    return result;
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
