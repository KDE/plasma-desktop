/*
    SPDX-FileCopyrightText: 2012, 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.plasmoid 2.0

Control {
    background: PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        anchors {
            fill: parent
            leftMargin: -backgroundFrame.margins.left
            topMargin: -backgroundFrame.margins.top
            rightMargin: -backgroundFrame.margins.right
            bottomMargin: -backgroundFrame.margins.bottom
        }
        imagePath: "widgets/background"
    }
    contentItem: Flow {
        spacing: PlasmaCore.Units.smallSpacing

        PlasmaComponents3.ToolButton {
            property QtObject qAction: plasmoid.action("add widgets")
            text: qAction.text
            icon.name: "list-add"
            onClicked: qAction.trigger()
        }
        PlasmaComponents3.ToolButton {
            property QtObject qAction: plasmoid.action("configure")
            text: qAction.text
            icon.name: "preferences-desktop-wallpaper"
            onClicked: qAction.trigger()
        }
        PlasmaComponents3.ToolButton {
            text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Choose Global Theme…")
            icon.name: "preferences-desktop-theme-global"
            onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_lookandfeel")
        }
        PlasmaComponents3.ToolButton {
            text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Configure Display Settings…")
            icon.name: "preferences-desktop-display"
            onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_kscreen")
        }
        PlasmaComponents3.ToolButton {
            icon.name: "window-close"
            Layout.preferredWidth: height
            onClicked: plasmoid.editMode = false
            PlasmaComponents3.ToolTip {
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Exit Edit Mode")
            }
        }
    }
}
