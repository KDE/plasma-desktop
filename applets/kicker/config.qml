/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title:group for configuration dialog page", "General")
         icon: "preferences-desktop-plasma"
         source: "ConfigGeneral.qml"
    }
}
