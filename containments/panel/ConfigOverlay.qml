/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.0
import QtQuick.Window
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami as Kirigami

MouseArea {
    id: configurationArea

    z: 1000
    hoverEnabled: true

    property Item currentApplet
    property int draggedItemIndex
    property real startDragOffset: 0.0

    Drag.dragType: Drag.Automatic
    Drag.active: false
    Drag.supportedActions: Qt.MoveAction
    Drag.mimeData: {
        "text/x-plasmoidinstanceid": Plasmoid.containment.id+':'+configurationArea.currentApplet?.applet.plasmoid.id
    }
    Drag.onDragFinished: dropEvent => {
        if (dropEvent != Qt.MoveAction) {
            //currentApplet.applet.plasmoid.internalAction("remove").trigger()
            appletsModel.insert(configurationArea.draggedItemIndex - 1, {applet: currentApplet.applet});
        }
        currentApplet.destroy()
        root.dragAndDropping = false
        root.layoutManager.save()
    }

    onPositionChanged: mouse => {
        if (pressed) {

            // If the object has been dragged outside of the panel and there's
            // a different containment there, we remove it from the panel
            // containment and add it to the new one.
            var padding = Kirigami.Units.gridUnit * 5;
            if (currentApplet && (mouse.x < -padding || mouse.y < -padding ||
                mouse.x > width + padding || mouse.y > height + padding)) {
                configurationArea.currentApplet.grabToImage(result => {
                    configurationArea.Drag.imageSource = result.url
                    appletsModel.remove(placeHolder.parent.index)
                    currentApplet.visible = false
                    configurationArea.Drag.active = true
                })
            }
            if (Plasmoid.formFactor === PlasmaCore.Types.Vertical && currentApplet) {
                currentApplet.y = mouse.y - startDragOffset;
            } else {
                currentApplet.x = mouse.x - startDragOffset;
            }

            const item = root.layoutManager.childAtCoordinates(mouse.x, mouse.y);

            if (item && item.applet !== placeHolder) {
                var posInItem = mapToItem(item, mouse.x, mouse.y)
                var pos = root.isHorizontal ? posInItem.x : posInItem.y
                var size = root.isHorizontal ? item.width : item.height
                if (pos < size / 3) {
                    root.layoutManager.move(placeHolder.parent, item.index)
                } else if (pos > size / 3 * 2) {
                    root.layoutManager.move(placeHolder.parent, item.index+1)
                }
            }

        } else {
            const item = currentLayout.childAt(mouse.x, mouse.y);
            if (item && item !== lastSpacer) {
                currentApplet = item;
            }
        }

        if (currentApplet) {
            hideTimer.stop();
            tooltip.raise();
        }
    }

    onEntered: hideTimer.stop();

    onExited: hideTimer.restart()

    onCurrentAppletChanged: {
        if (!currentApplet) {
            hideTimer.start();
            return;
        }
    }

    onPressed: mouse => {
        // Need to set currentApplet here too, to make touch selection + drag
        // with with a touchscreen, because there are no entered events in that
        // case
        let item = currentLayout.childAt(mouse.x, mouse.y);
        // BUG 454095: Don't allow dragging lastSpacer as it's not a real applet
        if (!item || item == lastSpacer || item == addWidgetsButton) {
            configurationArea.currentApplet = null
            return;
        }
        tooltip.raise();
        hideTimer.stop();

        // We set the current applet being dragged as a property of placeHolder
        // to be able to read its properties from the LayoutManager
        appletsModel.insert(item.index, {applet: placeHolder});
        placeHolder.parent.inThickArea = item.inThickArea
        currentApplet = appletContainerComponent.createObject(dropArea, {applet: item.applet, x: item.x,
                                                                     y: item.y, z: 900,
                                                                     width: item.width, height: item.height, index: -1})
        placeHolder.parent.dragging = currentApplet
        configurationArea.draggedItemIndex = item.index
        appletsModel.remove(item.index)
        root.dragAndDropping = true

        if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            startDragOffset = mouse.y - currentApplet.y;
        } else {
            startDragOffset = mouse.x - currentApplet.x;
        }
    }

    onReleased: mouse => finishDragOperation()

    onCanceled: finishDragOperation()

    function finishDragOperation() {
        root.dragAndDropping = false
        if (!currentApplet) {
            return;
        }
        appletsModel.set(placeHolder.parent.index, {applet: currentApplet.applet})
        let newCurrentApplet = currentApplet.applet.parent
        newCurrentApplet.animateFrom(currentApplet.x, currentApplet.y)
        newCurrentApplet.dragging = null
        placeHolder.parent = this
        currentApplet.destroy()
        root.layoutManager.save()
    }

    Item {
        id: placeHolder
        property Item dragging
        property bool busy: false
        visible: configurationArea.containsMouse
        Layout.preferredWidth: configurationArea.currentApplet?.Layout.preferredWidth ?? 0
        Layout.preferredHeight: configurationArea.currentApplet?.Layout.preferredHeight ?? 0
        Layout.maximumWidth: configurationArea.currentApplet?.Layout.maximumWidth ?? 0
        Layout.maximumHeight: configurationArea.currentApplet?.Layout.maximumHeight ?? 0
        Layout.minimumWidth: configurationArea.currentApplet?.Layout.minimumWidth ?? 0
        Layout.minimumHeight: configurationArea.currentApplet?.Layout.minimumHeight ?? 0
        Layout.fillWidth: configurationArea.currentApplet?.Layout.fillWidth ?? false
        Layout.fillHeight: configurationArea.currentApplet?.Layout.fillHeight ?? false
    }

    Timer {
        id: hideTimer
        interval: Kirigami.Units.humanMoment
        onTriggered: configurationArea.currentApplet = null
    }

    Rectangle {
        id: handle

        x: configurationArea.currentApplet?.x ?? 0
        y: configurationArea.currentApplet?.y ?? 0
        width: configurationArea.currentApplet?.width ?? 0
        height: configurationArea.currentApplet?.height ?? 0

        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
        opacity: configurationArea.currentApplet && configurationArea.containsMouse ? 0.5 : 0

        Kirigami.Icon {
            visible: !root.dragAndDropping
            source: "transform-move"
            width: Math.min(parent.width, parent.height)
            height: width
            anchors.centerIn: parent
        }
        Behavior on x {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.PopupPlasmaWindow {
        id: tooltip
        visible: configurationArea.currentApplet && !root.dragAndDropping
        visualParent: configurationArea.currentApplet
        // Try to dodge the ruler, as we can't cover it since it's a layershell surface
        margin: configurationArea.Window.window?.lengthMode === 2 ? Kirigami.Units.gridUnit * 2 : 0
        width: mainItem.implicitWidth + leftPadding + rightPadding
        height: mainItem.implicitHeight + topPadding + bottomPadding

        popupDirection: switch (Plasmoid.location) {
            case PlasmaCore.Types.TopEdge:
                return Qt.BottomEdge
            case PlasmaCore.Types.LeftEdge:
                return Qt.RightEdge
            case PlasmaCore.Types.RightEdge:
                return Qt.LeftEdge
            default:
                return Qt.TopEdge
        }

        onVisualParentChanged: {
            if (visualParent) {
                tooltip.requestActivate()
                const thisPlasmoid = configurationArea.currentApplet.applet.plasmoid;
                thisPlasmoid.contextualActionsAboutToShow();
                alternativesButton.visible = thisPlasmoid.internalAction("alternatives")?.enabled ?? false;
                configureButton.visible = thisPlasmoid.internalAction("configure")?.enabled ?? false;
                label.text = thisPlasmoid.title;
            }
        }

        mainItem: MouseArea {
            enabled: tooltip.visible
            implicitWidth: handleButtons.width
            implicitHeight: handleButtons.height
            hoverEnabled: true
            onEntered: hideTimer.stop();
            onExited:  hideTimer.restart();

            ColumnLayout {
                id: handleButtons
                spacing: Kirigami.Units.smallSpacing

                PlasmaExtras.PlasmoidHeading {
                    leftPadding: Kirigami.Units.smallSpacing * 2
                    rightPadding: Kirigami.Units.smallSpacing * 2
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 15

                    contentItem: Kirigami.Heading {
                        id: label
                        level: 3
                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 3
                    }
                }

                PlasmaComponents3.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the top unless it's on
                    // a top panel
                    visible: tooltip.location !== PlasmaCore.Types.TopEdge
                             && (configurationArea.currentApplet?.applet.plasmoid.internalAction("remove")?.enabled ?? false)
                    icon.name: "edit-delete-remove-symbolic"
                    text: i18n("Remove")
                    onClicked: {
                        configurationArea.currentApplet.applet.plasmoid.internalAction("remove").trigger();
                        configurationArea.currentApplet = null;
                    }
                }
                PlasmaComponents3.ToolButton {
                    id: configureButton
                    Layout.fillWidth: true
                    icon.name: "configure"
                    text: i18n("Configure…")
                    visible: configurationArea.currentApplet?.applet.plasmoid.hasConfigurationInterface ?? false
                    onClicked: {
                        configurationArea.currentApplet.applet.plasmoid.internalAction("configure").trigger();
                        configurationArea.currentApplet = null;
                    }
                }
                PlasmaComponents3.ToolButton {
                    id: alternativesButton
                    Layout.fillWidth: true
                    icon.name: "widget-alternatives"
                    text: i18n("Show Alternatives…")
                    onClicked: {
                        configurationArea.currentApplet.applet.plasmoid.internalAction("alternatives").trigger();
                        // We keep this popup open for a brief interval of time instead
                        // of re-setting the currentApplet now so that the alternatives
                        // popup has time to gain focus. The hideTimer will close this
                        // dialog.
                    }
                }
                PlasmaComponents3.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the bottom for top panels
                    visible: tooltip.location === PlasmaCore.Types.TopEdge
                             && (configurationArea.currentApplet?.applet.plasmoid.internalAction("remove")?.enabled ?? false)
                    icon.name: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        configurationArea.currentApplet.applet.plasmoid.internalAction("remove").trigger();
                        configurationArea.currentApplet = null;
                    }
                }

                Kirigami.Heading {
                    Layout.fillWidth: true
                    visible: panelSpacerWidth.visible
                    text: i18n("Spacer width")
                    textFormat: Text.PlainText
                    level: 3
                    horizontalAlignment: Text.AlignHCenter
                }

                PlasmaComponents3.SpinBox {
                    id: panelSpacerWidth
                    editable: true
                    Layout.fillWidth: true
                    focus: !Kirigami.InputMethod.willShowOnActive
                    visible: configurationArea.currentApplet?.applet.plasmoid.pluginName === "org.kde.plasma.panelspacer"
                        && !configurationArea.currentApplet.applet.plasmoid.configuration.expanding
                    from: 0
                    stepSize: 10
                    to: root.width
                    value: configurationArea.currentApplet?.applet.plasmoid.configuration.length ?? 0
                    onValueModified: {
                        configurationArea.currentApplet.applet.plasmoid.configuration.length = value
                    }
                }

                PlasmaComponents3.Switch {
                    text: i18nc("@option:check Whether to allow a spacer widget to fill available space", "Flexible size")
                    visible: configurationArea.currentApplet?.applet.plasmoid.pluginName === "org.kde.plasma.panelspacer"
                    checked: configurationArea.currentApplet?.applet.plasmoid.configuration.expanding ?? false
                    onToggled: configurationArea.currentApplet.applet.plasmoid.configuration.expanding = checked
                    Layout.fillWidth: true
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                }

            }
        }
    }
}
