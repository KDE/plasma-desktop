/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.private.kimpanel as Kimpanel
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: kimpanel

    property int visibleButtons: 0
    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    LayoutMirroring.enabled: !vertical && Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Layout.minimumWidth: vertical ? Kirigami.Units.iconSizes.small : mainLayout.implicitWidth
    Layout.minimumHeight: vertical ? mainLayout.implicitHeight : Kirigami.Units.iconSizes.small
    Layout.preferredWidth: vertical ? kimpanel.width : mainLayout.implicitWidth
    Layout.preferredHeight: !vertical ? kimpanel.height : mainLayout.implicitHeight

    preferredRepresentation: fullRepresentation

    Component.onCompleted: {
        timer.restart();
    }

    InputPanel { helper: helper }

    GridLayout {
        id: mainLayout

        rowSpacing: 0
        columnSpacing: 0
        anchors.fill: parent

        GridView {
            id: items

            Layout.alignment: Qt.AlignCenter

            interactive: false
            flow: kimpanel.vertical ? GridView.FlowLeftToRight : GridView.FlowTopToBottom

            // The icon size to display when not using the auto-scaling setting
            readonly property int smallIconSize: Kirigami.Units.iconSizes.smallMedium
            readonly property bool autoSize: Plasmoid.configuration.scaleIconsToFit

            readonly property int gridThickness: kimpanel.vertical ? kimpanel.width : kimpanel.height
            // Should change to 2 rows/columns on a 56px panel (in standard DPI)
            readonly property int rowsOrColumns: autoSize ? 1 : Math.max(1, Math.min(count, Math.floor(gridThickness / smallSizeCellLength)))

            // Add margins only if the panel is larger than a small icon (to avoid large gaps between tiny icons)
            readonly property int smallSizeCellLength: gridThickness < smallIconSize ? gridThickness : smallIconSize + Kirigami.Units.smallSpacing
            cellHeight: {
                if (kimpanel.vertical) {
                    return autoSize ? kimpanel.width : smallSizeCellLength
                } else {
                    return autoSize ? kimpanel.height : Math.floor(kimpanel.height / rowsOrColumns)
                }
            }
            cellWidth: {
                if (kimpanel.vertical) {
                    return autoSize ? kimpanel.width : Math.floor(kimpanel.width / rowsOrColumns)
                } else {
                    return autoSize ? kimpanel.height : smallSizeCellLength
                }
            }

            //depending on the form factor, we are calculating only one dimension, second is always the same as root/parent
            implicitHeight: kimpanel.vertical ? cellHeight * Math.ceil(count / rowsOrColumns) : kimpanel.height
            implicitWidth: !kimpanel.vertical ? cellWidth * Math.ceil(count / rowsOrColumns) : kimpanel.width

            readonly property int iconSize: {
                var size;
                if (autoSize) {
                    size = Math.min(implicitWidth / rowsOrColumns, implicitHeight / rowsOrColumns)
                } else {
                    size = Math.min(gridThickness, smallIconSize)
                }
                return Kirigami.Units.iconSizes.roundedIconSize(Math.min(size, Kirigami.Units.iconSizes.enormous))
            }

            model: ListModel {
                id: list
                dynamicRoles: true
            }

            delegate: Item {
                id: delegate

                required property string key
                required property string label
                required property string icon
                required property string tip
                required property string hint

                width: items.cellWidth
                height: items.cellHeight

                activeFocusOnTab: true

                Keys.onPressed: event => {
                    switch (event.key) {
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Select:
                        statusIcon.triggered(Qt.LeftButton);
                        break;
                    case Qt.Key_Menu:
                        statusIcon.triggered(Qt.RightButton);
                        break;
                    }
                }
                Accessible.name: statusIcon.label
                Accessible.description: statusIcon.tip
                Accessible.role: Accessible.Button

                StatusIcon {
                    id: statusIcon
                    anchors.centerIn: parent
                    Layout.alignment: Qt.AlignCenter

                    width: items.iconSize
                    height: items.iconSize
                    label: delegate.label
                    tip: delegate.tip
                    icon: delegate.icon
                    hint: delegate.hint

                    onTriggered: button => {
                        if (button === Qt.LeftButton) {
                            if (delegate.key === 'kimpanel-placeholder') {
                                return;
                            }
                            helper.triggerProperty(delegate.key);
                            actionMenu.visualParent = statusIcon;
                        } else {
                            contextMenu.open(statusIcon, {key: delegate.key, label: delegate.label});
                        }
                    }
                }
            }
        }
    }

    function hideAction(key) {
        // We must use assignment to change the configuration property,
        // otherwise it won't get notified.
        var hiddenList = Plasmoid.configuration.hiddenList;
        if (hiddenList.indexOf(key) === -1) {
            hiddenList.push(key);
            Plasmoid.configuration.hiddenList = hiddenList;
        }
        timer.restart();
    }

    function showAction(key) {
        // We must use assignment to change the configuration property,
        // otherwise it won't get notified.
        var hiddenList = Plasmoid.configuration.hiddenList;
        var index = hiddenList.indexOf(key);
        if (index !== -1) {
            hiddenList.splice(index, 1);
            Plasmoid.configuration.hiddenList = hiddenList;
        }
        timer.restart();
    }

    function showMenu(menu, menuData) {
        const actionList = [];
        for (const /*KimpanelProperty*/ prop of menuData) {
            actionList.push({
                actionId: prop.key,
                icon: prop.icon,
                text: prop.label,
                hint: prop.hint,
            });
        }
        if (actionList.length > 0) {
            menu.actionList = actionList;
            menu.open();
        }
    }

    ActionMenu {
        id: actionMenu
        onActionClicked: actionId => {
            helper.triggerProperty(actionId);
        }
    }

    ContextMenu {
        id: contextMenu
        helper: helper
        onShowAction: key => kimpanel.showAction(key)
        onHideAction: key => kimpanel.hideAction(key)
    }

    Timer {
        id: timer
        interval: 50
        onTriggered: {
            var data = helper.properties;
            var nodata = data.length == 0;
            var count = list.count;
            var c = 0, i;
            var hiddenActions = [];
            for (i = 0; i < data.length; i ++) {
                if (Plasmoid.configuration.hiddenList.indexOf(data[i].key) !== -1) {
                    hiddenActions.push({'key': data[i].key,
                                'icon': data[i].icon,
                                'label': data[i].label});
                } else {
                    c = c + 1;
                }
            }
            if (c < count) {
                list.remove(c, count - c);
            }
            kimpanel.visibleButtons = c;

            c = 0;
            for (i = 0; i < data.length; i ++) {
                if (Plasmoid.configuration.hiddenList.indexOf(data[i].key) !== -1) {
                    continue;
                }
                var itemData = {'key': data[i].key,
                                'icon': data[i].icon,
                                'label': data[i].label,
                                'tip': data[i].tip,
                                'hint': data[i].hint };
                if (c < count) {
                    list.set(c, itemData);
                } else {
                    list.append(itemData);
                }
                c = c + 1;
            }
            contextMenu.actionList = hiddenActions;

            // Add a place holder if there is nothing.
            if (list.count == 0 && !nodata) {
                var itemData = {'key': 'kimpanel-placeholder',
                                'icon': 'draw-freehand',
                                'label': i18nc("@label placeholder for missing label", "Input Method Panel"), // qmllint disable unqualified
                                'tip': '',
                                'hint': ''};
                list.append(itemData);
            }
        }
    }

    Kimpanel.Kimpanel {
        id: helper
    }

    Connections {
        target: helper
        function onPropertiesChanged() {
            timer.restart();
        }
        function onMenuTriggered(menu) {
            kimpanel.showMenu(actionMenu, menu);
        }
    }
}
