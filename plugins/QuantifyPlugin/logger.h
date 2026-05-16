/**
 * @file logger.h
 * @brief 简单日志类
 * @author howdy213
 * @date 2026-05-16
 * @version 2.0.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDir>

class Logger
{
public:
    enum Level { Info, Warning, Error };

    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    void setLogPath(const QString& path)
    {
        QMutexLocker locker(&m_mutex);
        m_logDir = path;
        ensureLogFile();
    }

    void log(Level level, const QString& message)
    {
        QMutexLocker locker(&m_mutex);
        ensureLogFile();
        if (!m_file.isOpen()) return;

        QString prefix;
        switch (level) {
        case Info:    prefix = "[INFO] "; break;
        case Warning: prefix = "[WARN] "; break;
        case Error:   prefix = "[ERR] "; break;
        }
        QTextStream out(&m_file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
            << " " << prefix << message << Qt::endl;
        m_file.flush();
    }

    void log(const QString& level, const QString& message)
    {
        Level lvl = Info;
        QString l = level.toLower();
        if (l == "warning" || l == "warn")
            lvl = Warning;
        else if (l == "error" || l == "err")
            lvl = Error;
        log(lvl, message);
    }

    void clear()
    {
        QMutexLocker locker(&m_mutex);
        if (m_file.isOpen()) {
            m_file.close();
        }
        if (!m_logDir.isEmpty()) {
            QString logFilePath = QDir(m_logDir).filePath("quantify.log");
            QFile::remove(logFilePath);
            ensureLogFile();
        }
    }

    void info(const QString& msg)  { log(Info, msg); }
    void warn(const QString& msg)  { log(Warning, msg); }
    void error(const QString& msg) { log(Error, msg); }

private:
    Logger() = default;
    void ensureLogFile()
    {
        if (m_logDir.isEmpty()) return;
        QDir dir(m_logDir);
        if (!dir.exists()) dir.mkpath(".");
        QString logFilePath = dir.filePath("quantify.log");
        if (!m_file.isOpen() || m_file.fileName() != logFilePath) {
            if (m_file.isOpen()) m_file.close();
            m_file.setFileName(logFilePath);
            m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        }
    }
    QFile m_file;
    QString m_logDir;
    QMutex m_mutex;
};

#endif // LOGGER_H