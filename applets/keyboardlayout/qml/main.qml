/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.workspace.components
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    signal layoutSelected(int layoutIndex)

    preferredRepresentation: fullRepresentation
    toolTipMainText: Plasmoid.title
    toolTipSubText: "" // proper subtext is set by fullRepresentation

    readonly property bool inEmbeddedContainment: Plasmoid.containment.containmentType === PlasmaCore.Containment.CustomEmbedded

    fullRepresentation: KeyboardLayoutSwitcher {
        id: switcher

        hoverEnabled: true
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : root.inEmbeddedContainment ? PlasmaCore.Types.HiddenStatus : PlasmaCore.Types.PassiveStatus

        Binding {
            root.toolTipSubText: switcher.layoutNames.longName
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: root.toolTipMainText
            subText: root.toolTipSubText
        }

        Instantiator {
            id: actionsInstantiator
            model: switcher.keyboardLayout.layoutsList
            delegate: PlasmaCore.Action {
                required property string longName
                required property string shortName
                required property int index

                text: longName
                icon.icon: KCMKeyboard.Flags.getIcon(shortName)
                onTriggered: {
                    root.layoutSelected(index);
                }
            }
            onObjectAdded: (index, object) => {
                Plasmoid.contextualActions.push(object)
            }
            onObjectRemoved: (index, object) => {
                Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(object), 1)
            }
        }

        Connections {
            target: root

            function onLayoutSelected(layoutIndex) {
               switcher.keyboardLayout.layout = layoutIndex;
            }
        }

        Connections {
            target: Plasmoid

            function onActivated() {
                switcher.keyboardLayout.switchToNextLayout()
            }
        }

        Kirigami.Icon {
            id: flag

            anchors.fill: parent

            visible: valid && (Plasmoid.configuration.displayStyle === 1 || Plasmoid.configuration.displayStyle === 2)

            active: switcher.containsMouse
            source: KCMKeyboard.Flags.getIcon(switcher.layoutNames.shortName)

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

            anchors.centerIn: parent
            width: Math.min(switcher.width, switcher.height)
            height: width

            visible: Plasmoid.configuration.displayStyle === 0 || !flag.valid

            font.pointSize: height || Kirigami.Theme.defaultFont.pointSize
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: switcher.layoutNames.displayName || switcher.layoutNames.shortName
            textFormat: Text.PlainText
        }
    }

    function actionTriggered(actionName) {
        const layoutIndex = parseInt(actionName);
        if (!isNaN(layoutIndex)) {
            layoutSelected(layoutIndex);
        }
    }
}
