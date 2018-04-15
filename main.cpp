#include <QCoreApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QVariantMap>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

#include "helper.h"

const QString MS = "ms";
const QString RUN_TOOK = "run took";

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;

    QCommandLineOption configOption(QStringList() << "c" << "cfg", "Config path", "path");
    parser.addOption(configOption);

    QCommandLineOption inputOption(QStringList() << "i" << "in", "Input file", "path");
    parser.addOption(inputOption);

    parser.process(a);

    QChar separator = '|';
    int dateIndex = 0;
    int patternIndex = 4;
    int logIndex = 6;

    if(parser.isSet(configOption))
    {
        auto configPath = parser.value(configOption);
        QFile configFile(configPath);
        if(!configFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Invalid config file provided";
            parser.showHelp(1);
        }

        auto configMap = QJsonDocument::fromJson(configFile.readAll()).toVariant().toMap();
    }

    if(!parser.isSet(inputOption))
    {
        qWarning() << "Please provide an input file";
        parser.showHelp(1);
    }

    auto filePath = parser.value(inputOption);
    QFile inputFile(filePath);
    if(!inputFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Invalid input file provided " << filePath;
        parser.showHelp(1);
    }

    int lineCpt = 1;
    QTextStream textStream(&inputFile);

    QDateTime lastLineTime;

    QElapsedTimer tmr;
    tmr.start();

    while(!textStream.atEnd())
    {
        auto currentLine = textStream.readLine();
        auto values = currentLine.split(separator, QString::SkipEmptyParts);
        if(values.isEmpty())
            continue;

        QDateTime timelog = QDateTime::fromString(parseDate(values[dateIndex]), "yyyy/MM/dd HH:mm:ss.zzz");
        if(lastLineTime.isValid())
        {
            auto delta = abs(timelog.msecsTo(lastLineTime));
            if(delta > 1000)
                qWarning() << "Delta is " << lastLineTime.msecsTo(timelog) << " ms line " << lineCpt;
        }
        lastLineTime = timelog;

        auto pattern = values[patternIndex].replace("#", "").trimmed().toInt();
        auto log = values[logIndex];
        if(pattern == 7200)
        {
            if(log.contains(RUN_TOOK))
            {
                log.remove(0, log.lastIndexOf(RUN_TOOK) + RUN_TOOK.length());
                log.replace(MS, "");
                log = log.trimmed();
                auto taskDuration = log.toLongLong();
                if(taskDuration > 20)
                    qInfo() << "Task lenght " << taskDuration << " ms line " << lineCpt;
            }
        }

        lineCpt++;
    }

    qInfo() << "Analysis took " << tmr.elapsed() << " ms for " << lineCpt << " lines";

    return a.exec();
}
