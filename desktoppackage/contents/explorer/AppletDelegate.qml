/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.1

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kwindowsystem
import org.kde.kirigami 2.20 as Kirigami
import org.kde.graphicaleffects as KGraphicalEffects

Item {
    id: delegate

    readonly property string pluginName: model.pluginName
    readonly property bool pendingUninstall: pendingUninstallTimer.applets.indexOf(pluginName) > -1

    width: list.cellWidth
    height: list.cellHeight

    HoverHandler {
        onHoveredChanged: if (hovered) delegate.GridView.view.currentIndex = index
    }

    TapHandler {
        enabled: !delegate.pendingUninstall
        onTapped: widgetExplorer.addApplet(delegate.pluginName)
    }

    // Avoid repositioning delegate item after dragFinished
    Item {
        anchors.fill: parent

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
            enabled: !delegate.pendingUninstall

            onActiveChanged: if (active) {
                iconContainer.grabToImage(function(result) {
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

                QQC2.Label {
                    id: countLabel
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: running
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
                visible: model.local && delegate.GridView.isCurrentItem

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
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            lineHeight: 0.95
            horizontalAlignment: Text.AlignHCenter
        }
        QQC2.Label {
            Layout.fillWidth: true
            // otherwise causes binding loop due to the way the Plasma sets the height
            height: implicitHeight
            text: model.description
            font: Kirigami.Theme.smallFont
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            maximumLineCount: heading.lineCount === 1 ? 3 : 2
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
