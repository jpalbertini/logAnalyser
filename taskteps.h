#ifndef TASKSTEPS_H
#define TASKSTEPS_H

#include <QString>

class TaskSteps
{
public:
    enum class TaskRunner
    {
        Server,
        Slave
    };

    enum class TaskStep
    {
        Unknown,
        ServerStart,
        Pending,
        Prepare,
        Run,
        Finished,
        Canceled,
    };

    void setID(qulonglong id)
    {
        m_iId = id;
    }

    void setStep(TaskStep step, qint64 time)
    {
        m_mTasksTimes[step] = time;
    }

    void setRunner(TaskRunner runner)
    {
        m_eRunner = runner;
    }

    void setRunnerName(const QString& runner)
    {
        m_sRunnerName = runner;
    }

    qint64 id() const
    {
        return m_iId;
    }

    qint64 length() const
    {
        return m_mTasksTimes[TaskStep::Finished] - m_mTasksTimes[TaskStep::Pending];
    }

    bool isValid() const
    {
        /* Don't have unknown states */
        if(m_mTasksTimes.contains(TaskStep::Unknown))
            return false;

        /* Slave queries must have a server start */
        if(m_eRunner == TaskRunner::Slave && !m_mTasksTimes.contains(TaskStep::ServerStart))
            return false;

        /* Queries must have an end */
        if(!m_mTasksTimes.contains(TaskStep::Finished) && !m_mTasksTimes.contains(TaskStep::Canceled))
            return false;

        /* 5 Steps for a slave task */
        if(m_eRunner == TaskRunner::Slave && m_mTasksTimes.count() != 5)
            return false;

        /* 4 Steps for a server task */
        if(m_eRunner == TaskRunner::Server && m_mTasksTimes.count() != 4)
            return false;

        return true;
    }

private:
    qint64                  m_iId;
    TaskRunner              m_eRunner;
    QString                 m_sRunnerName;
    QMap<TaskStep, qint64>  m_mTasksTimes;
};

#endif // TASKSTEPS_H
