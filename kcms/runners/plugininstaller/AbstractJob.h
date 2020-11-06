/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ABSTRACTJOB_H
#define ABSTRACTJOB_H

#include <QObject>

class QFileInfo;
class QProcess;

enum class Operation {
    Install,
    Uninstall
};

class AbstractJob: public QObject
{
Q_OBJECT

public:
    virtual void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, Operation operation) = 0;

    void runScriptInTerminal(const QString &script, const QString &pwd);
    QString terminalCloseMessage(Operation operation);

Q_SIGNALS:
    void finished();
    void error(const QString &errorMessage);

protected:
    void connectSignals(QProcess *process);
};

#endif // ABSTRACTJOB_H
