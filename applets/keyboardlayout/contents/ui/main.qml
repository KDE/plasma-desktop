/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt.labs.platform 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0

Item {
    id: root

    signal layoutSelected(int layoutIndex)

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: KeyboardLayoutSwitcher {
        id: switcher

        hoverEnabled: true

        Plasmoid.toolTipSubText: layoutNames.longName
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

        Connections {
            target: switcher.keyboardLayout

            function onLayoutsListChanged() {
                root.Plasmoid.clearActions();

                switcher.keyboardLayout.layoutsList.forEach((layout, layoutIndex) => {
                    const actionName = layoutIndex.toString();
                    const actionText = layout.longName;
                    const icon = iconURL(layout.shortName).toString().substring("file://".length);
                    root.Plasmoid.setAction(actionName, actionText, icon);
                });
            }

            function onLayoutChanged() {
                root.Plasmoid.activated();
            }
        }

        Connections {
            target: root

            function onLayoutSelected(layoutIndex) {
               switcher.keyboardLayout.layout = layoutIndex;
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
        const path = `kf5/locale/countries/${name}/flag.png`;
        return StandardPaths.locate(StandardPaths.GenericDataLocation, path);
    }

    function actionTriggered(actionName) {
        const layoutIndex = parseInt(actionName);
        if (!isNaN(layoutIndex)) {
            layoutSelected(layoutIndex);
        }
    }
}
