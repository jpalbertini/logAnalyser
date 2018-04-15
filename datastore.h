#ifndef DATASTORE_H
#define DATASTORE_H

#include <QDateTime>

class DataStore
{
public:
    void setFirstLog(const QDateTime& firstLog)
    {
        m_dFirstLog = firstLog;
    }

    void setReadyLog(const QDateTime& readyLog)
    {
        m_dReadyLog = readyLog;
    }

    qlonglong getLaunchTime() const
    {
        return m_dFirstLog.msecsTo(m_dReadyLog);
    }

private:
    QDateTime m_dFirstLog;
    QDateTime m_dReadyLog;
};

#endif // DATASTORE_H
