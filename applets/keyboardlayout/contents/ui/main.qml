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

    function iconURL(name) {
        return StandardPaths.locate(StandardPaths.GenericDataLocation,
                        "kf5/locale/countries/" + name + "/flag.png")
    }

    signal layoutSelected(int layout)

    function actionTriggered(selectedLayout) {
        layoutSelected(selectedLayout)
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: KeyboardLayoutSwitcher {
        Plasmoid.toolTipSubText: layoutNames.longName
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

        Connections {
            target: keyboardLayout

            function onLayoutsListChanged() {
                Plasmoid.clearActions()

                keyboardLayout.layoutsList.forEach(
                            function(layout, index) {
                                Plasmoid.setAction(
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

        hoverEnabled: true

        PlasmaCore.IconItem {
            id: icon

            source: iconURL(layoutNames.shortName)
            visible: Plasmoid.configuration.showFlag && source
            anchors.fill: parent
            active: containsMouse
        }

        PlasmaComponents3.Label {
            text: layoutNames.displayName || layoutNames.shortName
            visible: !icon.visible
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            fontSizeMode: Text.Fit
            font.pointSize: height
        }
    }
}
