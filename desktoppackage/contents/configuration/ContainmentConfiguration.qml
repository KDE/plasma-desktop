/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.15 as Kirigami
import org.kde.plasma.configuration 2.0


AppletConfiguration {
    id: root
    isContainment: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 35
    Layout.minimumHeight: Kirigami.Units.gridUnit * 30
    Layout.preferredWidth: Kirigami.Units.gridUnit * 32
    Layout.preferredHeight: Kirigami.Units.gridUnit * 36
    Layout.maximumWidth: plasmoid.availableScreenRect.width
    Layout.maximumHeight: plasmoid.availableScreenRect.height

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
