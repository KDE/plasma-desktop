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
    text: layout.layoutDisplayName
    Plasmoid.toolTipSubText: layout.layoutLongName
    icon.name: StandardPaths.locate(StandardPaths.GenericDataLocation,
                    "kf5/locale/countries/" + layout.layoutDisplayName + "/flag.png")

    display: plasmoid.configuration.showFlag && icon.name ? AbstractButton.IconOnly : AbstractButton.TextOnly
    Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Connections {
        target: layout
        function onLayoutsChanged() {
            plasmoid.clearActions()

            layout.layouts.forEach(
                function(layoutID) {
                    // TODO: add layoutDisplayName to layouts and lookup icon.name by index here
                    plasmoid.setAction(layoutID, layoutID /* ,icon.name */)
                }
            )
        }
    }

    function actionTriggered(selectedLayout) {
        Plasmoid.activated()
        layout.layout = selectedLayout
    }

    // to fit at least 2 letters in systray
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
}
