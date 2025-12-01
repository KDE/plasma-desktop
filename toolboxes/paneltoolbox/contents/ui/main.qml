/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami

Item {
    id: main

    property bool isVertical: Plasmoid.formFactor === 3

    width: Kirigami.Settings.hasTransientTouchInput
            ? (isVertical ? Plasmoid.width : height)
            : (isVertical
                ? Kirigami.Units.iconSizes.medium
                : Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2)
    height: Kirigami.Settings.hasTransientTouchInput
            ? (isVertical ? width : Plasmoid.height)
            : (isVertical
                ? Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
                : Kirigami.Units.iconSizes.medium)

    z: 999

    states: [
        State {
            when: Plasmoid.containment.corona.editMode
            PropertyChanges {
                target: main
                visible: true
            }
            PropertyChanges {
                target: main
                opacity: mouseArea.containsMouse || Plasmoid.userConfiguring ? 1 : 0.5
            }
        },
        State {
            when: !Plasmoid.containment.corona.editMode
            PropertyChanges {
                target: main
                visible: false
            }
            PropertyChanges {
                target: main
                opacity: 0
            }
        }
    ]
    Behavior on opacity {
        OpacityAnimator {
            duration: Kirigami.Units.longDuration;
            easing.type: Easing.InOutQuad;
        }
    }

    anchors {
        left: undefined
        top: undefined
        right: isVertical || !parent ? undefined : parent.right
        bottom: isVertical && parent ? parent.bottom : undefined
        verticalCenter: isVertical || !parent ? undefined : parent.verticalCenter
        horizontalCenter: isVertical && parent ? parent.horizontalCenter : undefined
    }

    Kirigami.Icon {
        source: "configure"

        anchors.centerIn: mouseArea
        width: Kirigami.Settings.hasTransientTouchInput
            ? Kirigami.Units.iconSizes.medium
            : Kirigami.Units.iconSizes.small
        height: width
    }

    Connections {
        target: Plasmoid
        function onUserConfiguringChanged() {
            if (Plasmoid.userConfiguring) {
                Plasmoid.containment.corona.editMode = true;
                toolTipArea.hideToolTip();
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: enabled
        enabled: Plasmoid.containment.corona.editMode || Plasmoid.userConfiguring
        onClicked: {
            main.Plasmoid.internalAction("configure").trigger()
        }
        activeFocusOnTab: true
        Keys.onPressed: event => {
            switch (event.key) {
            case Qt.Key_Space:
            case Qt.Key_Enter:
            case Qt.Key_Return:
            case Qt.Key_Select:
                main.Plasmoid.internalAction("configure").trigger();
                break;
            }
        }
        objectName: "configurePanelButton" // used for stable accessible id
        Accessible.name: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Configure Panel…")
        Accessible.description: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Open Panel configuration ui")
        Accessible.role: Accessible.Button
        Accessible.onPressAction: main.Plasmoid.internalAction("configure").trigger();

        PlasmaCore.ToolTipArea {
            id: toolTipArea
            anchors.fill: parent
            mainText: main.Plasmoid.internalAction("configure").text
            icon: "configure"
            enabled: mouseArea.containsMouse
        }
    }
}
