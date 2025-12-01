/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.configuration
import org.kde.plasma.plasmoid

AppletConfiguration {
    id: root
    isContainment: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 35
    Layout.minimumHeight: Kirigami.Units.gridUnit * 30
    Layout.preferredWidth: Kirigami.Units.gridUnit * 32
    Layout.preferredHeight: Kirigami.Units.gridUnit * 36

//BEGIN model
    globalConfigModel: globalContainmentConfigModel

    ConfigModel {
        id: globalContainmentConfigModel
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Wallpaper")
            icon: "preferences-desktop-wallpaper"
            source: "ConfigurationContainmentAppearance.qml"
        }
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Mouse Actions")
            icon: "preferences-desktop-mouse"
            source: "ConfigurationContainmentActions.qml"
        }
    }
//END model

}
