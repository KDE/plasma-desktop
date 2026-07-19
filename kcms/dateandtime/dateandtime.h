/*
 *  SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QTimeZone>
#include <QTimer>

#include <KQuickConfigModule>

#include "message.h"

class DateAndTime : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QList<Message> messages READ messages NOTIFY messagesChanged)

    Q_PROPERTY(QString timeZone READ timeZone WRITE setTimeZone NOTIFY timeZoneChanged)

    Q_PROPERTY(bool ntpAvailable READ ntpAvailable NOTIFY ntpChanged)
    Q_PROPERTY(bool ntpEnabled READ ntpEnabled WRITE setNtpEnabled NOTIFY ntpChanged)

    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime NOTIFY dateTimeChanged)
    Q_PROPERTY(QString timeString READ timeString NOTIFY dateTimeChanged)
    Q_PROPERTY(QString dateString READ dateString NOTIFY dateTimeChanged)

public:
    DateAndTime(QObject *parent, const KPluginMetaData &data);

    [[nodiscard]] QList<Message> messages() const;

    [[nodiscard]] QString timeZone() const;
    void setTimeZone(QString timeZone);

    [[nodiscard]] bool ntpAvailable() const;
    [[nodiscard]] bool ntpEnabled() const;
    void setNtpEnabled(bool ntpEnabled);

    [[nodiscard]] QDateTime dateTime() const;
    void setDateTime(const QDateTime &dateTime);
    Q_INVOKABLE void setDate(const QDate &date);
    Q_INVOKABLE void setTime(const QTime &time);
    [[nodiscard]] QString timeString() const;
    [[nodiscard]] QString dateString() const;

    void save() override;
    void load() override;
    void refresh();

Q_SIGNALS:
    void messagesChanged();
    void timeZoneChanged();
    void ntpChanged();
    void dateTimeChanged();

    void message(Message message);

private:
    QHash<QString, Message> m_messages;
    int m_nextKey = 0;
    void addMessage(const Message &message, bool isTemporary = true, const QString &id = {});

    QTimeZone m_systemTimeZone = {};
    QTimeZone m_timeZone = {};
    QTimeZone timeZoneToUse() const;
    bool m_ntpAvailable = false;
    bool m_systemNtpEnabled = false;
    bool m_systemNtpEnabledSet = false;
    bool m_ntpEnabled = false;
    bool m_ntpEnabledSet = false;
    QDateTime m_systemDateTime = {};
    QDateTime m_dateTime = {};
    QTimer m_refreshTimer;

    void checkNeedsSave();

    bool timedateSave();
};
