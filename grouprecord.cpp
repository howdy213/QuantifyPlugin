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
#include "grouprecord.h"

///
/// \brief GroupRecord::GroupRecord
///
GroupRecord::GroupRecord() {}
///
/// \brief GroupRecord::getScore
/// \return
///
Record GroupRecord::getScore() {
    Record rec = Record();
    foreach (Record rec2, records.values()) {
        rec.s1 += rec2.s1;
        rec.s2 += rec2.s2;
        rec.s3 += rec2.s3;
    }
    return rec;
}
///
/// \brief GroupRecord::addCRInfo
/// \param date
/// \param note
/// \param reason
/// \param delta
///
void GroupRecord::addCRInfo(QString date, QString note, QString reason,
                            float delta) {
    RecordInfo info;
    info.date = date;
    info.note = note;
    info.delta = delta;
    info.reason = reason;
    this->info.push_back(info);
}
