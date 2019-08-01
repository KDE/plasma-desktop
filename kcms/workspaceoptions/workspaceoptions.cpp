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
#include <KSharedConfig>
#include <KGlobalSettings>
#include <../migrationlib/kdelibs4config.h>

#include <QDBusMessage>
#include <QDBusConnection>

K_PLUGIN_FACTORY_WITH_JSON(KCMWorkspaceOptionsFactory, "kcm_workspace.json", registerPlugin<KCMWorkspaceOptions>();)

KCMWorkspaceOptions::KCMWorkspaceOptions(QObject *parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args),
      m_toolTipCurrentState(true),
      m_visualFeedbackCurrentState(true),
      m_singleClickCurrentState(true)
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_workspace"),
                                       i18n("General Behavior"),
                                       QStringLiteral("1.1"),
                                       i18n("System Settings module for configuring general workspace behavior."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Furkan Tokac"), QString(), QStringLiteral("furkantokac34@gmail.com"));
    setAboutData(about);

    setButtons(Default | Apply);
}

void KCMWorkspaceOptions::load()
{
    loadPlasmarc();
    loadKdeglobals();

    setNeedsSave(false);
}

void KCMWorkspaceOptions::save()
{
    savePlasmarc();
    saveKdeglobals();

    setNeedsSave(false);
}

void KCMWorkspaceOptions::defaults()
{
    setToolTip(true);
    setVisualFeedback(true);
    setSingleClick(true);

    handleNeedsSave();
}

bool KCMWorkspaceOptions::getToolTip() const
{
    return m_toolTipCurrentState;
}

void KCMWorkspaceOptions::setToolTip(bool state)
{
    // Prevent from binding loop
    if (m_toolTipCurrentState == state) {
        return;
    }

    m_toolTipCurrentState = state;

    emit toolTipChanged();
    handleNeedsSave();
}

bool KCMWorkspaceOptions::getVisualFeedback() const
{
    return m_visualFeedbackCurrentState;
}

void KCMWorkspaceOptions::setVisualFeedback(bool state)
{
    // Prevent from binding loop
    if (m_visualFeedbackCurrentState == state) {
        return;
    }

    m_visualFeedbackCurrentState = state;

    emit visualFeedbackChanged();
    handleNeedsSave();
}

bool KCMWorkspaceOptions::getSingleClick() const
{
    return m_singleClickCurrentState;
}

void KCMWorkspaceOptions::setSingleClick(bool state)
{
    // Prevent from binding loop
    if (m_singleClickCurrentState == state) {
        return;
    }

    m_singleClickCurrentState = state;

    emit singleClickChanged();
    handleNeedsSave();
}

void KCMWorkspaceOptions::loadPlasmarc()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));
    bool state;

    // Load toolTip
    {
        const KConfigGroup cg(config, QStringLiteral("PlasmaToolTips"));

        state = cg.readEntry("Delay", 0.7) > 0;
        setToolTip(state);
        m_toolTipOriginalState = state;
    }

    // Load visualFeedback
    {
        const KConfigGroup cg(config, QStringLiteral("OSD"));

        state = cg.readEntry(QStringLiteral("Enabled"), true);
        setVisualFeedback(state);
        m_visualFeedbackOriginalState = state;
    }
}

void KCMWorkspaceOptions::loadKdeglobals()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    bool state;

    // Load singleClick
    {
        const KConfigGroup cg(config, QStringLiteral("KDE"));

        state = cg.readEntry(QStringLiteral("SingleClick"), true);
        setSingleClick(state);
        m_singleClickOriginalState = state;
    }
}

void KCMWorkspaceOptions::savePlasmarc()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));
    bool state;

    // Save toolTip
    {
        KConfigGroup cg(config, QStringLiteral("PlasmaToolTips"));

        state = getToolTip();
        cg.writeEntry("Delay", state ? 0.7 : -1);
        m_toolTipOriginalState = state;
    }

    // Save visualFeedback
    {
        KConfigGroup cg(config, QStringLiteral("OSD"));

        state = getVisualFeedback();
        cg.writeEntry("Enabled", state);
        m_visualFeedbackOriginalState = state;
    }

    config->sync();
}

void KCMWorkspaceOptions::saveKdeglobals()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    bool state;

    // Save singleClick
    {
        KConfigGroup cg(config, QStringLiteral("KDE"));

        state = getSingleClick();
        cg.writeEntry("SingleClick", state);
        m_singleClickOriginalState = state;
    }

    config->sync();

    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("KDE"), "kdeglobals");

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(KGlobalSettings::SettingsChanged);
    args.append(KGlobalSettings::SETTINGS_MOUSE);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

// Checks if the current states are different than the first states. If yes, setNeedsSave(true).
void KCMWorkspaceOptions::handleNeedsSave()
{
    setNeedsSave(m_toolTipOriginalState != getToolTip() ||
            m_visualFeedbackOriginalState != getVisualFeedback() ||
            m_singleClickOriginalState != getSingleClick());
}

#include "workspaceoptions.moc"
