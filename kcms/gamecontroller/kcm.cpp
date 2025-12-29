/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2025 Yelsin Sepulveda <yelsin.sepulveda@kdemail.org>

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
    : KQuickConfigModule(parent, metaData)
{
    setButtons(Help);

    constexpr const char *uri{"org.kde.plasma.gamecontroller.kcm"};

    qmlRegisterType<DeviceModel>(uri, 1, 0, "DeviceModel");
    qmlRegisterType<AxesModel>(uri, 1, 0, "AxesModel");
    qmlRegisterType<AxesProxyModel>(uri, 1, 0, "AxesProxyModel");
    qmlRegisterType<ButtonModel>(uri, 1, 0, "ButtonModel");
    qmlRegisterType<HatModel>(uri, 1, 0, "HatModel");
    qmlRegisterSingletonInstance(uri, 1, 0, "KWinPlugin", this);
}

KCMGameController::~KCMGameController()
{
}

bool KCMGameController::isPluginEnabled() const
{
    KConfigGroup plugins(KSharedConfig::openConfig("kwinrc"), QStringLiteral("Plugins"));
    return plugins.readEntry(m_pluginId + QStringLiteral("Enabled"), true);
}

void KCMGameController::setPluginEnabled(bool enabled)
{
    KConfigGroup plugins(KSharedConfig::openConfig("kwinrc"), QStringLiteral("Plugins"));
    plugins.writeEntry(m_pluginId + QStringLiteral("Enabled"), enabled, KConfig::Notify);
    plugins.sync();

    QDBusMessage message;
    if (enabled) {
        message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                 QStringLiteral("/Plugins"),
                                                 QStringLiteral("org.kde.KWin.Plugins"),
                                                 QStringLiteral("LoadPlugin"));
        message << m_pluginId;
    } else {
        message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                 QStringLiteral("/Plugins"),
                                                 QStringLiteral("org.kde.KWin.Plugins"),
                                                 QStringLiteral("UnloadPlugin"));
        message << m_pluginId;
    }
    QDBusConnection::sessionBus().asyncCall(message);
    Q_EMIT pluginEnabledChanged();
}

#include "kcm.moc"

#include "moc_kcm.cpp"
