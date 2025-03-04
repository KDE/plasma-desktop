import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PC3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.ksvg as KSvg
import org.kde.plasma.shell.panel as Panel
import org.kde.kirigami as Kirigami

Item {
    id: root

    property string text
    property /*Qt::Alignment*/int alignment: Qt.AlignHCenter | Qt.AlignBottom
    property string tooltip

    property bool isVertical: false
    property bool isHorizontal: !isVertical
    property bool checked: false
    property bool windowVisible: false
    property bool panelVisible: true

    property bool isTop: !!(alignment & Qt.AlignTop)
    property bool isBottom: !!(alignment & Qt.AlignBottom)
    property bool isLeft: !!(alignment & Qt.AlignLeft)
    property bool isRight: !!(alignment & Qt.AlignRight)

    property bool translucentPanel: false
    property bool sunkenPanel: false
    property bool adaptivePanel: false
    property bool panelReservesSpace: true

    property bool fillAvailable: false
    property int floatingGap: 0
    property var mainIconSource: null
    property int screenHeight: Math.round(screenRect.height / 2)

    property bool visibleApplet: false
    property bool floatingApplet: false

    readonly property bool iconAndLabelsShouldlookSelected: checked || mouseArea.pressed

    function maximizeWindow() {

        hidePanelLater.stop()
        hidePanel.stop()
        showPanel.restart()

        moveWindowOverPanel.stop()
        resetWindowOverPanel.restart()

        maximizeAnimation.restart()
    }
    function hidePanel() {

        hidePanelLater.stop()
        maximizeAnimation.stop()
        resetMaximize.restart()

        moveWindowOverPanel.stop()
        resetWindowOverPanel.restart()

        hidePanel.restart()
    }

    function dodgePanel() {

        hidePanel.stop()
        showPanel.restart()

        maximizeAnimation.stop()
        resetMaximize.restart()

        moveWindowOverPanel.restart()
        hidePanelLater.restart()
    }

    Timer {
        id: hidePanelLater
        interval: 200
        onTriggered: hidePanel.restart()
        running: false
    }

    signal clicked()

    implicitHeight: mainItem.height
    implicitWidth: mainItem.width

    PC3.ToolTip {
        text: root.tooltip
        visible: mouseArea.containsMouse && text.length > 0
    }

    PlasmaExtras.Highlight {
        anchors.fill: parent
        anchors.margins: -Kirigami.Units.smallSpacing
        hovered: mouseArea.containsMouse
        pressed: root.iconAndLabelsShouldlookSelected
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }

    ColumnLayout {
        id: mainItem
        spacing: Kirigami.Units.smallSpacing
        Rectangle {
            id: screenRect

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Math.round(Math.min(Kirigami.Units.gridUnit * 6, Screen.width * 0.1))
            implicitHeight: Math.round(Math.min(Kirigami.Units.gridUnit * 4, Screen.width * 0.1))
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(1, 1, 1, 0.3))
            border.color: Kirigami.Theme.highlightColor
            radius: Kirigami.Units.cornerRadius
            clip: root.sunkenPanel

            RowLayout {
                anchors.fill: parent
                z: 1

                Rectangle {
                    id: panelImage

                    property real sunkenValue: 0

                    Component.onCompleted: {
                        panelImage.sunkenValue = root.sunkenPanel / 2
                    }

                    implicitWidth: root.isVertical ? Math.round(parent.width / 6) : Math.round(parent.width * (root.fillAvailable ? 1 : 0.7))
                    implicitHeight: root.isVertical ? Math.round(parent.height * (root.fillAvailable ? 1 : 0.8)) : Math.round(parent.height / 4)

                    Layout.alignment: root.alignment
                    Layout.bottomMargin: root.isBottom ? sunkenValue * -Math.round(height) + root.floatingGap : 0
                    Layout.topMargin: root.isTop ? sunkenValue * -Math.round(height) + root.floatingGap : 0
                    Layout.leftMargin: root.isLeft ? sunkenValue * -Math.round(width) + root.floatingGap : 0
                    Layout.rightMargin: root.isRight ? sunkenValue * -Math.round(width) + root.floatingGap : 0

                    color: root.translucentPanel ? screenRect.color : Kirigami.Theme.backgroundColor
                    opacity: root.translucentPanel ? 0.8 : 1.0
                    border.color: "transparent"
                    visible: root.panelVisible
                    clip: root.adaptivePanel
                    radius: Kirigami.Units.cornerRadius

                    SequentialAnimation on sunkenValue {
                        id: hidePanel
                        running: false
                        NumberAnimation {
                            to: 0
                            duration: Kirigami.Units.shortDuration
                        }
                        NumberAnimation {
                            to: 1
                            duration: Kirigami.Units.veryLongDuration
                        }
                        PauseAnimation {
                            duration: Kirigami.Units.veryLongDuration * 2
                        }
                        NumberAnimation {
                            to: 0.5
                            duration: Kirigami.Units.veryLongDuration
                        }
                    }
                    SequentialAnimation on sunkenValue {
                        id: showPanel
                        running: false
                        NumberAnimation {
                            to: 0
                            duration: Kirigami.Units.shortDuration
                        }
                    }

                    Loader {
                        id: horizontalAdaptivePanelLoader
                        active: root.adaptivePanel && root.isHorizontal
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
                        active: root.adaptivePanel && root.isVertical
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
                        radius: panelImage.radius
                    }
                }
            }

            Rectangle {
                x: panelImage.x + root.isLeft * panelImage.width - root.isRight * width + root.isVertical * panelSpacing
                y: panelImage.y + root.isTop * panelImage.height - root.isBottom * height + (root.isHorizontal) * panelSpacing
                height: Math.round(parent.height / 2)
                width: Math.round(parent.width / 3)
                visible: root.visibleApplet

                property int panelSpacing: Kirigami.Units.smallSpacing * (root.floatingApplet ? -1 : 1) * (root.isRight || root.isBottom ? 1 : -1)

                color: window.color
                radius: 5

                Behavior on y {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }
                Behavior on x {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }
            }

            Rectangle {
                id: window

                property real maximized: 0
                property real windowOverPanel: 0

                width: Math.round(parent.width * (0.4 + 0.6 * maximized) - panelImage.width * root.panelReservesSpace * maximized * root.isVertical)
                height: Math.round(parent.height * (0.4 + 0.6 * maximized) - panelImage.height * root.panelReservesSpace * maximized * root.isHorizontal)
                visible: root.windowVisible
                radius: 5
                color: Kirigami.Theme.highlightColor
                border.color: "transparent"

                x: Math.round(screenRect.width / 2 - width / 2) * (1 - maximized) + windowOverPanel * Kirigami.Units.mediumSpacing * 2 * root.isVertical * (root.isLeft ? - 1 : 1) + panelImage.width * maximized * root.isLeft * root.panelReservesSpace
                y: Math.round(screenRect.height / 2 - height / 2) * (1 - maximized) + windowOverPanel * Kirigami.Units.mediumSpacing * 2 * root.isHorizontal * (root.isTop ? - 1 : 1) + panelImage.height * maximized * root.isTop * root.panelReservesSpace
                z: 0

                SequentialAnimation on maximized {
                    id: maximizeAnimation
                    running: false
                    NumberAnimation {
                        to: 0
                        duration: Kirigami.Units.shortDuration
                    }
                    NumberAnimation {
                        to: 1
                        duration: Kirigami.Units.longDuration
                    }
                    PauseAnimation {
                        duration: Kirigami.Units.veryLongDuration * 2
                    }
                    NumberAnimation {
                        to: 0
                        duration: Kirigami.Units.longDuration
                    }
                }
                NumberAnimation on maximized {
                    id: resetMaximize
                    running: false
                    to: 0
                    duration: Kirigami.Units.shortDuration
                }
                SequentialAnimation on windowOverPanel {
                    id: moveWindowOverPanel
                    NumberAnimation {
                        to: 0
                        duration: Kirigami.Units.shortDuration
                    }
                    NumberAnimation {
                        to: 1
                        duration: Kirigami.Units.veryLongDuration
                    }
                    PauseAnimation {
                        duration: Kirigami.Units.veryLongDuration * 2
                    }
                    NumberAnimation {
                        to: 0
                        duration: Kirigami.Units.veryLongDuration
                    }
                }
                NumberAnimation on windowOverPanel {
                    id: resetWindowOverPanel
                    running: false
                    to: 0
                    duration: Kirigami.Units.shortDuration
                }

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
                visible: valid
                anchors.centerIn: parent
                transform: Translate {
                    y: root.isVertical ? 0 : Math.round((mainIcon.y - panelImage.y) / 4)
                    x: root.isVertical ? Math.round((mainIcon.x - panelImage.x) / 4) : 0
                }
                height: parent.height / 2
                source: root.mainIconSource
            }
        }
    }
}
