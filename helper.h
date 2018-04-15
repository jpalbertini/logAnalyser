#ifndef HELPER_H
#define HELPER_H

#include <QString>

QString parseDate(const QString& oldString)
{
    auto values = oldString.split(" ");
    auto date = values[0];
    auto time = values[1];

    auto timeValues = time.split(":");
    timeValues[2] = timeValues[2].mid(0, 6);
    time = QString("%0:%1:%2").arg(timeValues[0]).arg(timeValues[1]).arg(timeValues[2]);

    return QString("%0 %1").arg(date).arg(time);
}

#endif // HELPER_H
