#ifndef DATASTORE_H
#define DATASTORE_H

#include <QList>

#include "run.h"

class DataStore
{
public:
    Run& createRun()
    {
        Run newRun;
        newRun.setOutputFile(m_sOutputFile);
        m_lRuns += newRun;
        return m_lRuns.last();
    }

    void setOutputFile(const QString& outputFile)
    {
        m_sOutputFile = outputFile;
    }

private:
    QString     m_sOutputFile;
    QList<Run>  m_lRuns;
};

#endif // DATASTORE_H
