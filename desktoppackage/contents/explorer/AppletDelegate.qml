/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.kwindowsystem
import org.kde.kirigami 2.20 as Kirigami
import org.kde.graphicaleffects as KGraphicalEffects

Item {
    id: delegate

    readonly property string pluginName: model.pluginName
    readonly property bool pendingUninstall: pendingUninstallTimer.applets.indexOf(pluginName) > -1
    readonly property bool pressed: tapHandler.pressed

    width: list.cellWidth
    height: list.cellHeight

    HoverHandler {
        id: hoverHandler
        onHoveredChanged: if (hovered) delegate.GridView.view.currentIndex = index
    }

    TapHandler {
        id: tapHandler
        enabled: !delegate.pendingUninstall && model.isSupported
        onTapped: widgetExplorer.addApplet(delegate.pluginName)
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        visible: !model.isSupported
        mainText: i18n("Unsupported Widget")
        subText: model.unsupportedMessage
    }

    // Avoid repositioning delegate item after dragFinished
    Item {
        anchors.fill: parent
        enabled: model.isSupported

        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.MoveAction | Qt.LinkAction
        Drag.mimeData: {
            "text/x-plasmoidservicename" : delegate.pluginName,
        }
        Drag.onDragStarted: {
            KWindowSystem.showingDesktop = true;
            main.draggingWidget = true;
        }
        Drag.onDragFinished: {
            main.draggingWidget = false;
        }

        DragHandler {
            id: dragHandler
            enabled: !delegate.pendingUninstall && model.isSupported

            onActiveChanged: if (active) {
                iconContainer.grabToImage(function(result) {
                    if (!dragHandler.active) {
                        return;
                    }
                    parent.Drag.imageSource = result.url;
                    parent.Drag.active = dragHandler.active;
                }, Qt.size(Kirigami.Units.iconSizes.huge, Kirigami.Units.iconSizes.huge));
            } else {
                parent.Drag.active = false;
                parent.Drag.imageSource = "";
            }
        }
    }

    ColumnLayout {
        id: mainLayout

        readonly property color textColor: tapHandler.pressed ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

        spacing: Kirigami.Units.smallSpacing
        anchors {
            left: parent.left
            right: parent.right
            //bottom: parent.bottom
            margins: Kirigami.Units.smallSpacing * 2
            rightMargin: Kirigami.Units.smallSpacing * 2 // don't cram the text to the border too much
            top: parent.top
        }

        Item {
            id: iconContainer
            width: Kirigami.Units.iconSizes.enormous
            height: width
            Layout.alignment: Qt.AlignHCenter
            opacity: delegate.pendingUninstall ? 0.6 : 1
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Item {
                id: iconWidget
                anchors.fill: parent
                Kirigami.Icon {
                    anchors.fill: parent
                    source: model.decoration
                    visible: model.screenshot === ""
                    selected: tapHandler.pressed
                    enabled: model.isSupported
                }
                Image {
                    width: Kirigami.Units.iconSizes.enormous
                    height: width
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: model.screenshot
                }
            }

            Item {
                id: badgeMask
                anchors.fill: parent

                Rectangle {
                    x: Math.round(-Kirigami.Units.smallSpacing * 1.5 / 2)
                    y: x
                    width: runningBadge.width + Math.round(Kirigami.Units.smallSpacing * 1.5)
                    height: width
                    radius: height
                    visible: running && delegate.GridView.isCurrentItem
                }
            }

            KGraphicalEffects.BadgeEffect {
                anchors.fill: parent
                source: ShaderEffectSource {
                    sourceItem: iconWidget
                    hideSource: true
                    live: false
                }
                mask: ShaderEffectSource {
                    id: maskShaderSource
                    sourceItem: badgeMask
                    hideSource: true
                    live: false
                }
            }

            Rectangle {
                id: runningBadge
                width: height
                height: Math.round(Kirigami.Units.iconSizes.sizeForLabels * 1.3)
                radius: height
                color: Kirigami.Theme.highlightColor
                visible: running && delegate.GridView.isCurrentItem
                onVisibleChanged: maskShaderSource.scheduleUpdate()

                PlasmaComponents.Label {
                    id: countLabel
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: running
                    textFormat: Text.PlainText
                }
            }


            PlasmaComponents.ToolButton {
                id: uninstallButton
                anchors {
                    top: parent.top
                    right: parent.right
                }
                icon.name: delegate.pendingUninstall ? "edit-undo" : "edit-delete"
                // we don't really "undo" anything but we'll pretend to the user that we do
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents.ToolTip.visible: hovered
                PlasmaComponents.ToolTip.text: delegate.pendingUninstall ? i18nd("plasma_shell_org.kde.plasma.desktop", "Undo uninstall")
                                                    : i18nd("plasma_shell_org.kde.plasma.desktop", "Uninstall widget")
                flat: false
                visible: model.local && delegate.GridView.isCurrentItem && !dragHandler.active

                onHoveredChanged: {
                    if (hovered) {
                        // hovering the uninstall button triggers onExited of the main mousearea
                        delegate.GridView.view.currentIndex = index
                    }
                }

                onClicked: {
                    var pending = pendingUninstallTimer.applets
                    if (delegate.pendingUninstall) {
                        var index = pending.indexOf(pluginName)
                        if (index > -1) {
                            pending.splice(index, 1)
                        }
                    } else {
                        pending.push(pluginName)
                    }
                    pendingUninstallTimer.applets = pending

                    if (pending.length) {
                        pendingUninstallTimer.restart()
                    } else {
                        pendingUninstallTimer.stop()
                    }
                }
            }
        }
        Kirigami.Heading {
            id: heading
            Layout.fillWidth: true
            level: 4
            text: model.name
            textFormat: Text.PlainText
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            lineHeight: 0.95
            horizontalAlignment: Text.AlignHCenter
            color: mainLayout.textColor
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            // otherwise causes binding loop due to the way the Plasma sets the height
            height: implicitHeight
            text: model.description
            textFormat: Text.PlainText
            font: Kirigami.Theme.smallFont
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            maximumLineCount: heading.lineCount === 1 ? 3 : 2
            horizontalAlignment: Text.AlignHCenter
            color: mainLayout.textColor
        }
    }
}
