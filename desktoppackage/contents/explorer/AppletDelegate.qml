/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.kwindowsystem
import org.kde.kirigami as Kirigami
import org.kde.graphicaleffects as KGraphicalEffects

Item {
    id: delegate

    readonly property string pluginName: model.pluginName
    readonly property bool pendingUninstall: pendingUninstallTimer.applets.indexOf(pluginName) > -1
    readonly property bool pressed: tapHandler.pressed
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    width: list.cellWidth
    height: list.cellHeight

    Accessible.name: i18nc("@action:button accessible only, %1 is widget name", "Add %1", model.name)  + (model.isSupported ? "" : unsupportedTooltip.mainText)
    Accessible.description: (model.isSupported ? "" : model.unsupportedMessage) + model.description + (overlayedBadge.visible ? countLabel.Accessible.name : "")
    Accessible.role: Accessible.Button

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
        id: unsupportedTooltip
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
            delegate.forceActiveFocus()
        }
        Drag.onDragFinished: {
            main.draggingWidget = false;
        }

        DragHandler {
            id: dragHandler
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
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


        DragHandler {
            id: touchDragHandler
            acceptedDevices: PointerDevice.Stylus | PointerDevice.TouchScreen
            enabled: dragHandler.enabled
            yAxis.enabled: false

            onActiveChanged: if (active) {
                iconContainer.grabToImage(function(result) {
                    if (!touchDragHandler.active) {
                        return;
                    }
                    parent.Drag.imageSource = result.url;
                    parent.Drag.active = touchDragHandler.active;
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
                    width: overlayedBadge.width + Math.round(Kirigami.Units.smallSpacing * 1.5)
                    height: overlayedBadge.height + Math.round(Kirigami.Units.smallSpacing * 1.5)
                    radius: height
                    visible: (running && delegate.GridView.isCurrentItem) ?? false
                }
            }

            KGraphicalEffects.BadgeEffect {
                anchors.fill: parent
                source: ShaderEffectSource {
                    sourceItem: iconWidget
                    hideSource: !softwareRendering
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
                id: overlayedBadge
                width: countLabel.width + height
                height: Math.round(Kirigami.Units.iconSizes.sizeForLabels * 1.3)
                radius: height
                color: (running && delegate.GridView.isCurrentItem) ? Kirigami.Theme.highlightColor : Kirigami.Theme.positiveTextColor
                visible: ((running && delegate.GridView.isCurrentItem) || model.recent) ?? false
                onVisibleChanged: maskShaderSource.scheduleUpdate()

                PlasmaComponents.Label {
                    id: countLabel
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                    text: (running && delegate.GridView.isCurrentItem) ? running : i18ndc("plasma_shell_org.kde.plasma.desktop", "Text displayed on top of newly installed widgets", "New!")
                    Accessible.name: running
                        ? i18ncp("@info:other accessible for badge showing applet count", "%1 widget active", "%1 widgets active", running)
                        : i18nc(" @info:other accessible for badge indicating new widget", "Recently installed")
                    textFormat: Text.PlainText
                }
            }


            PlasmaComponents.ToolButton {
                id: uninstallButton
                anchors {
                    top: parent.top
                    right: parent.right
                }
                text: delegate.pendingUninstall ? i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel Uninstallation")
                    : i18ndc("plasma_shell_org.kde.plasma.desktop","@info:tooltip uninstall a widget", "Mark for Uninstallation")
                icon.name: delegate.pendingUninstall ? "edit-undo" : "edit-delete"
                display: PlasmaComponents.AbstractButton.IconOnly
                Accessible.description: delegate.pendingUninstall
                    ? i18nc("@action:button accessible only, %1 is widget name", "Cancel pending uninstallation for widget %1", model.name)
                    : i18nc("@action:button accessible only, %1 is widget name", "Mark widget %1 for uninstallation. Requires confirmation", model.name)
                // we don't really "undo" anything but we'll pretend to the user that we do
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents.ToolTip.visible: hovered
                PlasmaComponents.ToolTip.text: text
                flat: false
                visible: (model.local && delegate.GridView.isCurrentItem && !dragHandler.active && !touchDragHandler.active) ?? false

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

            PlasmaComponents.ToolButton {
                id: removeInstancesButton
                anchors {
                    top: parent.top
                    right: uninstallButton.visible ? uninstallButton.left : parent.right
                    rightMargin: uninstallButton.visible ? Kirigami.Units.smallSpacing : 0
                }
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove all instances")
                display: PlasmaComponents.AbstractButton.IconOnly
                icon.name: "edit-clear-all"
                Accessible.description: i18ncp("@action:button accessible description, %1 number of instances %2 is widget name",
                                               "Remove running instance of widget %2",
                                               "Remove all %1 running instances of widget %2", running ,model.name)

                // we don't really "undo" anything but we'll pretend to the user that we do
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents.ToolTip.visible: hovered
                PlasmaComponents.ToolTip.text: text
                flat: false
                visible: (running && delegate.GridView.isCurrentItem && !dragHandler.active) ?? false

                onHoveredChanged: {
                    if (hovered) {
                        // hovering the uninstall button triggers onExited of the main mousearea
                        delegate.GridView.view.currentIndex = index
                    }
                }

                onClicked: widgetExplorer.removeAllInstances(pluginName)
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
            maximumLineCount: 3
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
            maximumLineCount: 5 - heading.lineCount
            horizontalAlignment: Text.AlignHCenter
            color: mainLayout.textColor
        }
    }
}
