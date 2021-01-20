/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ZYPPERRPMJOB_H
#define ZYPPERRPMJOB_H

#include "AbstractJob.h"

class ZypperRPMJob : public AbstractJob
{
    Q_OBJECT

public:
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install) override;
};

#endif // ZYPPERRPMJOB_H
