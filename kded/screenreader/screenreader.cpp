/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "screenreader.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QProcess>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <csignal>

using namespace Qt::StringLiterals;

ScreenReader::ScreenReader(QObject *parent)
    : KDEDModule(parent)
{
    m_configWatcher = KConfigWatcher::create(KSharedConfig::openConfig(u"kaccessrc"_s));

    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, [this] {
        m_settings.read();

        if (m_settings.enabled()) {
            startScreenReader();
        } else {
            stopScreenReader();
        }
    });

    if (m_settings.enabled()) {
        startScreenReader();
    }
}

void ScreenReader::toggle()
{
    bool enabled = !m_settings.enabled();
    m_settings.setEnabled(enabled);
    m_settings.save();

    if (enabled) {
        startScreenReader();
    } else {
        stopScreenReader();
    }
}

void ScreenReader::startScreenReader()
{
    if (!startScreenReaderSystemd()) {
        startScreenReaderFallback();
    }
}

bool ScreenReader::startScreenReaderSystemd()
{
    if (qEnvironmentVariableIntValue("KDE_SCREENREADER_NO_SYSTEMD")) {
        return false;
    }

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

void ScreenReader::stopScreenReader()
{
    if (!stopScreenReaderSystemd()) {
        stopScreenReaderFallback();
    }
}

bool ScreenReader::stopScreenReaderSystemd()
{
    if (qEnvironmentVariableIntValue("KDE_SCREENREADER_NO_SYSTEMD")) {
        return false;
    }

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
