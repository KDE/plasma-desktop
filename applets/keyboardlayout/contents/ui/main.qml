/*
 * SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.workspace.components 2.0

KeyboardLayoutButton {
    text: layout.layoutDisplayName
    Plasmoid.toolTipSubText: layout.layoutLongName
    // TODO: add flag support
    icon.name: ""

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    Connections {
        target: layout
        function onLayoutsChanged() {
            plasmoid.clearActions()

            layout.layouts.forEach(
                function(layoutID) {
                    plasmoid.setAction(layoutID, layoutID)
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
