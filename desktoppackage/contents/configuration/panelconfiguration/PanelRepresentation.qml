import QtQuick 2.15
import QtQuick.Layouts 1.0
import QtQuick.Window 2.15

import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: panelRepresentation

    property string text: ""
    property var alignment: Qt.AlignHCenter | Qt.AlignBottom
    property string tooltip

    property bool isVertical: false
    property bool checked: false
    property bool windowVisible: false
    property bool panelVisible: true
    property bool translucentPanel: false
    property bool sunkenPanel: false
    property bool adaptivePanel: false
    property bool fillAvailable: false
    property int floatingGap: 0
    property int windowZ: 0
    property var mainIconSource: null
    property int screenHeight: Math.round(screenRect.height / 2)

    readonly property bool iconAndLabelsShouldlookSelected: checked || mouseArea.pressed

    signal clicked()

    implicitHeight: mainItem.height
    implicitWidth: mainItem.width

    PC3.ToolTip {
        text: parent.tooltip
        visible: mouseArea.containsMouse && text.length > 0
    }

    PlasmaExtras.Highlight {
        anchors.fill: parent
        anchors.margins: -Kirigami.Units.smallSpacing
        hovered: mouseArea.containsMouse
        pressed: panelRepresentation.iconAndLabelsShouldlookSelected
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }

    ColumnLayout {
        id: mainItem
        spacing: Kirigami.Units.smallSpacing
        Rectangle {
            id: screenRect

            readonly property double margin: Kirigami.Units.smallSpacing * 2
            readonly property int floatingGap: panelRepresentation.floatingGap > -1 ? panelRepresentation.floatingGap : (panel.floating ? Kirigami.Units.smallSpacing : 0)

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Math.round(Math.min(Kirigami.Units.gridUnit * 6, Screen.width * 0.1))
            implicitHeight: Math.round(Math.min(Kirigami.Units.gridUnit * 4, Screen.width * 0.1))
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(1, 1, 1, 0.3))
            border.color: Kirigami.Theme.highlightColor
            radius: 5
            clip: sunkenPanel

            RowLayout {
                anchors.fill: parent
                Rectangle {
                    id: panelImage

                    width: isVertical ? Math.round(parent.width / 6) : Math.round(parent.width * (fillAvailable ? 1 : 0.8))
                    height: isVertical ? Math.round(parent.height * (fillAvailable ? 1 : 0.8)) : Math.round(parent.height / 4)
                    implicitWidth: width
                    implicitHeight: height
                    Layout.alignment: alignment
                    Layout.bottomMargin: sunkenPanel * -Math.round(height / 2) + floatingGap
                    color: panelRepresentation.translucentPanel ? screenRect.color : Kirigami.Theme.backgroundColor
                    opacity: panelRepresentation.translucentPanel ? 0.8 : 1.0
                    border.color: "transparent"
                    visible: panelRepresentation.panelVisible
                    clip: panelRepresentation.adaptivePanel
                    radius: 5

                    z: 1

                    Loader {
                        id: horizontalAdaptivePanelLoader
                        active: panelRepresentation.adaptivePanel && !isVertical
                        sourceComponent: Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: Math.round(panelImage.width / 3)
                            color: Qt.lighter(screenRect.color)
                            border.color: Kirigami.Theme.highlightColor
                            width: panelImage.width
                            height: Math.round(panelImage.height * 4)
                            radius: Math.round(height / 2)
                            rotation: 45
                        }
                    }

                    Loader {
                        id: verticalAdaptivePanelLoader
                        active: panelRepresentation.adaptivePanel && isVertical
                        sourceComponent: Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: Math.round(panelImage.height / 4)
                            color: Qt.lighter(screenRect.color)
                            border.color: Kirigami.Theme.highlightColor
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
                        border.color: Kirigami.Theme.highlightColor
                        radius: parent.radius
                    }
                }
            }

            Rectangle {
                id: window
                width: Math.round(parent.width / 2)
                height: Math.round(parent.height / 2)
                visible: panelRepresentation.windowVisible
                radius: 5
                color: Kirigami.Theme.highlightColor
                border.color: "transparent"

                x: isVertical ? Math.round(panelImage.x + panelImage.width / 2) : Math.round(screenRect.width / 2 - width / 2) + Kirigami.Units.gridUnit
                y: isVertical ? Math.round(screenRect.height / 2 - height / 2) : Math.round(panelImage.y - height + panelImage.height / 2)
                z: panelRepresentation.windowZ

                Row {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: Kirigami.Units.smallSpacing
                    spacing: Kirigami.Units.smallSpacing
                    Repeater {
                        model: 3
                        delegate: Rectangle {
                            width: Math.round(Kirigami.Units.gridUnit / 6)
                            height: width
                            radius: Math.round(height / 2)
                            color: Kirigami.Theme.textColor
                        }
                    }
                }
            }

            Kirigami.Icon {
                id: mainIcon
                visible: panelRepresentation.mainIconSource
                anchors.centerIn: parent
                transform: Translate {
                    y: isVertical ? 0 : Math.round((mainIcon.y - panelImage.y) / 4)
                    x: isVertical ? Math.round((mainIcon.x - panelImage.x) / 4) : 0
                }
                height: parent.height / 2
                source: panelRepresentation.mainIconSource
            }
        }
    }
}


