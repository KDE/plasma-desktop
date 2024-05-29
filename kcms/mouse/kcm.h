/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>
#include <memory>

class KMessageWidget;
class InputBackend;

namespace MessageType
{
Q_NAMESPACE

enum MessageType {
    None,
    Information,
    Error,
};
Q_ENUM_NS(MessageType)
};

class Message
{
    Q_GADGET
    Q_PROPERTY(QString text MEMBER text FINAL)
    Q_PROPERTY(MessageType::MessageType type MEMBER type FINAL)

public:
    explicit Message();
    explicit Message(MessageType::MessageType type, const QString &text);

    static Message error(const QString &text);
    static Message information(const QString &text);

    bool operator==(const Message &other) const;

    MessageType::MessageType type = MessageType::MessageType::None;
    QString text;
};

class KCMMouse : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(Message message READ message NOTIFY messageChanged FINAL)
    Q_PROPERTY(InputBackend *inputBackend READ inputBackend CONSTANT FINAL)

public:
    explicit KCMMouse(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    const Message &message() const;
    InputBackend *inputBackend() const;

    bool isSaveNeeded() const override;
    bool isDefaults() const override;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

    void checkForChanges();

private Q_SLOTS:
    void onDeviceAdded(bool success);
    void onDeviceRemoved(int index);

Q_SIGNALS:
    void messageChanged();

private:
    void setMessage(const Message &message = Message());

    Message m_message;
    std::unique_ptr<InputBackend> m_inputBackend;
    bool m_initError = false;
};
