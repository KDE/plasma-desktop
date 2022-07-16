/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt.labs.platform 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0

Item {
    id: root

    signal layoutSelected(int layout)

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.fullRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: KeyboardLayoutSwitcher {
        id: keyboardLayoutSwitcher

        activeFocusOnTab: true
        hoverEnabled: true

        Plasmoid.onActivated: keyboardLayout.switchToNextLayout();
        Plasmoid.toolTipSubText: layoutNames.longName
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Space:
            case Qt.Key_Enter:
            case Qt.Key_Return:
            case Qt.Key_Select:
                Plasmoid.activated();
                break;
            }
        }
        Accessible.name: Plasmoid.title
        Accessible.description: i18nc("@info:tooltip", "Current keyboard layout is %1", layoutNames.longName)
        Accessible.role: Accessible.Button

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
        }

        Connections {
            target: root

            function onLayoutSelected(layout) {
               keyboardLayout.layout = layout
            }
        }

        Component {
            id: iconComponent

            PlasmaCore.IconItem {
                active: keyboardLayoutSwitcher.containsMouse
                source: iconURL(layoutNames.shortName)
            }
        }

        Component {
            id: labelComponent

            PlasmaComponents3.Label {
                font.pointSize: height
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                text: layoutNames.displayName || layoutNames.shortName
            }
        }

        Loader {
            anchors.fill: parent
            sourceComponent: Plasmoid.configuration.showFlag && source ? iconComponent : labelComponent
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
