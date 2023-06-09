/*
    "Desktop Icons Options" Tab for KDesktop configuration

    SPDX-FileCopyrightText: 1996 Martin R. Jones
    SPDX-FileCopyrightText: 1998, 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

// The "Path" Tab contains :
// The paths for Desktop and Documents

class DesktopPathsData;
class DesktopPathsSettings;

class DesktopPathConfig : public KQuickManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QObject *settings READ settings CONSTANT)

public:
    DesktopPathConfig(QObject *parent, const KPluginMetaData &metaData);
    ~DesktopPathConfig() override;

    QObject *settings() const;

    bool isDefaults() const override;

private:
    DesktopPathsData *const m_data;
};
