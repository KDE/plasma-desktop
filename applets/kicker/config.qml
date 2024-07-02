/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18n("General")
         icon: "preferences-desktop-plasma"
         source: "ConfigGeneral.qml"
    }
}
