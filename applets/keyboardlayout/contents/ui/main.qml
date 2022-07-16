/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import Qt.labs.platform 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0

Item {
    id: root

    signal layoutSelected(int layout)

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: KeyboardLayoutSwitcher {
        hoverEnabled: true

        Plasmoid.toolTipSubText: layoutNames.longName
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

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

        Connections {
            target: root

            function onLayoutSelected(layout) {
               keyboardLayout.layout = layout
            }
        }

        PlasmaCore.IconItem {
            id: icon

            anchors.fill: parent
            visible: plasmoid.configuration.showFlag && source

            active: containsMouse
            source: iconURL(layoutNames.shortName)
        }

        PlasmaComponents3.Label {
            anchors.fill: parent
            visible: !icon.visible

            font.pointSize: height
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            text: layoutNames.displayName || layoutNames.shortName
        }
    }

    function iconURL(name) {
        return StandardPaths.locate(StandardPaths.GenericDataLocation,
                        "kf5/locale/countries/" + name + "/flag.png")
    }

    function actionTriggered(selectedLayout) {
        layoutSelected(selectedLayout)
    }
}
