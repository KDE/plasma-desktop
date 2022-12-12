/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.8
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.15 as Kirigami

Item {
    id: main

    property bool isVertical: plasmoid.formFactor === 3

    width: Kirigami.Settings.hasTransientTouchInput
            ? (isVertical ? plasmoid.width : height)
            : (isVertical
                ? PlasmaCore.Units.iconSizes.medium
                : PlasmaCore.Units.iconSizes.smallMedium + PlasmaCore.Units.smallSpacing * 2)
    height: Kirigami.Settings.hasTransientTouchInput
            ? (isVertical ? width : plasmoid.height)
            : (isVertical
                ? PlasmaCore.Units.iconSizes.smallMedium + PlasmaCore.Units.smallSpacing * 2
                : PlasmaCore.Units.iconSizes.medium)

    z: 999

    states: [
        State {
            when: plasmoid.editMode
            PropertyChanges {
                target: main
                visible: true
            }
            PropertyChanges {
                target: main
                opacity: mouseArea.containsMouse || plasmoid.userConfiguring ? 1 : 0.5
            }
        },
        State {
            when: !plasmoid.editMode
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
            duration: PlasmaCore.Units.longDuration;
            easing.type: Easing.InOutQuad;
        }
    }

    LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)
    anchors {
        left: undefined
        top: undefined
        right: isVertical || !parent ? undefined : parent.right
        bottom: isVertical && parent ? parent.bottom : undefined
        verticalCenter: isVertical || !parent ? undefined : parent.verticalCenter
        horizontalCenter: isVertical && parent ? parent.horizontalCenter : undefined
    }

    PlasmaCore.SvgItem {
        id: toolBoxIcon
        svg: PlasmaCore.Svg {
            id: iconSvg
            imagePath: "widgets/configuration-icons"
        }
        elementId: "configure"

        anchors.centerIn: mouseArea
        width: Kirigami.Settings.hasTransientTouchInput ? Math.min(PlasmaCore.Units.iconSizes.medium, parent.width - Kirigami.Units.smallSpacing * 2) : PlasmaCore.Units.iconSizes.small
        height: width
    }

    Connections {
        target: plasmoid
        function onUserConfiguringChanged() {
            if (plasmoid.userConfiguring) {
                plasmoid.editMode = true;
                toolTipArea.hideToolTip();
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: enabled
        enabled: plasmoid.editMode || plasmoid.userConfiguring
        onClicked: {
            main.Plasmoid.action("configure").trigger()
        }
        activeFocusOnTab: true
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Space:
            case Qt.Key_Enter:
            case Qt.Key_Return:
            case Qt.Key_Select:
                main.Plasmoid.action("configure").trigger();
                break;
            }
        }
        objectName: "configurePanelButton" // used for stable accessible id
        Accessible.name: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Configure Panel…")
        Accessible.description: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Open Panel configuration ui")
        Accessible.role: Accessible.Button
        Accessible.onPressAction: main.Plasmoid.action("configure").trigger();

        PlasmaCore.ToolTipArea {
            id: toolTipArea
            anchors.fill: parent
            mainText: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Configure Panel…")
            icon: "configure"
            enabled: mouseArea.containsMouse
        }
    }
}
