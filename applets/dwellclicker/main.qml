/*
 * SPDX-FileCopyrightText: 2026 Sebastian Sauer <dipesh@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.workspace.dbus as DBus
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    property int selectedButtonId: 0

    DBus.DBusServiceWatcher {
        id: dbusServiceWatcher
        busType: DBus.BusType.Session
        watchedService: "org.kde.KWin.DwellClicker"
        onRegisteredChanged: {
            root.selectedButtonId = dbusServiceWatcher.registered ? dbusProperties.button : 0
        }
    }

    DBus.Properties {
        id: dbusProperties
        busType: DBus.BusType.Session
        service: "org.kde.KWin.DwellClicker"
        iface: "org.kde.KWin.DwellClicker"

        // Proper call DBusProperties::(connectToPropertiesChangedSignal|disconnectFromPropertiesChangedSignal)
        // depending on the dbusServiceWatcher.registered state since DBusProperties has no enabled property.
        path: dbusServiceWatcher.registered ? "/org/kde/KWin/DwellClicker" : ""

        property int button: Number(properties.button)
        onButtonChanged: {
            root.selectedButtonId = button
        }
    }

    Plasmoid.status: dbusServiceWatcher.registered ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus
    Plasmoid.icon: "input-mouse-click-left"
    toolTipMainText: Plasmoid.title

    fullRepresentation: PlasmaExtras.Representation {
        id: dialogItem

        Layout.minimumWidth: Kirigami.Units.gridUnit * 12
        Layout.minimumHeight: Kirigami.Units.gridUnit * 10
        collapseMarginsHint: true

        header: PlasmaExtras.PlasmoidHeading {
            contentItem: PlasmaComponents3.Label {
                text: i18nc("@info:usagetip", "Choose the next type of click:")
                wrapMode: Text.Wrap
                padding: Kirigami.Units.smallSpacing
            }
        }

        contentItem: ColumnLayout {
            spacing: 0

            PlasmaComponents3.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListView {
                    id: listView
                    width: parent.width
                    enabled: dbusServiceWatcher.registered

                    Component.onCompleted: {
                        currentIndex = getIndexForActionId(root.selectedButtonId)
                    }

                    Connections {
                        target: root
                        function onSelectedButtonIdChanged() {
                            listView.currentIndex = listView.getIndexForActionId(root.selectedButtonId)
                        }
                    }

                    function getIndexForActionId(id) {
                        return id - 10;
                    }

                    readonly property var mouseActionModel: [
                        { name: i18nc("mouse button action", "Left-click"), actionIcon: "input-mouse-click-left", actionId: 10 },
                        { name: i18nc("mouse button action", "Double-click"), actionIcon: "input-mouse-click-left-double", actionId: 11 },
                        { name: i18nc("mouse button action", "Click-and-drag"), actionIcon: "input-mouse", actionId: 12 },
                        { name: i18nc("mouse button action", "Middle-click"), actionIcon: "input-mouse-click-middle", actionId: 13 },
                        { name: i18nc("mouse button action", "Right-click"), actionIcon: "input-mouse-click-right", actionId: 14 }
                    ]

                    model: listView.mouseActionModel

                    delegate: PlasmaComponents3.ItemDelegate {
                        required property int index
                        required property var modelData

                        icon.name: modelData.actionIcon
                        text: modelData.name
                        width: ListView.view.width
                        height: Kirigami.Units.gridUnit * 2
                        highlighted: ListView.isCurrentItem

                        onClicked: {
                            // If the item is already selected, unselect it by mapping to actionId 0 what
                            // equals NoButton. Otherwise select the clicked item and use its actionId.
                            const targetId = (listView.currentIndex === index) ? 0 : modelData.actionId

                            root.expanded = false
                            dbusProperties.properties.button = targetId
                        }
                    }
                }
            }
        }
    }
}
