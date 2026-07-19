/*
 *  SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "dateandtime.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimeZone>
#include <qqml.h>

#include <KLocalizedString>
#include <KPluginFactory>

#include "datetime_logging.h"
#include "message.h"
#include "timedated_interface.h"

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(DateAndTime, "kcm_clock.json")

namespace
{
constexpr inline auto NtpUnavailableKey = "ntpUnavailable"_L1;
}

DateAndTime::DateAndTime(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
{
    const auto uri = "org.kde.plasma.private.kcm_clock";
    qmlRegisterUncreatableType<Message>(uri, 1, 0, "message", QString());

    connect(&m_refreshTimer, &QTimer::timeout, this, [this]() {
        refresh();
    });

    setButtons(Help | Apply);
}

QList<Message> DateAndTime::messages() const
{
    return m_messages.values();
}

void DateAndTime::addMessage(const Message &message, bool isTemporary, const QString &id)
{
    if (!isTemporary) {
        m_messages[id] = message;
        Q_EMIT messagesChanged();
        return;
    }

    const auto tempId = QString::number(m_nextKey);
    ++m_nextKey;
    m_messages[tempId] = message;
    Q_EMIT messagesChanged();
    QTimer::singleShot(5000, this, [this, tempId]() {
        m_messages.remove(tempId);
        Q_EMIT messagesChanged();
    });
}

QString DateAndTime::timeZone() const
{
    return timeZoneToUse().id();
}

QTimeZone DateAndTime::timeZoneToUse() const
{
    return m_timeZone.isValid() ? m_timeZone : m_systemTimeZone;
}

void DateAndTime::setTimeZone(QString timeZone)
{
    if (m_timeZone.id() == timeZone) {
        return;
    }
    m_timeZone = QTimeZone(timeZone.toLatin1());
    checkNeedsSave();
    Q_EMIT timeZoneChanged();
    Q_EMIT dateTimeChanged();
}

bool DateAndTime::ntpAvailable() const
{
    return m_ntpAvailable;
}

bool DateAndTime::ntpEnabled() const
{
    return m_ntpEnabledSet ? m_ntpEnabled : m_systemNtpEnabled;
}

void DateAndTime::setNtpEnabled(bool ntpEnabled)
{
    if (m_ntpEnabled == ntpEnabled) {
        return;
    }
    m_ntpEnabledSet = true;
    m_ntpEnabled = ntpEnabled;
    checkNeedsSave();
    Q_EMIT ntpChanged();

    if (m_ntpEnabled) {
        m_dateTime = QDateTime::currentDateTime();
        Q_EMIT dateTimeChanged();
    }
}

QDateTime DateAndTime::dateTime() const
{
    return m_dateTime.isValid() ? m_dateTime : m_systemDateTime;
}

void DateAndTime::setDateTime(const QDateTime &dateTime)
{
    if (m_ntpEnabled || m_dateTime == dateTime) {
        return;
    }
    m_dateTime = dateTime;
    checkNeedsSave();
    Q_EMIT dateTimeChanged();
}

void DateAndTime::setDate(const QDate &date)
{
    if (m_ntpEnabled || m_dateTime.date() == date) {
        return;
    }
    m_dateTime.setDate(date);
    checkNeedsSave();
    Q_EMIT dateTimeChanged();
}

void DateAndTime::setTime(const QTime &time)
{
    if (m_ntpEnabled || m_dateTime.time() == time) {
        return;
    }
    m_dateTime.setTime(time);
    checkNeedsSave();
    Q_EMIT dateTimeChanged();
}

QString DateAndTime::timeString() const
{
    return dateTime().toTimeZone(timeZoneToUse()).time().toString(QLocale::system().timeFormat(QLocale::ShortFormat));
}

QString DateAndTime::dateString() const
{
    return dateTime().toTimeZone(timeZoneToUse()).date().toString(QLocale::system().dateFormat(QLocale::ShortFormat));
}

void DateAndTime::checkNeedsSave()
{
    bool timeZoneChanged = m_systemTimeZone.isValid() && m_timeZone.isValid() && m_systemTimeZone != m_timeZone;
    bool ntpChanged = m_systemNtpEnabledSet && m_ntpEnabledSet && m_systemNtpEnabled != m_ntpEnabled;
    bool dateTimeChanged = m_systemDateTime.isValid() && m_dateTime.isValid() && m_systemDateTime != m_dateTime;
    setNeedsSave(timeZoneChanged || ntpChanged || dateTimeChanged);
}

void DateAndTime::save()
{
    if (timedateSave()) {
        QDBusMessage msg = QDBusMessage::createSignal("/org/kde/kcmshell_clock"_L1, "org.kde.kcmshell_clock"_L1, "clockUpdated"_L1);
        QDBusConnection::sessionBus().send(msg);
    }
    load();
}

void DateAndTime::load()
{
    m_systemTimeZone = {};
    m_timeZone = {};
    m_systemNtpEnabled = false;
    m_systemNtpEnabledSet = false;
    m_ntpEnabled = false;
    m_ntpEnabledSet = false;
    m_systemDateTime = {};
    m_dateTime = {};
    refresh();
    m_refreshTimer.start(1000);
}

void DateAndTime::refresh()
{
    if (!m_systemNtpEnabledSet || !m_ntpAvailable) {
        OrgFreedesktopTimedate1Interface timeDatedIface("org.freedesktop.timedate1"_L1, "/org/freedesktop/timedate1"_L1, QDBusConnection::systemBus());
        const auto oldNtpAvailable = std::exchange(m_ntpAvailable, timeDatedIface.canNTP());
        if (m_ntpAvailable) {
            m_messages.remove(NtpUnavailableKey);
        } else {
            addMessage(Message::error(i18n("Unable to change NTP settings")), false, NtpUnavailableKey);
        }
        const auto oldSystemNtpEnabled = std::exchange(m_systemNtpEnabled, timeDatedIface.nTP());
        m_systemNtpEnabledSet = true;
        if (oldNtpAvailable != m_ntpAvailable || oldSystemNtpEnabled != m_systemNtpEnabled) {
            Q_EMIT ntpChanged();
        }
    }

    // Reset to the current date and time
    if (m_systemDateTime.isNull()) {
        const auto oldSystemDateTime = std::exchange(m_systemDateTime, QDateTime::currentDateTime());
        if (oldSystemDateTime != m_systemDateTime) {
            Q_EMIT dateTimeChanged();
        }
    }

    // Timezone
    if (!m_systemTimeZone.isValid()) {
        const auto oldSystemTimeZone = std::exchange(m_systemTimeZone, QTimeZone::systemTimeZone());
        if (oldSystemTimeZone != m_systemTimeZone) {
            Q_EMIT timeZoneChanged();
        }
    }
}

bool DateAndTime::timedateSave()
{
    OrgFreedesktopTimedate1Interface timedateIface("org.freedesktop.timedate1"_L1, "/org/freedesktop/timedate1"_L1, QDBusConnection::systemBus());

    // final arg in each method is "user-interaction" i.e whether it's OK for polkit to ask for auth

    // we cannot send requests up front then block for all replies as we need NTP to be disabled before we can make a call to SetTime
    // timedated processes these in parallel and will return an error otherwise

    auto reply = timedateIface.SetNTP(m_ntpEnabled, true);
    reply.waitForFinished();
    if (reply.isError()) {
        if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
            addMessage(Message::error(i18n("Unable to change NTP settings")));
        } else {
            addMessage(Message::error(i18n("Permission to change NTP setting denied")));
        }
        qCWarning(KCM_CLOCK) << "Failed to enable NTP" << reply.error().name() << reply.error().message();
        return false;
    }

    if (!m_ntpEnabled) {
        qint64 timeDiff = m_dateTime.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
        //*1000 for milliseconds -> microseconds
        auto reply = timedateIface.SetTime(timeDiff * 1000, true, true);
        reply.waitForFinished();
        if (reply.isError()) {
            if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
                addMessage(Message::error(i18n("Unable to set current time")));
            } else {
                addMessage(Message::error(i18n("Permission to date and time setting denied")));
            }
            qCWarning(KCM_CLOCK) << "Failed to set current time" << reply.error().name() << reply.error().message();
            return false;
        }
    }
    if (!m_timeZone.isValid()) {
        auto reply = timedateIface.SetTimezone(m_timeZone.id(), true);
        reply.waitForFinished();
        if (reply.isError()) {
            if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
                addMessage(Message::error(i18n("Unable to set time zone")));
            } else {
                addMessage(Message::error(i18n("Permission to change time zone denied")));
            }
            qCWarning(KCM_CLOCK) << "Failed to set time zone" << reply.error().name() << reply.error().message();
            return false;
        }
    }

    return true;
}

#include "dateandtime.moc"
