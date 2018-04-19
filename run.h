#ifndef RUN_H
#define RUN_H

#include <QMultiMap>
#include <QTextStream>
#include <QFile>

#include "taskteps.h"

class Run
{
public:
    void setStartLog(qint64 startLog)
    {
        m_dStartLog = startLog;
    }

    void setReadyLog(qint64 readyLog)
    {
        m_dReadyLog = readyLog;
    }

    bool started() const
    {
        return m_dStartLog != 0;
    }

    qint64 startTime() const
    {
        return m_dReadyLog - m_dStartLog;
    }

    void setTaskStepTime(qulonglong id, TaskSteps::TaskStep step, qint64 time)
    {
        m_mTaskSteps[id].setStep(step, time);
    }

    void setTaskStepRunner(qulonglong id, const QString& runner)
    {
        m_mTaskSteps[id].setID(id);
        m_mTaskSteps[id].setRunner(runner);
        m_mRunnerLastTasks[runner] = id;
    }

    qint64 takeLastRunnerId(const QString& runner)
    {
        return m_mRunnerLastTasks.take(runner);
    }

    qint64 taskLength(qulonglong id)
    {
        return m_mTaskSteps[id].length();
    }

    void setOutputFile(const QString& outputFile)
    {
        m_sOutputFile = outputFile;
    }

    void dump() const
    {
        QSharedPointer<QTextStream> stream;
        QFile dumpFile(m_sOutputFile);

        if(m_sOutputFile.isEmpty())
            stream.reset(new QTextStream(stdout));
        else
        {
            if(dumpFile.open(QIODevice::Append | QIODevice::Text))
                stream.reset(new QTextStream(&dumpFile));
            else
                stream.reset(new QTextStream(stdout));
        }

        if(stream.isNull())
            return;

        *stream << " -- Start analysis -- " << endl;
        *stream << "Start took " << startTime() << " ms" << endl;
        *stream << "Number of tasks: " << m_mTaskSteps.count() << endl;

        if(m_mTaskSteps.count() > 0)
        {
            *stream << " -- Task analysis -- " << endl;

            TaskSteps firstValid;
            for(int i = 0 ; i < m_mTaskSteps.values().count() ; i++)
            {
                if(m_mTaskSteps.values()[i].isValid())
                {
                    firstValid = m_mTaskSteps.values()[i];
                    break;
                }
            }

            qint64 mMin = firstValid.length(), mMax = firstValid.length(), mAverage = firstValid.length();
            qint64 mMinId = firstValid.id(), mMaxId = firstValid.id();
            for(const auto& taskSteps: m_mTaskSteps.values())
            {
                if(taskSteps.isValid())
                {
                    auto length = taskSteps.length();
                    if(mMin > length)
                    {
                        mMin = length;
                        mMinId = taskSteps.id();
                    }
                    else if(mMax < length)
                    {
                        mMax = length;
                        mMaxId = taskSteps.id();
                    }

                    mAverage = (mAverage + length) / 2;
                }
                else
                {
                    *stream << "Weird task: " << taskSteps.id() << endl;
                }
            }

            *stream << " -- Task Results -- " << endl;
            *stream << "Task min length: " << mMin << " ms is " << mMinId << endl;
            *stream << "Task average length: " << mAverage << " ms" << endl;
            *stream << "Task max length: " << mMax << " ms is " << mMaxId << endl;
        }

        *stream << " -- Analysis Done -- " << endl;

        *stream << endl << endl;
        stream->flush();
    }

private:
    QString                         m_sOutputFile;
    qint64                          m_dStartLog {0};
    qint64                          m_dReadyLog {0};
    QMap<qulonglong, TaskSteps>     m_mTaskSteps;
    QMap<QString, qulonglong>       m_mRunnerLastTasks;
};

#endif // RUN_H
