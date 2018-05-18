/*
 *  main.h
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *  Copyright (C) 2018 David Edmundson <davidedmundson@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#pragma once

#include <KQuickAddons/ConfigModule>
#include "backend.h"

class QTimer;

class KclockModule : public KQuickAddons::ConfigModule
{
  Q_OBJECT

    //properties reflect what user has set in the KCM, not current saved state
  Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime NOTIFY timeChanged)
  Q_PROPERTY(bool canNtp READ canNtp CONSTANT);
  Q_PROPERTY(bool ntpEnabled MEMBER m_ntpEnabled NOTIFY ntpEnabledChanged)
  Q_PROPERTY(QString timeZone MEMBER m_timeZone NOTIFY timezoneChanged)

public:
    explicit KclockModule(QObject *parent, const QVariantList &);
    void save() override;
    void load() override;

    bool canNtp() const;
    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dateTime);

signals:
    void timeChanged();
    void ntpEnabledChanged();
    void timezoneChanged();
private:
    QScopedPointer<Backend> m_backend;
    QTimer *m_clockTimer;
    bool m_ntpEnabled;
    QString m_timeZone;
    QDateTime m_dateTime;
};
