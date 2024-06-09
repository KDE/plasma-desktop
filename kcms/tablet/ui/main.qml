/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

AbstractKCM {
    id: root

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property var device

    Kirigami.PlaceholderMessage {
        icon.name: "input-tablet"
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
            onTriggered: {
                const component = Qt.createComponent("Tester.qml");
                if (component.status === Component.Ready) {
                    const window = component.createObject(root, {tabletEvents});
                    window.show();
                } else {
                    console.error(component.errorString());
                }
            }
        }
    ]

    Rectangle {
        // Custom background to match framedView bg
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor

        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        // Do not inherit from the parent
        Kirigami.Theme.inherit: false
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Kirigami.FormLayout {
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            visible: combo.count > 1

            QQC2.ComboBox {
                id: combo
                Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The device we are configuring", "Device:")
                model: kcm.toolsModel

                onCountChanged: if (count > 0 && currentIndex < 0) {
                    currentIndex = 0;
                }

                function reloadOutputView() {
                    const initialOutputArea = form.device.outputArea;
                    if (initialOutputArea === Qt.rect(0, 0, 1, 1)) {
                        outputAreaCombo.currentIndex = 0;
                    } else if (initialOutputArea.x === 0 && initialOutputArea.y === 0 && initialOutputArea.width === 1) {
                        outputAreaCombo.currentIndex = 1;
                    } else {
                        outputAreaCombo.currentIndex = 2;
                    }
                    keepAspectRatio.checked = tabletItem.aspectRatio === (form.device.size.width / form.device.size.height)
                    outputAreaView.resetOutputArea(outputAreaCombo.currentIndex, initialOutputArea)
                }

                onCurrentIndexChanged: {
                    root.device = kcm.toolsModel.deviceAt(combo.currentIndex)
                    reloadOutputView()
                }

                Connections {
                    target: kcm
                    function onSettingsRestored() {
                        outputAreaView.changed = false
                        combo.reloadOutputView()
                    }
                }
            }
        }

        Kirigami.NavigationTabBar {
            Layout.fillWidth: true
            currentIndex: 0

            actions: [
                Kirigami.Action {
                    icon.name: "input-tablet-symbolic"
                    text: i18nc("@title:tab", "General")
                    checked: contentLoader.sourceComponent === tabGeneral
                    onTriggered: contentLoader.sourceComponent = tabGeneral
                },
                Kirigami.Action {
                    icon.name: "tool_pen-symbolic"
                    text: i18nc("@title:tab", "Stylus")
                    checked: contentLoader.sourceComponent === tabStylus
                    onTriggered: contentLoader.sourceComponent = tabStylus
                },
                Kirigami.Action {
                    icon.name: "tablet-symbolic"
                    text: i18nc("@title:tab", "Tablet")
                    checked: contentLoader.sourceComponent === tabTablet
                    onTriggered: contentLoader.sourceComponent = tabTablet
                }
            ]
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Custom background to match framedView bg
            color: Kirigami.Theme.backgroundColor

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            // Do not inherit from the parent
            Kirigami.Theme.inherit: false

            Loader {
                id: contentLoader
                anchors.fill: parent

                asynchronous: true
                sourceComponent: tabGeneral
            }
        }
    }

    Component {
        id: tabGeneral

        TabGeneral {
            device: root.device
        }
    }

    Component {
        id: tabStylus

        TabStylus {
            device: root.device
        }
    }

    Component {
        id: tabTablet

        TabTablet {
            device: root.device
        }
    }
}
