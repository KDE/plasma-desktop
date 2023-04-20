import QtQuick 2.15
import QtQuick.Layouts 1.0
import QtQuick.Window 2.15

import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Controls 2.3 as QtControls
import org.kde.plasma.shell.panel 0.1 as Panel

Item {
    id: panelRepresentation

    required property string text
    required property string tooltip
    required property int alignment

    property bool checked: false
    property bool windowVisible: false
    property bool panelVisible: true
    property bool translucentPanel: false
    property bool sunkenPanel: false
    property bool adaptivePanel: false
    property int floatingGap: -1
    property int windowZ: 0
    property int screenHeight: Math.round(screenRect.height / 2)

    signal clicked()

    implicitHeight: childrenRect.height
    implicitWidth: childrenRect.width

    PC3.ToolTip {
        text: parent.tooltip
        visible: mouseArea.containsMouse
    }

    PlasmaCore.FrameSvgItem {
        anchors.fill: parent
        visible: mouseArea.containsMouse || parent.checked
        opacity: mouseArea.containsMouse ? 1.0 : 0.5
        scale: 1.1
        imagePath: "widgets/viewitem"
        prefix: "hover"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked(null)
    }

    ColumnLayout {
        spacing: PlasmaCore.Units.smallSpacing * 2
        Rectangle {
            id: screenRect

            readonly property double margin: PlasmaCore.Units.smallSpacing * 2
            readonly property bool isVertical: panel.formFactor === PlasmaCore.Types.Vertical
            readonly property int floatingGap: panelRepresentation.floatingGap > -1 ? panelRepresentation.floatingGap : (panel.floating ? PlasmaCore.Units.smallSpacing : 0)

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Math.round(Math.min(PlasmaCore.Units.gridUnit * 6, Screen.width * 0.1))
            implicitHeight: Math.round(Math.min(PlasmaCore.Units.gridUnit * 4, Screen.width * 0.1))
            color: PlasmaCore.Theme.backgroundColor
            border.color: PlasmaCore.Theme.highlightColor
            radius: 5
            clip: sunkenPanel

            Rectangle {
                id: panelImage

                width: screenRect.isVertical ? Math.round(parent.width / 6): Math.round(parent.width * 0.8)
                height: screenRect.isVertical ? Math.round(parent.height * 0.8) : Math.round(parent.height / 4)
                color: panelRepresentation.translucentPanel ? parent.color : PlasmaCore.Theme.buttonBackgroundColor
                opacity: panelRepresentation.translucentPanel ? 0.8 : 1.0
                border.color: "transparent"
                visible: panelRepresentation.panelVisible
                clip: panelRepresentation.adaptivePanel
                radius: clip ? 2 : 5 // Use very small radius when clip is true since clip cannot clip it. Zero radius will make it look too sharp

                x: screenRect.calculateOrdinate(screenRect.width, width, true) + (screenRect.isVertical && sunkenPanel ? -Math.round(width / 2) : 0)
                y: screenRect.calculateOrdinate(screenRect.height, height, false) + (!screenRect.isVertical && sunkenPanel ? Math.round(height / 2) : 0)
                z: 1

                Loader {
                    id: horizontalAdaptivePanelLoader
                    active: panelRepresentation.adaptivePanel && !screenRect.isVertical
                    sourceComponent: Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: Math.round(panelImage.width / 3)
                        color: screenRect.color
                        border.color: PlasmaCore.Theme.highlightColor
                        width: panelImage.width
                        height: Math.round(panelImage.height * 4)
                        radius: Math.round(height / 2)
                        rotation: 45
                    }
                }

                Loader {
                    id: verticalAdaptivePanelLoader
                    active: panelRepresentation.adaptivePanel && screenRect.isVertical
                    sourceComponent: Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: Math.round(panelImage.height / 4)
                        color: screenRect.color
                        border.color: PlasmaCore.Theme.highlightColor
                        width: Math.round(panelImage.width * 2)
                        height: panelImage.height
                        radius: Math.round(height / 2)
                        rotation: 45
                    }
                }

                Rectangle {
                    id: panelBorder
                    anchors.fill: parent
                    color: "transparent"
                    border.color: PlasmaCore.Theme.highlightColor
                    radius: parent.radius
                }
            }

            Rectangle {
                id: window
                width: Math.round(parent.width / 2)
                height: Math.round(parent.height / 2)
                visible: panelRepresentation.windowVisible
                radius: 5
                color: PlasmaCore.Theme.highlightColor
                border.color: "transparent"

                x: screenRect.isVertical ? Math.round(panelImage.x + panelImage.width / 2) : Math.round(screenRect.width / 2 - width / 2) + PlasmaCore.Units.largeSpacing
                y: screenRect.isVertical ? Math.round(screenRect.height / 2 - height / 2) : Math.round(panelImage.y - height + panelImage.height / 2)
                z: panelRepresentation.windowZ

                Row {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: PlasmaCore.Units.smallSpacing
                    spacing: PlasmaCore.Units.smallSpacing
                    Repeater {
                        model: 3
                        delegate: Rectangle {
                            width: Math.round(PlasmaCore.Units.gridUnit / 6)
                            height: width
                            radius: Math.round(height / 2)
                            color: PlasmaCore.Theme.textColor
                        }
                    }
                }
            }

            function calculateOrdinate(screenLength, panelLength, isX) {
                if(isVertical && isX) {
                    return 0 + floatingGap;
                }
                if(!isVertical && !isX) {
                    return screenLength - panelLength - floatingGap;
                }
                switch(panelRepresentation.alignment) {
                    case Qt.AlignLeft:
                        return 0;

                    case Qt.AlignCenter:
                        return Math.round((screenLength / 2) - ( panelLength / 2));

                    case Qt.AlignRight:
                        return Math.round(screenLength - panelLength);
                }
            }
        }

        PC3.Label {
            text: panelRepresentation.text
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: screenRect.implicitWidth
            wrapMode: Text.Wrap
        }
    }
}


