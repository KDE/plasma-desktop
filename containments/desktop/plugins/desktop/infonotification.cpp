/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "infonotification.h"

#include <KNotification>

InfoNotification::InfoNotification(QObject *parent)
    : QObject(parent)
    , m_enabled(false)
{
}

InfoNotification::~InfoNotification()
{
    delete m_notification;
}

bool InfoNotification::enabled() const
{
    return m_enabled;
}

void InfoNotification::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        Q_EMIT enabledChanged();
    }
}

void InfoNotification::show()
{
    if (m_enabled) {
        delete m_notification;

        m_notification = new KNotification(QStringLiteral("notification"));
        m_notification->setFlags(KNotification::DefaultEvent);
        m_notification->setIconName(m_iconName);
        m_notification->setTitle(m_titleText);
        m_notification->setText(m_text);
        m_notification->setActions(QStringList() << m_acknowledgeActionText);

        connect(m_notification.data(), &KNotification::action1Activated, this, &InfoNotification::acknowledged);

        m_notification->sendEvent();
    }
}

QString InfoNotification::iconName() const
{
    return m_iconName;
}

void InfoNotification::setIconName(const QString &iconName)
{
    if (m_iconName != iconName) {
        m_iconName = iconName;

        Q_EMIT iconNameChanged();
    }
}

QString InfoNotification::titleText() const
{
    return m_titleText;
}

void InfoNotification::setTitleText(const QString &titleText)
{
    if (m_titleText != titleText) {
        m_titleText = titleText;

        Q_EMIT titleTextChanged();
    }
}

QString InfoNotification::text() const
{
    return m_text;
}

void InfoNotification::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;

        Q_EMIT textChanged();
    }
}

QString InfoNotification::acknowledgeActionText() const
{
    return m_acknowledgeActionText;
}

void InfoNotification::setAcknowledgeActionText(const QString &acknowledgeActionText)
{
    if (m_acknowledgeActionText != acknowledgeActionText) {
        m_acknowledgeActionText = acknowledgeActionText;

        Q_EMIT acknowledgeActionTextChanged();
    }
}
