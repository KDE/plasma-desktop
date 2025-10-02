/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickConfigModule>

class KCMGameController : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(bool pluginEnabled READ isPluginEnabled WRITE setPluginEnabled NOTIFY pluginEnabledChanged)

public:
    KCMGameController(QObject *parent, const KPluginMetaData &metaData);
    ~KCMGameController() override;

    Q_INVOKABLE bool isPluginEnabled() const;
    Q_INVOKABLE void setPluginEnabled(bool enabled);

Q_SIGNALS:
    void pluginEnabledChanged();

private:
    QString m_pluginId = QStringLiteral("gamecontroller");
};
