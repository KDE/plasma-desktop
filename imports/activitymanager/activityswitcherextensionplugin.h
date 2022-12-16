/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQmlExtensionPlugin>

class ActivitySwitcherExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.plasma.activityswitcher")

public:
    explicit ActivitySwitcherExtensionPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};
