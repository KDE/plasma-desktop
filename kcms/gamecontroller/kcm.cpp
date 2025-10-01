/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include "axesmodel.h"
#include "axesproxymodel.h"
#include "buttonmodel.h"
#include "devicemodel.h"
#include "hatmodel.h"

K_PLUGIN_CLASS_WITH_JSON(KCMGameController, "kcm_gamecontroller.json")

KCMGameController::KCMGameController(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
{
    setButtons(Help | Apply);

    constexpr const char *uri{"org.kde.plasma.gamecontroller.kcm"};

    qmlRegisterType<DeviceModel>(uri, 1, 0, "DeviceModel");
    qmlRegisterType<AxesModel>(uri, 1, 0, "AxesModel");
    qmlRegisterType<AxesProxyModel>(uri, 1, 0, "AxesProxyModel");
    qmlRegisterType<ButtonModel>(uri, 1, 0, "ButtonModel");
    qmlRegisterType<HatModel>(uri, 1, 0, "HatModel");

    qmlRegisterSingletonInstance(uri, 1, 0, "KWinIntegrationPlugin", this);

    load();
}

KCMGameController::~KCMGameController()
{
}

void KCMGameController::load()
{
    KQuickManagedConfigModule::load();

    KConfigGroup plugins(KSharedConfig::openConfig("kwinrc"), QStringLiteral("Plugins"));
    m_pluginEnabled = plugins.readEntry(m_pluginId + QStringLiteral("Enabled"), false);
    Q_EMIT pluginEnabledChanged();
}

void KCMGameController::save()
{
    KQuickManagedConfigModule::save();

    KConfigGroup plugins(KSharedConfig::openConfig("kwinrc"), QStringLiteral("Plugins"));
    plugins.writeEntry(m_pluginId + QStringLiteral("Enabled"), m_pluginEnabled, KConfig::Notify);
    plugins.sync();

    QDBusMessage message =
        QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reconfigure"));
    QDBusConnection::sessionBus().asyncCall(message);
    // QDBusConnection::sessionBus().send(message);
}

bool KCMGameController::isPluginEnabled() const
{
    return m_pluginEnabled;
}

void KCMGameController::setPluginEnabled(bool enabled)
{
    if (m_pluginEnabled != enabled) {
        m_pluginEnabled = enabled;
        setNeedsSave(true);
        Q_EMIT pluginEnabledChanged();
    }
}

#include "kcm.moc"

#include "moc_kcm.cpp"
