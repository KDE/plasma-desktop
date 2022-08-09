/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workspaceoptions.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KWindowSystem>

#include <QDBusConnection>
#include <QDBusMessage>

#include "workspaceoptions_kdeglobalssettings.h"
#include "workspaceoptions_kwinsettings.h"
#include "workspaceoptions_plasmasettings.h"
#include "workspaceoptionsdata.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMWorkspaceOptionsFactory, "kcm_workspace.json", registerPlugin<KCMWorkspaceOptions>(); registerPlugin<WorkspaceOptionsData>();)

KCMWorkspaceOptions::KCMWorkspaceOptions(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
    , m_data(new WorkspaceOptionsData(this))
{
    qmlRegisterAnonymousType<WorkspaceOptionsGlobalsSettings>("org.kde.plasma.workspaceoptions.kcm", 0);
    qmlRegisterAnonymousType<WorkspaceOptionsPlasmaSettings>("org.kde.plasma.workspaceoptions.kcm", 0);
    qmlRegisterAnonymousType<WorkspaceOptionsKwinSettings>("org.kde.plasma.workspaceoptions.kcm", 0);

    setButtons(Apply | Default | Help);
}

WorkspaceOptionsGlobalsSettings *KCMWorkspaceOptions::globalsSettings() const
{
    return m_data->workspaceOptionsGlobalsSettings();
}

WorkspaceOptionsPlasmaSettings *KCMWorkspaceOptions::plasmaSettings() const
{
    return m_data->workspaceOptionsPlasmaSettings();
}

WorkspaceOptionsKwinSettings *KCMWorkspaceOptions::kwinSettings() const
{
    return m_data->workspaceOptionsKwinSettings();
}

bool KCMWorkspaceOptions::isWayland() const
{
    return KWindowSystem::isPlatformWayland();
}

void KCMWorkspaceOptions::save()
{
    ManagedConfigModule::save();

    {
        QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
        QList<QVariant> args;
        args.append(3 /*KGlobalSettings::SettingsChanged*/);
        args.append(0 /*GlobalSettings::SettingsCategory::SETTINGS_MOUSE*/);
        message.setArguments(args);
        QDBusConnection::sessionBus().send(message);
    }

    {
        QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/kwinrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
        const QHash<QString, QByteArrayList> changes = {
            {QStringLiteral("Wayland"), {"EnablePrimarySelection"}},
        };
        message.setArguments({QVariant::fromValue(changes)});
        QDBusConnection::sessionBus().send(message);
    }
}

#include "moc_workspaceoptions.cpp"
#include "workspaceoptions.moc"
