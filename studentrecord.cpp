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
#include "studentrecord.h"

///
/// \brief StudentRecord::StudentRecord
///
StudentRecord::StudentRecord() {}
///
/// \brief StudentRecord::getScore
/// \return
///
Record StudentRecord::getScore() const {
    Record rec = Record();
    foreach (Record rec2, records.values()) {
        rec.s1 += rec2.s1;
        rec.s2 += rec2.s2;
        rec.s3 += rec2.s3;
    }
    return rec;
}
///
/// \brief StudentRecord::addCRInfo
/// \param date
/// \param note
/// \param reason
/// \param delta
/// \param rt
/// \param rule
/// \param week
///
void StudentRecord::addCRInfo(QString date, QString note, QString reason,
                              float delta, RecordType rt, QString rule,
                              int week) {
    RecordInfo info;
    info.date = date;
    info.note = note;
    info.delta = delta;
    info.reason = reason;
    info.recordType = rt;
    info.rule = rule;
    info.week = week;
    this->info.push_back(info);
}
///
/// \brief StudentRecord::getWeeklyInfo
/// \param week
/// \return
///
QVector<RecordInfo> StudentRecord::getWeeklyInfo(int week) const {
    auto it = info.begin();
    int last = -1;
    bool isExist = 0;
    for (; it != info.end(); it++) {
        if (it->week == week && last <= week - 1) {
            isExist = 1;
            break;
        }
        last = it->week;
    }
    if (!isExist)
        return QVector<RecordInfo>();
    auto it2 = it;
    for (; it2 != info.end(); it2++) {
        if (it2->week >= week + 1)
            break;
    }
    QVector<RecordInfo> info;
    for (; it != it2; it++) {
        info.push_back(*it);
    }
    return info;
}
