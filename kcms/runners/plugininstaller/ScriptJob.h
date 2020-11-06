/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SCRIPTJOB_H
#define SCRIPTJOB_H

#include "AbstractJob.h"

class ScriptJob : public AbstractJob
{
Q_OBJECT

public:
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, Operation operation) override;

private:
    QString formatScriptCommand(Operation operation, const QString &installerPath);
};

#endif //SCRIPTJOB_H
