/*
 *  SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

class Message
{
    Q_GADGET
    QML_VALUE_TYPE(message)

    Q_PROPERTY(QString text MEMBER text FINAL)
    Q_PROPERTY(MessageType type MEMBER type FINAL)

public:
    enum MessageType {
        None,
        Information,
        Error,
    };
    Q_ENUM(MessageType)

    explicit Message() = default;
    explicit Message(MessageType type, const QString &text)
        : type(type)
        , text(text)
    {
    }

    static Message error(const QString &text)
    {
        return Message(Error, text);
    }

    static Message information(const QString &text)
    {
        return Message(Information, text);
    }

    bool operator==(const Message &other) const = default;

    MessageType type = None;
    QString text = {};
};
