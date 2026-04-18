
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