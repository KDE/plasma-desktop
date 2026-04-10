/*
 *  SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

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
    void refresh(bool forceNtp = false);

Q_SIGNALS:
    void messagesChanged();
    void timeZoneChanged();
    void ntpChanged();
    void dateTimeChanged();

    void message(Message message);

private:
    void initialize();

    QHash<QString, Message> m_messages;
    int m_nextKey = 0;
    void addMessage(const Message &message, bool isTemporary = true, const QString &id = {});

    QString m_timeZone;
    bool m_timeZoneModified = false;
    bool m_ntpAvailable = false;
    bool m_ntpEnabled = false;
    bool m_ntpEnabledModified = false;
    QDateTime m_dateTime;
    bool m_dateTimeModified = false;
    QTimer m_refreshTimer;

    bool timedateSave();
};
