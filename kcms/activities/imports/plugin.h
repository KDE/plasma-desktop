/*
    SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KACTIVITIES_IMPORTS_PLUGIN_H
#define KACTIVITIES_IMPORTS_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitiesSettingsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.activities.settings")

public:
    ActivitiesSettingsPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // KACTIVITIES_IMPORTS_PLUGIN_H
