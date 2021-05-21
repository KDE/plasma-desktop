/*
    kcmsmserver.cpp
    SPDX-FileCopyrightText: 2000, 2002 Oswald Buddenhagen <ossi@kde.org>

    based on kcmtaskbar.cpp
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

    SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAction>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFileInfo>

#include <KDesktopFile>
#include <KPluginFactory>
#include <KProcess>
#include <QDBusInterface>
#include <QRegExp>
#include <kworkspace.h>

#include "kcmsmserver.h"
#include "smserverdata.h"
#include "smserversettings.h"

#include <KAboutData>
#include <KLocalizedString>

#include <sessionmanagement.h>

#include "login1_manager.h"

K_PLUGIN_FACTORY_WITH_JSON(SMServerConfigFactory, "metadata.json", registerPlugin<SMServerConfig>(); registerPlugin<SMServerData>();)

SMServerConfig::SMServerConfig(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_login1Manager(new OrgFreedesktopLogin1ManagerInterface(QStringLiteral("org.freedesktop.login1"),
                                                               QStringLiteral("/org/freedesktop/login1"),
                                                               QDBusConnection::systemBus(),
                                                               this))
{
    auto settings = new SMServerSettings(this);
    qmlRegisterSingletonInstance("org.kde.desktopsession.private", 1, 0, "Settings", settings);

    setQuickHelp(
        i18n("<h1>Session Manager</h1>"
             " You can configure the session manager here."
             " This includes options such as whether or not the session exit (logout)"
             " should be confirmed, whether the session should be restored again when logging in"
             " and whether the computer should be automatically shut down after session"
             " exit by default."));

    checkFirmwareSetupRequested();
    m_restartInSetupScreenInitial = m_restartInSetupScreen;

    KAboutData *about = new KAboutData(QStringLiteral("kcm_smserver"),
                                       i18n("Desktop Session"),
                                       QStringLiteral("1.0"),
                                       i18n("Desktop Session Login and Logout"),
                                       KAboutLicense::GPL,
                                       i18n("Copyright © 2000–2020 Desktop Session team"));

    about->addAuthor(i18n("Oswald Buddenhagen"), QString(), QStringLiteral("ossi@kde.org"));
    about->addAuthor(i18n("Carl Schwan"), QStringLiteral("QML rewrite"), QStringLiteral("carl@carlschwan.eu"), QStringLiteral("https://carlschwan.eu"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    const QString canFirmwareSetup = m_login1Manager->CanRebootToFirmwareSetup().value();
    if (canFirmwareSetup == QLatin1String("yes") || canFirmwareSetup == QLatin1String("challenge")) {
        m_canFirmwareSetup = true;
        // now check whether we're UEFI to provide a more descriptive button label
        if (QFileInfo(QStringLiteral("/sys/firmware/efi")).isDir()) {
            m_isUefi = true;
        }
    }
}

SMServerConfig::~SMServerConfig() = default;

bool SMServerConfig::isUefi() const
{
    return m_isUefi;
}

bool SMServerConfig::restartInSetupScreen() const
{
    return m_restartInSetupScreen;
}

void SMServerConfig::setRestartInSetupScreen(bool restartInSetupScreen)
{
    if (m_restartInSetupScreen == restartInSetupScreen) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(m_login1Manager->service(),
                                                          m_login1Manager->path(),
                                                          m_login1Manager->interface(),
                                                          QStringLiteral("SetRebootToFirmwareSetup"));

    message.setArguments({restartInSetupScreen});
    // This cannot be set through a generated DBus interface, so we have to create the message ourself.
    message.setInteractiveAuthorizationAllowed(true);

    QDBusPendingReply<void> call = m_login1Manager->connection().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(call, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this, restartInSetupScreen](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<void> reply = *watcher;
        watcher->deleteLater();

        checkFirmwareSetupRequested();

        settingsChanged();

        if (reply.isError()) {
            // User likely canceled the PolKit prompt, don't show an error in this case
            if (reply.error().type() != QDBusError::AccessDenied) {
                m_error = reply.error().message();
                Q_EMIT errorChanged();
            }
            return;
        } else if (m_error.length() > 0) {
            m_error = QString();
            Q_EMIT errorChanged();
        }
        m_restartInSetupScreen = restartInSetupScreen;
        Q_EMIT restartInSetupScreenChanged();

        if (!restartInSetupScreen) {
            return;
        }
    });
}

QString SMServerConfig::error() const
{
    return m_error;
}

void SMServerConfig::reboot()
{
    auto sm = new SessionManagement(this);
    auto doShutdown = [sm]() {
        sm->requestReboot();
        delete sm;
    };
    if (sm->state() == SessionManagement::State::Loading) {
        connect(sm, &SessionManagement::stateChanged, this, doShutdown);
    } else {
        doShutdown();
    }
}

void SMServerConfig::checkFirmwareSetupRequested()
{
    m_restartInSetupScreen = m_login1Manager->property("RebootToFirmwareSetup").toBool();
    Q_EMIT restartInSetupScreenChanged();
}

bool SMServerConfig::canFirmwareSetup() const
{
    return m_canFirmwareSetup;
}

bool SMServerConfig::isSaveNeeded() const
{
    return m_restartInSetupScreen != m_restartInSetupScreenInitial;
}

bool SMServerConfig::isDefaults() const
{
    return !m_restartInSetupScreen;
}

void SMServerConfig::defaults()
{
    ManagedConfigModule::defaults();
    setRestartInSetupScreen(false);
}

#include "kcmsmserver.moc"
