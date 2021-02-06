/*
 * SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.workspace.components 2.0

KeyboardLayoutButton {
    id: root

    text: layoutNames.displayName || layoutNames.shortName
    Plasmoid.toolTipSubText: layoutNames.longName
    icon.name: iconURL(layoutNames.shortName)

    display: plasmoid.configuration.showFlag && icon.name ? AbstractButton.IconOnly : AbstractButton.TextOnly
    Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    function iconURL(name) {
        return StandardPaths.locate(StandardPaths.GenericDataLocation,
                    "kf5/locale/countries/" + name + "/flag.png")
    }

    Connections {
        target: keyboardLayout

        function onLayoutsListChanged() {
            plasmoid.clearActions()

            keyboardLayout.layoutsList.forEach(
                function(layout, index) {
                    plasmoid.setAction(
                        index,
                        layout.longName,
                        iconURL(layout.shortName).toString().substring(7) // remove file:// scheme
                    )
                }
            )
        }

        function onLayoutChanged() {
            root.Plasmoid.activated()
        }
    }

    function actionTriggered(selectedLayout) {
        keyboardLayout.layout = selectedLayout
    }

    // to fit at least 2 letters in systray
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
}
