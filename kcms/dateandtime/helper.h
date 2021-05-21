/*
    SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLOCK_HELPER_H
#define CLOCK_HELPER_H

#include <KAuth>

using namespace KAuth;

class ClockHelper : public QObject
{
    Q_OBJECT

public:
    enum { CallError = 1 << 0, TimezoneError = 1 << 1, NTPError = 1 << 2, DateError = 1 << 3 };

public Q_SLOTS:
    ActionReply save(const QVariantMap &map);

private:
    int ntp(const QStringList &ntpServers, bool ntpEnabled);
    int date(const QString &newdate, const QString &olddate);
    int tz(const QString &selectedzone);
    int tzreset();

    void toHwclock();
};

#endif // CLOCK_HELPER_H
