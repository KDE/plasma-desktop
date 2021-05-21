/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0


AppletConfiguration {
    id: root
    isContainment: true
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 30
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 20
    Layout.preferredWidth: PlasmaCore.Units.gridUnit * 32
    Layout.preferredHeight: PlasmaCore.Units.gridUnit * 36
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
