/*
 *   Copyright (C) 2014 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H
#define ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitySwitcherExtensionPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.plasma.activityswitcher")

public:
    explicit ActivitySwitcherExtensionPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // ACTIVITY_SWITCHER_EXTENSION_PLUGIN_H
