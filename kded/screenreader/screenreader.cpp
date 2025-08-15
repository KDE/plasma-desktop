/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "screenreader.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QProcess>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <csignal>

#include "logging.h"

using namespace Qt::StringLiterals;

bool isServiceEnabled()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s,
                                                      u"/org/freedesktop/systemd1/unit/orca_2eservice"_s,
                                                      u"org.freedesktop.DBus.Properties"_s,
                                                      u"Get"_s);
    msg.setArguments({u"org.freedesktop.systemd1.Unit"_s, u"UnitFileState"_s});
    QDBusReply<QDBusVariant> reply = QDBusConnection::sessionBus().call(msg);

    if (!reply.isValid()) {
        qCWarning(KDED_SCREENREADER) << "Failed to query Orca unit file state" << reply.error().message();
    }

    const QString state = reply.value().variant().toString();

    return state == u"enabled";
}

void enableService()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s,
                                                      u"/org/freedesktop/systemd1"_s,
                                                      u"org.freedesktop.systemd1.Manager"_s,
                                                      u"EnableUnitFiles"_s);
    msg.setArguments({QStringList{u"orca.service"_s}, false, false});
    auto reply = QDBusConnection::sessionBus().call(msg);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(KDED_SCREENREADER) << "Failed to enable Orca unit" << reply.errorMessage();
    }
}

void disableService()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s,
                                                      u"/org/freedesktop/systemd1"_s,
                                                      u"org.freedesktop.systemd1.Manager"_s,
                                                      u"DisableUnitFiles"_s);
    msg.setArguments({QStringList{u"orca.service"_s}, false});
    auto reply = QDBusConnection::sessionBus().call(msg);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(KDED_SCREENREADER) << "Failed to disable Orca unit" << reply.errorMessage();
    }
}

bool isOrcaRunning()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s,
                                                      u"/org/freedesktop/systemd1/unit/orca_2eservice"_s,
                                                      u"org.freedesktop.DBus.Properties"_s,
                                                      u"Get"_s);
    msg.setArguments({u"org.freedesktop.systemd1.Unit"_s, u"ActiveState"_s});
    QDBusReply<QDBusVariant> reply = QDBusConnection::sessionBus().call(msg);

    if (!reply.isValid()) {
        qCWarning(KDED_SCREENREADER) << "Failed to query Orca unit state" << reply.error().message();
    }

    const QString state = reply.value().variant().toString();

    return state == u"active";
}

ScreenReader::ScreenReader(QObject *parent)
    : KDEDModule(parent)
{
    if (!m_settings.systemdManaged() && QDBusConnection::sessionBus().interface()->isServiceRegistered(u"org.freedesktop.systemd1"_s)) {
        // migrate to systemd managed
        m_settings.setSystemdManaged(true);
        m_settings.config()->group(u"ScreenReader"_s).deleteEntry("Enabled");
        m_settings.save();

        if (m_settings.enabled()) {
            startScreenReaderSystemd();
        } else {
            // if it was enabled externally keep it enabled
        }
    }

    if (!m_settings.systemdManaged() && m_settings.enabled()) {
        startScreenReaderFallback();
    }
}

void ScreenReader::toggle()
{
    if (m_settings.systemdManaged()) {
        if (isOrcaRunning()) {
            stopScreenReaderSystemd();
        } else {
            startScreenReaderSystemd();
        }
    } else {
        bool enabled = !m_settings.enabled();
        m_settings.setEnabled(enabled);
        m_settings.save();

        if (enabled) {
            startScreenReaderFallback();
        } else {
            stopScreenReaderFallback();
        }
    }
}

bool ScreenReader::enabled() const
{
    if (m_settings.systemdManaged()) {
        return isServiceEnabled();
    }

    return m_settings.enabled();
}

bool ScreenReader::enabledByDefault() const
{
    if (m_settings.systemdManaged()) {
        return false; // FIXME read unit preset?
    }

    return m_settings.defaultEnabledValue();
}

void ScreenReader::setEnabled(bool enabled)
{
    if (m_settings.systemdManaged()) {
        if (enabled) {
            startScreenReaderSystemd();
        } else {
            stopScreenReaderSystemd();
        }
    } else {
        m_settings.setEnabled(enabled);

        if (enabled) {
            startScreenReaderFallback();
        } else {
            stopScreenReaderFallback();
        }
    }
}

bool ScreenReader::startScreenReaderSystemd()
{
    enableService();

    auto msg =
        QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s, u"/org/freedesktop/systemd1"_s, u"org.freedesktop.systemd1.Manager"_s, u"StartUnit"_s);
    msg.setArguments({u"orca.service"_s, u"replace"_s});
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);

    return reply.isValid();
}

void ScreenReader::startScreenReaderFallback()
{
    qint64 pid = -1;
    QProcess::startDetached(QStringLiteral("orca"), {QStringLiteral("--replace")}, QString(), &pid);

    if (pid != -1) {
        m_orcaPid = pid;
    }
}

bool ScreenReader::stopScreenReaderSystemd()
{
    disableService();

    auto msg =
        QDBusMessage::createMethodCall(u"org.freedesktop.systemd1"_s, u"/org/freedesktop/systemd1"_s, u"org.freedesktop.systemd1.Manager"_s, u"StopUnit"_s);
    msg.setArguments({u"orca.service"_s, u"replace"_s});
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(msg);

    return reply.isValid();
}

void ScreenReader::stopScreenReaderFallback()
{
    if (m_orcaPid) {
        kill(m_orcaPid.value(), SIGTERM);
    }
}

K_PLUGIN_CLASS_WITH_JSON(ScreenReader, "metadata.json")

#include "screenreader.moc"
