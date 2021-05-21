/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EMOJIERDECLARATIVEPLUGIN_H
#define EMOJIERDECLARATIVEPLUGIN_H

#include <QQmlExtensionPlugin>

class EmojierDeclarativePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.plasma.emoji")
public:
    void registerTypes(const char *uri) override;
};

#endif
