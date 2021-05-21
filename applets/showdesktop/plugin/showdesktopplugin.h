/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHOWDESKTOPPLUGIN_H
#define SHOWDESKTOPPLUGIN_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class ShowDesktopPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // SHOWDESKTOPPLUGIN_H
