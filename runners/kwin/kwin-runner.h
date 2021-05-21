/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KWINRUNNER_H
#define KWINRUNNER_H

#include <krunner/abstractrunner.h>

class KWinRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit KWinRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KWinRunner() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;

private:
    void checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    bool m_enabled;
};

#endif
