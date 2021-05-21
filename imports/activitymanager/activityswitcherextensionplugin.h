/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H
#define ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitySwitcherExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.plasma.activityswitcher")

public:
    explicit ActivitySwitcherExtensionPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H
