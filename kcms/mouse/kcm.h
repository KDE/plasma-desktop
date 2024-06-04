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
    Q_PROPERTY(Message saveLoadMessage MEMBER m_saveLoadMessage NOTIFY saveLoadMessageChanged FINAL)
    Q_PROPERTY(Message hotplugMessage MEMBER m_hotplugMessage NOTIFY hotplugMessageChanged FINAL)
    Q_PROPERTY(InputBackend *inputBackend READ inputBackend CONSTANT FINAL)
    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged FINAL)

public:
    explicit KCMMouse(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    InputBackend *inputBackend() const;

    int currentDeviceIndex() const;
    void setCurrentDeviceIndex(int index);

    bool isSaveNeeded() const override;
    bool isDefaults() const override;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void updateKcmNeedsSave();
    void onDeviceAdded(bool success);
    void onDeviceRemoved(int index);

Q_SIGNALS:
    void saveLoadMessageChanged();
    void hotplugMessageChanged();
    void currentDeviceIndexChanged();

private:
    void setSaveLoadMessage(const Message &message = Message());
    void setHotplugMessage(const Message &message = Message());

    Message m_saveLoadMessage;
    Message m_hotplugMessage;
    std::unique_ptr<InputBackend> m_inputBackend;
    bool m_initError = false;
    int m_currentDeviceIndex = 0;
};
