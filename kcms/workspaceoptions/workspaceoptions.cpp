/*
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
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
#include <KConfigGroup>
#include <ksharedconfig.h>
#include <KDELibs4Support/KDE/KGlobalSettings>
#include <../migrationlib/kdelibs4config.h>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

K_PLUGIN_FACTORY_WITH_JSON(KCMWorkspaceOptionsFactory, "kcm_workspace.json", registerPlugin<KCMWorkspaceOptions>();)

KCMWorkspaceOptions::KCMWorkspaceOptions(QObject *parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args),
      m_stateToolTip(true),
      m_stateVisualFeedback(true),
      m_stateSingleClick(true)
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_workspace"),
                    i18n("Plasma Workspace global options"),
                    QStringLiteral("1.1"),
                    i18n("System Settings module for managing global options for the Plasma Workspace."),
                    KAboutLicense::GPL);

    about->addAuthor(i18n("Furkan Tokac"), QString(), QStringLiteral("furkantokac34@gmail.com"));
    setAboutData(about);

    setButtons( Default | Apply );
}

/*ConfigModule functions*/
void KCMWorkspaceOptions::load()
{
    bool state = false;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));

    // Load toolTip
    {
        const KConfigGroup cg(config, QStringLiteral("PlasmaToolTips"));
        state = cg.readEntry("Delay", 0.7) > 0;
        setToolTip(state);
        m_ostateToolTip = getToolTip();
    }

    // Load visualFeedback
    {
        const KConfigGroup cg(config, QStringLiteral("OSD"));
        state = cg.readEntry(QStringLiteral("Enabled"), true);
        setVisualFeedback(state);
        m_ostateVisualFeedback = getVisualFeedback();
    }

    // Load singleClick
    {
        KSharedConfig::Ptr configKdeGlobals = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
        const KConfigGroup cg(configKdeGlobals, QStringLiteral("KDE"));
        state = cg.readEntry(QStringLiteral("SingleClick"), true);
        setSingleClick(state);
        m_ostateSingleClick = getSingleClick();
    }

    setNeedsSave(false);
}

void KCMWorkspaceOptions::save()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));

    // Save toolTip
    {
        KConfigGroup cg(config, QStringLiteral("PlasmaToolTips"));
        cg.writeEntry("Delay", getToolTip() ? 0.7 : -1);
        m_ostateToolTip = getToolTip();
    }

    // Save visualFeedback
    {
        KConfigGroup cg(config, QStringLiteral("OSD"));
        cg.writeEntry("Enabled", getVisualFeedback());
        m_ostateVisualFeedback = getVisualFeedback();
    }

    // Save singleClick
    KSharedConfig::Ptr configKdeGlobals = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfigGroup cg(configKdeGlobals, QStringLiteral("KDE"));
    cg.writeEntry("SingleClick", getSingleClick());
    m_ostateSingleClick = getSingleClick();
    config->sync();
    configKdeGlobals->sync();

    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("KDE"), "kdeglobals");

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(KGlobalSettings::SettingsChanged);
    args.append(KGlobalSettings::SETTINGS_MOUSE);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);

    setNeedsSave(false);
}

void KCMWorkspaceOptions::defaults()
{
    setToolTip(true);
    setVisualFeedback(true);
    setSingleClick(true);

    handleNeedsSave();
}

/*ToolTip functions*/
bool KCMWorkspaceOptions::getToolTip() const
{
    return m_stateToolTip;
}

void KCMWorkspaceOptions::setToolTip(bool state)
{
    // Prevent from binding loop
    if( m_stateToolTip == state ) {
        return;
    }

    m_stateToolTip = state;

    emit toolTipChanged();
    handleNeedsSave();
}

/*VisualFeedback functions*/
bool KCMWorkspaceOptions::getVisualFeedback() const
{
    return m_stateVisualFeedback;
}

void KCMWorkspaceOptions::setVisualFeedback(bool state)
{
    // Prevent from binding loop
    if( m_stateVisualFeedback == state ) {
        return;
    }

    m_stateVisualFeedback = state;

    emit visualFeedbackChanged();
    handleNeedsSave();
}

/*SingleClick functions*/
bool KCMWorkspaceOptions::getSingleClick() const
{
    return m_stateSingleClick;
}

void KCMWorkspaceOptions::setSingleClick(bool state)
{
    // Prevent from binding loop
    if( m_stateSingleClick == state ) {
        return;
    }

    m_stateSingleClick = state;

    emit singleClickChanged();
    handleNeedsSave();
}

/*Other functions*/
// Checks if the current states are different than the first states.
// If yes, setNeedsSave(true).
void KCMWorkspaceOptions::handleNeedsSave()
{
    setNeedsSave(m_ostateToolTip != getToolTip() ||
            m_ostateVisualFeedback != getVisualFeedback() ||
            m_ostateSingleClick != getSingleClick());
}

#include "workspaceoptions.moc"
