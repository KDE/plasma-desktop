/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workspaceoptions.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>

#include "workspaceoptions_kdeglobalssettings.h"
#include "workspaceoptions_plasmasettings.h"
#include "workspaceoptionsdata.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMWorkspaceOptionsFactory, "kcm_workspace.json", registerPlugin<KCMWorkspaceOptions>(); registerPlugin<WorkspaceOptionsData>();)

KCMWorkspaceOptions::KCMWorkspaceOptions(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new WorkspaceOptionsData(this))
{
    qmlRegisterType<WorkspaceOptionsGlobalsSettings>();
    qmlRegisterType<WorkspaceOptionsPlasmaSettings>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_workspace"),
                                       i18n("General Behavior"),
                                       QStringLiteral("1.1"),
                                       i18n("System Settings module for configuring general workspace behavior."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Furkan Tokac"), QString(), QStringLiteral("furkantokac34@gmail.com"));
    setAboutData(about);

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

void KCMWorkspaceOptions::save()
{
    ManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(3 /*KGlobalSettings::SettingsChanged*/);
    args.append(0 /*GlobalSettings::SettingsCategory::SETTINGS_MOUSE*/);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

#include "workspaceoptions.moc"
