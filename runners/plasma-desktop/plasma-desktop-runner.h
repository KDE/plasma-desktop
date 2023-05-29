/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <KRunner/AbstractRunner>

using namespace KRunner;

/**
 * This class runs programs using the literal name of the binary, much as one
 * would use at a shell prompt.
 */
class PlasmaDesktopRunner : public AbstractRunner
{
    Q_OBJECT

public:
    explicit PlasmaDesktopRunner(QObject *parent, const KPluginMetaData &metaData);

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &action) override;

private:
    const QString m_desktopConsoleKeyword;
    const QString m_kwinConsoleKeyword;
};
