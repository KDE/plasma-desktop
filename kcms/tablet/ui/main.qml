/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM
import org.kde.kcmutils
import org.kde.kquickcontrols

SimpleKCM {
    id: root

    property bool testerWindowOpen: false
    property KCM.InputDevice device
    property KCM.InputDevice padDevice

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    // So it doesn't scroll while dragging
    flickable.interactive: Kirigami.Settings.hasTransientTouchInput

    Kirigami.PlaceholderMessage {
        icon.name: "preferences-desktop-tablet"
        text: i18nd("kcm_tablet", "No drawing tablets found")
        explanation: i18n("Connect a drawing tablet")
        anchors.centerIn: parent
        visible: combo.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    actions: [
        Kirigami.Action {
            text: i18ndc("kcm_tablet", "Tests tablet functionality like the pen", "Test Tablet…")
            icon.name: "tool_pen-symbolic"
            enabled: !root.testerWindowOpen && combo.count !== 0
            onTriggered: {
                const component = Qt.createComponent("Tester.qml");
                if (component.status === Component.Ready) {
                    const window = component.createObject(root, {tabletEvents: events});
                    window.showNormal();
                    window.closing.connect(close => {
                        root.testerWindowOpen = false;
                    });

                    root.testerWindowOpen = true;
                } else {
                    console.error(component.errorString());
                }
            }
        }
    ]

    function cancelCalibration(): void {
        if (root.calibrationWindowOpen) {
            calibrationWindow.close();
            calibrationWindow = null;
        }
    }

    Connections {
        target: combo.model

        function onDeviceRemoved(sysname: string): void {
            if (root.currentCalibrationSysName === sysname) {
                cancelCalibration();
            }
        }
    }

    ColumnLayout {
        id: form
        visible: combo.count > 0
        enabled: combo.count > 0

        Kirigami.FormLayout {
            visible: combo.count > 1

            Layout.fillWidth: true

            QQC2.ComboBox {
                id: combo
                Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The device we are configuring", "Device:")
                model: kcm.tabletsModel

                onCountChanged: {
                    // If a device is added, select that device first.
                    if (count > 0 && currentIndex < 0) {
                        currentIndex = 0;
                        return;
                    }

                    // If we had previously selected the last device (that was removed) then go back one.
                    if (currentIndex >= count) {
                        currentIndex = count - 1;
                    }
                }

                onCurrentIndexChanged: reload()

                function reload(): void {
                    root.device = kcm.tabletsModel.penAt(combo.currentIndex);
                    root.padDevice = kcm.tabletsModel.padAt(combo.currentIndex);

                    // If it is a pen device, prefer the display tab first.
                    // Otherwise, if it's a lonely pad then we only have the pad tab available.
                    if (root.device) {
                        displayAction.trigger();
                    } else if (root.padDevice) {
                        padAction.trigger();
                    }

                    if (displayAction.checked) {
                        contentLoader.item.reloadOutputView();
                    }
                }

                Connections {
                    target: kcm.tabletsModel

                    function onDeviceChanged(index: int): void {
                        if (index === combo.currentIndex) {
                            combo.reload();
                        }
                    }
                }

                Connections {
                    target: kcm

                    function onSettingsRestored(): void {
                        if (displayAction.checked) {
                            contentLoader.item.reloadOutputView();
                        }
                    }
                }
            }
        }

        Kirigami.NavigationTabBar {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window

            enabled: root.device || root.padDevice

            actions: [
                Kirigami.Action {
                    id: displayAction

                    icon.name: "input-tablet-symbolic"
                    text: i18nc("@title:tab", "Display")
                    visible: root.device
                    checked: contentLoader.sourceComponent === displayTab
                    onTriggered: {
                        contentLoader.sourceComponent = displayTab;
                        contentLoader.item.reloadOutputView(); // When switching between tabs, the visual rect comes out of sync so we need to correct it again.
                        checked = true;
                    }
                },
                Kirigami.Action {
                    id: penAction

                    icon.name: "tool_pen-symbolic"
                    text: i18nc("@title:tab", "Pen")
                    visible: root.device
                    checked: contentLoader.sourceComponent === stylusTab
                    onTriggered: {
                        contentLoader.sourceComponent = stylusTab;
                        checked = true;
                    }
                },
                Kirigami.Action {
                    id: padAction

                    icon.name: "tablet-symbolic"
                    text: i18nc("@title:tab", "Pad")
                    visible: root.padDevice
                    checked: contentLoader.sourceComponent === padTab
                    onTriggered: {
                        contentLoader.sourceComponent = padTab;
                        checked = true;
                    }
                }
            ]

            Layout.fillWidth: true
        }

        Loader {
            id: contentLoader

            active: root.device || root.padDevice

            Layout.fillWidth: true
            Layout.fillHeight: true

            KCM.TabletEvents {
                id: events

                anchors.fill: parent
            }
        }
    }

    Component {
        id: displayTab

        DisplayTab {
            id: tab

            device: root.device

            Connections {
                target: kcm
                function onSettingsRestored(): void {
                    tab.settingsRestored();
                }
            }
        }
    }

    Component {
        id: stylusTab

        StylusTab {
            id: tab

            db: kcm.db
            device: root.device
            tabletEvents: events

            Connections {
                target: kcm
                function onSettingsRestored(): void {
                    tab.settingsRestored();
                }
            }
        }
    }

    Component {
        id: padTab

        PadTab {
            device: root.device
            padDevice: root.padDevice
            tabletEvents: events
        }
    }
}
