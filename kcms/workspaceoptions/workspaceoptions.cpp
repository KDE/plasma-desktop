/*
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
 *  Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "workspaceoptions.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KGlobalSettings>

#include <QDBusMessage>
#include <QDBusConnection>

#include "workspaceoptions_kdeglobalssettings.h"
#include "workspaceoptions_plasmasettings.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMWorkspaceOptionsFactory, "kcm_workspace.json", registerPlugin<KCMWorkspaceOptions>();)

KCMWorkspaceOptions::KCMWorkspaceOptions(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_globalsSettings(new WorkspaceOptionsGlobalsSettings(this))
    , m_plasmaSettings(new WorkspaceOptionsPlasmaSettings(this))
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
    return m_globalsSettings;
}

WorkspaceOptionsPlasmaSettings *KCMWorkspaceOptions::plasmaSettings() const
{
    return m_plasmaSettings;
}

void KCMWorkspaceOptions::save()
{
    ManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(KGlobalSettings::SettingsChanged);
    args.append(KGlobalSettings::SETTINGS_MOUSE);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

#include "workspaceoptions.moc"
