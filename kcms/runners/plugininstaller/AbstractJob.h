/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ABSTRACTJOB_H
#define ABSTRACTJOB_H

#include <QObject>

class QFileInfo;

class AbstractJob : public QObject
{
    Q_OBJECT

public:
    /**
     * @param fileInfo QFileInfo of the file or directory
     * @param mimeType Mime type of the file
     * @param install Set to true if the entry should be installed, flase if it should be uninstalled
     */
    virtual void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install) = 0;

Q_SIGNALS:
    void finished();
    void error(const QString &errorMessage);

protected:
    void runScriptInTerminal(const QString &script, const QString &pwd);
    QString terminalCloseMessage(bool install);
};

#endif // ABSTRACTJOB_H
