#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QVariantMap>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

#include <signal.h>
#include <stdlib.h>

#include "helper.h"
#include "datastore.h"

const QString MS = "ms";
const QString START_LOGGER = "START LOGGER";
const QString RUN_TOOK = "run took";
const QString PENDING = "Pending";
const QString PREPARE = "Prepare";
const QString RUN = "Run";
const QString SENT = "Sent";
const QString SLAVE_RUNNER = "dasslave";

static bool keepRunning = true;

void  INThandler(int sig)
{
    if(sig == SIGINT)
    {
        keepRunning = false;
        signal(sig, SIG_IGN);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    signal(SIGINT, INThandler);

    QCommandLineParser parser;

    QCommandLineOption configOption(QStringList() << "c" << "cfg", "Config path", "path");
    parser.addOption(configOption);

    QCommandLineOption inputOption(QStringList() << "i" << "in", "Input file", "path");
    parser.addOption(inputOption);

    QCommandLineOption endlessOption(QStringList() << "e" << "endless", "Endless mode");
    parser.addOption(endlessOption);

    QCommandLineOption outputOption(QStringList() << "o" << "ouput", "Output Path", "path");
    parser.addOption(outputOption);

    parser.process(a);

    QChar separator = '|';
    int dateIndex = 0;
    int runnerIndex = 1;
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
    DataStore ds;

    if(parser.isSet(outputOption))
        ds.setOutputFile(parser.value(outputOption));

    QTextStream textStream(&inputFile);

    QElapsedTimer tmr;
    tmr.start();

    Run& currentRun = ds.createRun();

    while((!textStream.atEnd() || parser.isSet(endlessOption)) && keepRunning)
    {
        auto currentLine = textStream.readLine();
        if(currentLine.isNull())
            continue;

        auto values = currentLine.split(separator, QString::SkipEmptyParts);
        if(values.isEmpty())
            continue;

        auto timelog = QDateTime::fromString(parseDate(values[dateIndex]), "yyyy/MM/dd HH:mm:ss.zzz").toMSecsSinceEpoch();
        auto pattern = values[patternIndex].replace("#", "").trimmed().toInt();
        auto log = values[logIndex];
        auto runner = values[runnerIndex].trimmed();

        if(log.contains(START_LOGGER))
        {
            if(currentRun.started())
            {
                currentRun.dump();
                currentRun = ds.createRun();
            }
            currentRun.setStartLog(timelog);
        }

        if(pattern == 7200)
        {
            if(log.contains(PENDING) || log.contains(PREPARE) || log.contains(RUN))
            {
                auto firstSlice = log.split("[")[0];
                auto id = firstSlice.split(" ").last().toULongLong();
                if(log.contains(PENDING))
                    currentRun.setTaskStepTime(id, TaskSteps::TaskStep::Pending, timelog);
                else if(log.contains(PREPARE))
                    currentRun.setTaskStepTime(id, TaskSteps::TaskStep::Prepare, timelog);
                else if(log.contains(RUN))
                    currentRun.setTaskStepTime(id, TaskSteps::TaskStep::Run, timelog);

                currentRun.setTaskStepRunner(id, runner);
            }
            else if(log.contains(SENT) && runner.contains(SLAVE_RUNNER))
            {
                auto id = log.split(" ").last().toLongLong();
                currentRun.setTaskStepTime(id, TaskSteps::TaskStep::Finished, timelog);
            }
        }
        else if(pattern == 9003)
            currentRun.setReadyLog(timelog);

        lineCpt++;

        if(lineCpt % 500 == 0)
            qInfo() << "Current line: " << lineCpt;
    }

    qInfo() << "Analysis took " << tmr.elapsed() << " ms for " << lineCpt << " lines";
    currentRun.dump();
    return 0;
}
