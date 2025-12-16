/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>
    SPDX-FileCopyrightText: 2025 Yelsin Sepulveda <yelsin.sepulveda@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kwindowsystem
import org.kde.plasma.gamecontroller.kcm

KCMUtils.AbstractKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 45
    implicitHeight: Kirigami.Units.gridUnit * 25

    framedView: false

    // Shared device model - just the data source
    DeviceModel {
        id: sharedDeviceModel
    }

    // Shared selected device - this is what gets passed around
    property var sharedSelectedDevice: null

    property var elements: [
        {
            icon: "games-config-custom",
            title: i18nc("@title Category name in sidebar", "General Settings"),
            defaultnessKey: "generalSettingsIsDefaults"
        },
        {
            icon: "applications-games-symbolic",
            title: i18nc("@title Category name in sidebar", "Input Detection"),
            defaultnessKey: "inputDetectionIsDefaults"
        }
    ]

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        QQC2.ScrollView {
            id: leftSidePaneBackground
            Layout.fillHeight: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 13

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            ListView {
                id: listView
                activeFocusOnTab: true
                clip: true
                keyNavigationEnabled: true
                model: root.elements

                delegate: QQC2.ItemDelegate {
                    id: baseDelegate

                    required property int index
                    required property var modelData

                    width: listView.width

                    highlighted: listView.currentIndex === index

                    icon.name: modelData.icon
                    text: modelData.title
                    visible: modelData.available === undefined || modelData.available

                    onClicked: {
                        listView.currentIndex = index
                        listView.forceActiveFocus()
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        KD.IconTitleSubtitle {
                            Layout.fillWidth: true
                            icon.name: baseDelegate.icon.name
                            title: baseDelegate.text
                            selected: baseDelegate.highlighted || baseDelegate.down
                        }

                        Rectangle {
                            radius: width * 0.5
                            implicitWidth: Kirigami.Units.largeSpacing
                            implicitHeight: Kirigami.Units.largeSpacing
                            visible: kcm.defaultsIndicatorsVisible
                            opacity: !kcm[modelData.defaultnessKey]
                            color: Kirigami.Theme.neutralTextColor
                        }
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillHeight: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor

            QQC2.ScrollView {
                id: scrollView
                anchors.fill: parent

                Item {
                    id: containerItem
                    // Ensures we have correct margins on our content, which should
                    // fill the scrollView or scroll vertically when larger

                    readonly property int margins: Kirigami.Units.gridUnit

                    width: scrollView.availableWidth
                    height: Math.max(implicitHeight, scrollView.availableHeight)
                    // NOTE: No need to calculate implicitWidth, as we don't use it for sizing and
                    //       if present, the ScrollView will use it to show horizontal scroll bars
                    //implicitWidth: stackLayout.implicitWidth + margins * 2
                    implicitHeight: stackLayout.implicitHeight + margins * 2

                    StackLayout {
                        id: stackLayout
                        anchors.fill: parent
                        anchors.margins: containerItem.margins

                        currentIndex: listView.currentIndex

                        GeneralSettings {
                            deviceModel: sharedDeviceModel
                            // Two-way binding: GeneralSettings updates this, InputDetection reads it
                            selectedDevice: root.sharedSelectedDevice
                            onSelectedDeviceChanged: {
                                root.sharedSelectedDevice = selectedDevice
                            }
                        }
                        InputDetection {
                            deviceModel: sharedDeviceModel
                            device: root.sharedSelectedDevice
                        }                    }
                }
            }
        }
    }
}
