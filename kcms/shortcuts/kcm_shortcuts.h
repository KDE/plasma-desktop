/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include <KQuickConfigModule>

class KCMShortcuts : public KQuickConfigModule
{
    Q_OBJECT

public:
    KCMShortcuts(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
};
