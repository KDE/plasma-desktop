/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickAddons/ManagedConfigModule>

class KCMGamePad : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

public:
    KCMGamePad(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
};
