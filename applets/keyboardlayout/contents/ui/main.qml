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
            id: flag

            anchors.fill: parent
            visible: valid && (Plasmoid.configuration.displayStyle === 1 || Plasmoid.configuration.displayStyle === 2)

            active: containsMouse
            source: iconURL(layoutNames.shortName)

            BadgeOverlay {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: !countryCode.visible && Plasmoid.configuration.displayStyle === 2

                text: countryCode.text
                icon: flag
            }
        }


        PlasmaComponents3.Label {
            id: countryCode
            anchors.fill: parent
            visible: Plasmoid.configuration.displayStyle === 0 || !flag.valid

            font.pointSize: height
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
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
