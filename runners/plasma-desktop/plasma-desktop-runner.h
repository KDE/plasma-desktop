/*
 *   Copyright (C) 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
        PlasmaDesktopRunner(QObject *parent, const QVariantList &args);
        ~PlasmaDesktopRunner() override;

        void match(Plasma::RunnerContext &context) override;
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;

    private Q_SLOTS:
      void checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner);

    private:
        const QString m_desktopConsoleKeyword;
        const QString m_kwinConsoleKeyword;
        bool m_enabled;
};

#endif
