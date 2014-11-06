/*
 *  main.h
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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
#ifndef CLOCK_HELPER_H
#define CLOCK_HELPER_H

#include <kauth.h>

using namespace KAuth;

class ClockHelper : public QObject
{
    Q_OBJECT

    public:
        enum
        {
            CallError       = 1 << 0,
            TimezoneError   = 1 << 1,
            NTPError        = 1 << 2,
            DateError       = 1 << 3
        };

    public Q_SLOTS:
        ActionReply save(const QVariantMap &map);

    private:
        int ntp(const QStringList& ntpServers, bool ntpEnabled);
        int date(const QString& newdate, const QString& olddate);
        int tz(const QString& selectedzone);
        int tzreset();

        void toHwclock();
};

#endif // CLOCK_HELPER_H
