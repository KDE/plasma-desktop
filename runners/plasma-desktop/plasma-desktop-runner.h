/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PLASMADESKTOPRUNNER_H
#define PLASMADESKTOPRUNNER_H

#include <krunner/abstractrunner.h>

/**
 * This class runs programs using the literal name of the binary, much as one
 * would use at a shell prompt.
 */
class PlasmaDesktopRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    PlasmaDesktopRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~PlasmaDesktopRunner() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;

private:
    const QString m_desktopConsoleKeyword;
    const QString m_kwinConsoleKeyword;
};

#endif
