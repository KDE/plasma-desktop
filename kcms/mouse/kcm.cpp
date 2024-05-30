/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"
#include "cursortheme.h"
#include "inputbackend.h"
#include "logging.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowSystem>

#include <QQmlProperty>
#include <QQuickItem>
#include <qqml.h>

#include <memory>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(KCMMouse, "kcm_mouse.json")

extern "C" {
Q_DECL_EXPORT void kcminit()
{
    std::unique_ptr<InputBackend> backend(InputBackend::implementation());
    if (backend) {
        backend->kcmInit();
    }

#if BUILD_KCM_MOUSE_X11
    if (KWindowSystem::isPlatformX11()) {
        auto config = KSharedConfig::openConfig(u"kcminputrc"_s, KConfig::NoGlobals);
        KConfigGroup group = config->group(QStringLiteral("Mouse"));
        const QString theme = group.readEntry("cursorTheme", QStringLiteral("breeze_cursors"));
        const int size = group.readEntry("cursorSize", 24);

        // Note: If you update this code, update kapplymousetheme as well.

        CursorTheme::applyCursorTheme(theme, size);
    }
#endif
}
}

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

KCMMouse::KCMMouse(QObject *parent, const KPluginMetaData &data, [[maybe_unused]] const QVariantList &args)
    : KQuickManagedConfigModule(parent, data)
{
    const auto uri = "org.kde.plasma.private.kcm_mouse";
    qmlRegisterUncreatableType<Message>(uri, 1, 0, "message", QString());
    qmlRegisterUncreatableMetaObject(MessageType::staticMetaObject, uri, 1, 0, "MessageType", QString());
    qmlRegisterUncreatableType<KCMMouse>(uri, 1, 0, "KCMMouse", QString());
    qmlRegisterUncreatableType<InputBackend>(uri, 1, 0, "InputBackend", QString());
    qmlRegisterUncreatableType<InputDevice>(uri, 1, 0, "InputDevice", QString());
    InputBackend::registerImplementationTypes(uri);

    m_inputBackend.reset(InputBackend::implementation());

    if (!m_inputBackend) {
        qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
        return;
    }

    m_initError = !m_inputBackend->errorString().isNull();

    if (m_initError) {
        setMessage(Message::error(m_inputBackend->errorString()));
    } else {
        connect(m_inputBackend.get(), &InputBackend::deviceAdded, this, &KCMMouse::onDeviceAdded);
        connect(m_inputBackend.get(), &InputBackend::deviceRemoved, this, &KCMMouse::onDeviceRemoved);
    }
    setCurrentDeviceIndex(0);
}

const Message &KCMMouse::message() const
{
    return m_message;
}

InputBackend *KCMMouse::inputBackend() const
{
    return m_inputBackend.get();
}

int KCMMouse::currentDeviceIndex() const
{
    return m_currentDeviceIndex;
}

void KCMMouse::setCurrentDeviceIndex(int index)
{
    // Should be at least zero, even if there are no devices. Thus it can't be
    // a clamp() because that might crash when low is greater than high.
    index = std::max(0, std::min(index, m_inputBackend->deviceCount() - 1));
    if (m_currentDeviceIndex != index) {
        m_currentDeviceIndex = index;
        Q_EMIT currentDeviceIndexChanged();
    }
}

bool KCMMouse::isSaveNeeded() const
{
    return m_inputBackend->isSaveNeeded();
}

bool KCMMouse::isDefaults() const
{
    return false;
}

void KCMMouse::load()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_inputBackend->load()) {
        setMessage(Message::error(i18n("Error while loading values. See logs for more information. Please restart this configuration module.")));
    } else {
        if (!m_inputBackend->deviceCount()) {
            setMessage(Message::information(i18n("No pointer device found. Connect now.")));
        }
    }
    setNeedsSave(false);
}

void KCMMouse::save()
{
    if (!m_inputBackend->save()) {
        setMessage(
            Message::error(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again.")));
    } else {
        setMessage();
    }
    // load newly written values
    load();
    // in case of error, config still in changed state
    setNeedsSave(m_inputBackend->isSaveNeeded());
}

void KCMMouse::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_inputBackend->defaults()) {
        setMessage(Message::error(i18n("Error while loading default values. Failed to set some options to their default values.")));
    }

    setNeedsSave(m_inputBackend->isSaveNeeded());
}

void KCMMouse::checkForChanges()
{
    if (m_inputBackend->deviceCount() > 0) {
        setMessage();
    }
    setNeedsSave(m_inputBackend->isSaveNeeded());
}

void KCMMouse::onDeviceAdded(bool success)
{
    if (!success) {
        setMessage(Message::error(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module.")));
        return;
    }

    if (m_inputBackend->deviceCount() == 1) {
        setMessage();
    }
}

void KCMMouse::onDeviceRemoved(int index)
{
    if (m_currentDeviceIndex == index) {
        QString text;
        if (m_inputBackend->deviceCount()) {
            text = i18n("Pointer device disconnected. Closed its setting dialog.");
        } else {
            text = i18n("Pointer device disconnected. No other devices found.");
        }
        setMessage(Message::information(text));
    }

    if (m_currentDeviceIndex >= index) {
        setCurrentDeviceIndex(index - 1);
    }

    setNeedsSave(m_inputBackend->isSaveNeeded());
}

void KCMMouse::setMessage(const Message &message)
{
    if (m_message != message) {
        m_message = message;
        Q_EMIT messageChanged();
    }
}

#include "kcm.moc"
#include "moc_kcm.cpp"
