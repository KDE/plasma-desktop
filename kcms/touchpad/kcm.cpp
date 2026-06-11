/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"

#include "kwinwaylandbackend.h"
#include "kwinwaylandtouchpad.h"
#include "logging.h"
#include "touchpadmoduledata.h"

#include <KLocalizedContext>
#include <KLocalizedString>
#include <KPluginFactory>

#include <qqml.h>

#include <algorithm>

K_PLUGIN_FACTORY_WITH_JSON(KCMTouchpadFactory, "kcm_touchpad.json", registerPlugin<KCMTouchpad>(); registerPlugin<TouchpadModuleData>();)

Message::Message() = default;

Message::Message(MessageType::MessageType type, const QString &text)
    : type(type)
    , text(text)
{
}

Message Message::error(const QString &text)
{
    return Message(MessageType::MessageType::Error, text);
}

Message Message::information(const QString &text)
{
    return Message(MessageType::MessageType::Information, text);
}

bool Message::operator==(const Message &other) const = default;

KCMTouchpad::KCMTouchpad(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
{
    const auto uri = "org.kde.plasma.private.kcm_touchpad";
    qmlRegisterUncreatableType<KWinWaylandTouchpad>(uri, 1, 0, "InputDevice", QString());
    qmlRegisterUncreatableType<Message>(uri, 1, 0, "message", QString());
    qmlRegisterUncreatableMetaObject(MessageType::staticMetaObject, uri, 1, 0, "MessageType", QString());
    qmlRegisterUncreatableType<KCMTouchpad>(uri, 1, 0, "KCMTouchpad", QString());
    qmlRegisterUncreatableType<KWinWaylandBackend>(uri, 1, 0, "TouchpadBackend", QString());

    m_backend = KWinWaylandBackend::implementation();
    if (!m_backend) {
        m_initError = true;
        setSaveLoadMessage(Message::error(i18n("Not able to select appropriate backend")));
        qCCritical(KCM_TOUCHPAD) << "Not able to select appropriate backend.";
        return;
    }

    m_initError = !m_backend->errorString().isNull();

    if (m_initError) {
        setSaveLoadMessage(Message::error(m_backend->errorString()));
    } else {
        connect(m_backend, &KWinWaylandBackend::needsSaveChanged, this, &KCMTouchpad::updateKcmNeedsSave);
        connect(m_backend, &KWinWaylandBackend::deviceAdded, this, &KCMTouchpad::onDeviceAdded);
        connect(m_backend, &KWinWaylandBackend::deviceRemoved, this, &KCMTouchpad::onDeviceRemoved);
    }
    setCurrentDeviceIndex(0);
}

KWinWaylandBackend *KCMTouchpad::backend() const
{
    return m_backend;
}

int KCMTouchpad::currentDeviceIndex() const
{
    return m_currentDeviceIndex;
}

void KCMTouchpad::setCurrentDeviceIndex(int index)
{
    // Should be at least zero, even if there are no devices. Thus it can't be
    // a clamp() because that might crash when low is greater than high.
    index = std::max(0, std::min(index, m_backend->deviceCount() - 1));
    if (m_currentDeviceIndex != index) {
        m_currentDeviceIndex = index;
        Q_EMIT currentDeviceIndexChanged();
    }
}

void KCMTouchpad::load()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->load()) {
        setSaveLoadMessage(Message::error(i18n("Error while loading values. See logs for more information. Please restart this configuration module.")));
    }
    if (!m_backend->deviceCount()) {
        setHotplugMessage(Message::information(i18n("No pointer device found. Connect now.")));
    }
    setNeedsSave(false);
}

void KCMTouchpad::save()
{
    if (!m_backend->save()) {
        setSaveLoadMessage(
            Message::error(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again.")));
    } else {
        setSaveLoadMessage();
    }

    // load newly written values
    load();
    // in case of error, config still in changed state
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->defaults()) {
        setSaveLoadMessage(Message::error(i18n("Error while loading default values. Failed to set some options to their default values.")));
    }
}

void KCMTouchpad::updateKcmNeedsSave()
{
    if (!m_backend->isSaveNeeded()) {
        setSaveLoadMessage();
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::onDeviceAdded(bool success)
{
    if (!success) {
        setHotplugMessage(Message::error(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module.")));
        return;
    }

    if (m_backend->deviceCount() > 0) {
        setHotplugMessage();
    }
}

void KCMTouchpad::onDeviceRemoved(int index)
{
    if (m_currentDeviceIndex == index) {
        if (m_backend->deviceCount()) {
            setHotplugMessage(Message::information(i18n("Pointer device disconnected. Closed its setting dialog.")));
        } else {
            setHotplugMessage(Message::information(i18n("Pointer device disconnected. No other devices found.")));
        }
    }

    if (m_currentDeviceIndex >= index) {
        setCurrentDeviceIndex(index - 1);
    }

    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::setSaveLoadMessage(const Message &message)
{
    if (m_saveLoadMessage != message) {
        m_saveLoadMessage = message;
        Q_EMIT saveLoadMessageChanged();
    }
}

void KCMTouchpad::setHotplugMessage(const Message &message)
{
    if (m_hotplugMessage != message) {
        m_hotplugMessage = message;
        Q_EMIT hotplugMessageChanged();
    }
}

#include "kcm.moc"
#include "moc_kcm.cpp"
