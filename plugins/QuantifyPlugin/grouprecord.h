/**
 * @file grouprecord.h
 * @brief 组记录类
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
#ifndef GROUPRECORD_H
#define GROUPRECORD_H
#include <QString>
#include <QVector>

#include "rulebase.h"
#include "studentrecordviewer.h"

class GroupRecord {
public:
    GroupRecord();
    QMap<QString, Record> records;
    QVector<QString> members;
    QVector<RecordInfo> info;
    QVector<Record> weekly;
    QString name_ch;
    void addCRInfo(QString date, QString note, QString reason, float delta);
    Record getScore();
};

#endif // GROUPRECORD_H
