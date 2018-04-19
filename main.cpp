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
#include <cassert>

#include "helper.h"
#include "datastore.h"

const QString MS = "ms";
const QString START_LOGGER = "=== Start logger ===";
const QString RUN_TOOK = "run took";
const QString SERVER_RUNNER = "dasserve";
const QString SLAVE_RUNNER  = "dasslave";

static constexpr const int READY_STATE      = 9003;
static constexpr const int PENDING_STATE    = 7293;
static constexpr const int PREPARE_STATE    = 7294;
static constexpr const int RUN_STATE        = 7295;
static constexpr const int FINISHED_STATE   = 7296;
static constexpr const int CANCELED_STATE   = 7297;

static TaskSteps::TaskStep patternToState(int pattern)
{
    switch (pattern)
    {
        case PENDING_STATE: return TaskSteps::TaskStep::Pending;
        case PREPARE_STATE: return TaskSteps::TaskStep::Prepare;
        case RUN_STATE: return TaskSteps::TaskStep::Run;
        case FINISHED_STATE: return TaskSteps::TaskStep::Finished;
        case CANCELED_STATE: return TaskSteps::TaskStep::Canceled;
        default: return TaskSteps::TaskStep::Unknown;
    }
}

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
        if(values.count() < 7)
            continue;

        auto timelog = QDateTime::fromString(parseDate(values[dateIndex]), "yyyy/MM/dd HH:mm:ss.zzz").toMSecsSinceEpoch();
        auto pattern = values[patternIndex].replace("#", "").trimmed().toInt();
        auto log = values[logIndex];
        auto runner = values[runnerIndex].trimmed();

        if(log.contains(START_LOGGER) && runner.contains(SERVER_RUNNER))
        {
            if(currentRun.started())
            {
                currentRun.dump();
                currentRun = ds.createRun();
            }
            currentRun.setStartLog(timelog);
        }

        if(pattern >= PENDING_STATE && pattern <= CANCELED_STATE)
        {
            auto spliceList = log.split("]");
            assert(spliceList.count() >= 2);

            auto splittedList = spliceList[1].split(" ", QString::SkipEmptyParts);
            assert(splittedList.count() > 1);

            auto id = splittedList.first().toULongLong();

            currentRun.setTaskStepTime(id, patternToState(pattern), timelog);
            currentRun.setTaskStepRunner(id, runner);
        }
        else if(pattern == READY_STATE && runner.contains(SERVER_RUNNER))
            currentRun.setReadyLog(timelog);

        lineCpt++;

        if(lineCpt % 500 == 0)
            qInfo() << "Current line: " << lineCpt;
    }

    qInfo() << "Analysis took " << tmr.elapsed() << " ms for " << lineCpt << " lines";
    currentRun.dump();
    return 0;
}
