/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

class KCMGameController : public KQuickManagedConfigModule
{
    Q_OBJECT

public:
    KCMGameController(QObject *parent, const KPluginMetaData &metaData);
    ~KCMGameController() override;
};
