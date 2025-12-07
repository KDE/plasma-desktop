/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18n("General") // qmllint disable unqualified
         icon: "preferences-desktop-plasma"
         source: "configGeneral.qml"
    }
}
