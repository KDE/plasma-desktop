/*
 *   Copyright (C) 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>
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

#ifndef KWINRUNNER_H
#define KWINRUNNER_H

#include <krunner/abstractrunner.h>

class KWinRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        explicit KWinRunner(QObject *parent, const QVariantList &args);
        ~KWinRunner() override;

        void match(Plasma::RunnerContext &context) override;
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;

    private:
      void checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner);

    private:
        bool m_enabled;
};

#endif
