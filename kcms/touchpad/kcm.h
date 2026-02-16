/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KPluginMetaData>
#include <KQuickConfigModule>

class TouchpadBackend;
class TouchpadConfigPlugin;

namespace MessageType
{
Q_NAMESPACE

enum MessageType {
    None,
    Information,
    Error,
};
Q_ENUM_NS(MessageType)
} // namespace MessageType

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

class KCMTouchpad : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(Message saveLoadMessage MEMBER m_saveLoadMessage NOTIFY saveLoadMessageChanged FINAL)
    Q_PROPERTY(Message hotplugMessage MEMBER m_hotplugMessage NOTIFY hotplugMessageChanged FINAL)
    Q_PROPERTY(TouchpadBackend *backend READ backend CONSTANT FINAL)
    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged FINAL)

public:
    explicit KCMTouchpad(QObject *parent, const KPluginMetaData &data);

    static void kcmInit();

    TouchpadBackend *backend() const;

    int currentDeviceIndex() const;
    void setCurrentDeviceIndex(int index);

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void onDeviceAdded(bool success);
    void onDeviceRemoved(int index);

Q_SIGNALS:
    void saveLoadMessageChanged();
    void hotplugMessageChanged();
    void currentDeviceIndexChanged();

private:
    void updateKcmNeedsSave();

    void setSaveLoadMessage(const Message &message = Message());
    void setHotplugMessage(const Message &message = Message());

    Message m_saveLoadMessage;
    Message m_hotplugMessage;
    TouchpadBackend *m_backend;
    bool m_initError = false;
    int m_currentDeviceIndex = 0;
};
