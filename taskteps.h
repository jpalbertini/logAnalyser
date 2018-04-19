#ifndef TASKSTEPS_H
#define TASKSTEPS_H

#include <QString>

class TaskSteps
{
public:
    enum class TaskStep
    {
        Unknown,
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

    void setRunner(const QString& runner)
    {
        m_sRunner = runner;
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
        if(m_mTasksTimes.contains(TaskStep::Unknown))
            return false;
        if(!m_mTasksTimes.contains(TaskStep::Finished) && !m_mTasksTimes.contains(TaskStep::Canceled))
            return false;

        return true;
    }

private:
    qint64                  m_iId;
    QString                 m_sRunner;
    QMap<TaskStep, qint64>  m_mTasksTimes;
};

#endif // TASKSTEPS_H
