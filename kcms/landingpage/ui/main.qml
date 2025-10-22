/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019-2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

import org.kde.plasma.landingpage.kcm

KCMUtils.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 33

    Kirigami.FormLayout2 {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18n("Theme:")
                contentItem: RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: Kirigami.Units.largeSpacing

                    QQC2.ButtonGroup { id: themeGroup }

                    LookAndFeelBox {
                        id: lightLookAndFeelBox
                        packageId: kcm.globalsSettings.defaultLightLookAndFeel
                        checked: !kcm.globalsSettings.automaticLookAndFeel && kcm.globalsSettings.lookAndFeelPackage === kcm.globalsSettings.defaultLightLookAndFeel
                        group: themeGroup

                        availablePackages: LookAndFeelModel {
                            variant: LookAndFeel.Variant.Light
                        }

                        onToggled: {
                            kcm.globalsSettings.automaticLookAndFeel = false;
                            kcm.globalsSettings.lookAndFeelPackage = kcm.globalsSettings.defaultLightLookAndFeel;
                        }

                        onAccepted: (lnfId) => {
                            kcm.globalsSettings.defaultLightLookAndFeel = lnfId;
                        }

                        KCMUtils.SettingHighlighter {
                            highlight: kcm.globalsSettings.automaticLookAndFeel || kcm.globalsSettings.lookAndFeelPackage != kcm.defaultLookAndFeelPackage
                        }
                    }

                    LookAndFeelBox {
                        id: darkLookAndFeelBox
                        packageId: kcm.globalsSettings.defaultDarkLookAndFeel
                        checked: !kcm.globalsSettings.automaticLookAndFeel && kcm.globalsSettings.lookAndFeelPackage === kcm.globalsSettings.defaultDarkLookAndFeel
                        group: themeGroup

                        availablePackages: LookAndFeelModel {
                            variant: LookAndFeel.Variant.Dark
                        }

                        onToggled: {
                            kcm.globalsSettings.automaticLookAndFeel = false;
                            kcm.globalsSettings.lookAndFeelPackage = kcm.globalsSettings.defaultDarkLookAndFeel;
                        }

                        onAccepted: (lnfId) => {
                            kcm.globalsSettings.defaultDarkLookAndFeel = lnfId;
                        }

                        KCMUtils.SettingHighlighter {
                            highlight: kcm.globalsSettings.automaticLookAndFeel || kcm.globalsSettings.lookAndFeelPackage != kcm.defaultLookAndFeelPackage
                        }
                    }

                    Thumbnail {
                        text: i18nc("Switch between dark and light look and feel packages automatically", "Automatic")
                        checked: kcm.globalsSettings.automaticLookAndFeel
                        QQC2.ToolTip.text: i18nc("@info:tooltip 1 is the name of a light global theme, 2 is the name of a dark global theme", "Use “%1” during the day and “%2” at night", lightLookAndFeelBox.text, darkLookAndFeelBox.text)
                        QQC2.ToolTip.visible: autoLookAndFeelHoverHandler.hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ButtonGroup.group: themeGroup

                        preview: SplitView {
                            anchors.fill: parent
                            first: lightLookAndFeelBox.preview
                            second: darkLookAndFeelBox.preview
                            shutter: 0.15

                            Kirigami.Icon {
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: Kirigami.Units.smallSpacing
                                source: "lighttable"
                                isMask: true
                                color: "white"
                                width: Kirigami.Units.iconSizes.smallMedium
                                height: width
                            }
                        }

                        onToggled: {
                            kcm.globalsSettings.automaticLookAndFeel = true;
                        }

                        KCMUtils.SettingHighlighter {
                            highlight: kcm.globalsSettings.automaticLookAndFeel || kcm.globalsSettings.lookAndFeelPackage != kcm.defaultLookAndFeelPackage
                        }

                        HoverHandler { id: autoLookAndFeelHoverHandler }
                    }
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@title:group translate as short as possible", "More appearance settings:")
                contentItem: RowLayout {
                    id: appearanceButtonsRow
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.FormData.label: i18nc("@title:group translate as short as possible", "More appearance settings:")

                    MostUsedIcon {
                        id: wallpaperKCMButton
                        Layout.fillWidth: true
                        Layout.preferredWidth: 50 // 50% of the available width
                        kcmId: "kcm_wallpaper"
                        visible: kcmAction !== null
                    }

                    MostUsedIcon {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 50 // 50% of the available width
                        kcmId: "kcm_lookandfeel"
                        visible: kcmAction !== null
                    }
                }
            }
        }

        Kirigami.FormGroup {
            //title: i18n("Behavior")
            Kirigami.FormEntry {
                title: slider.Accessible.name
                contentItem: ColumnLayout {
                    // We want to show the slider in a logarithmic way. ie
                    // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
                    // 0 is a special case, which means "instant speed / no animations"
                    Layout.fillWidth: true

                    Kirigami.FormData.label: slider.Accessible.name
                    Kirigami.FormData.buddyFor: slider

                    QQC2.Slider {
                        id: slider
                        Layout.fillWidth: true
                        from: -4
                        to: 4
                        stepSize: 0.5
                        snapMode: QQC2.Slider.SnapAlways
                        onMoved: kcm.globalsSettings.animationDurationFactor =
                            (value === to) ? 0 : (1.0 / Math.pow(2, value))
                        value: (kcm.globalsSettings.animationDurationFactor === 0)
                            ? slider.to
                            : -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2))

                        Accessible.name: i18n("Animation speed:")

                        KCMUtils.SettingStateBinding {
                            configObject: kcm.globalsSettings
                            settingName: "animationDurationFactor"
                        }
                    }

                    RowLayout {
                        QQC2.Label {
                            text: i18nc("Animation speed", "Slow")
                            textFormat: Text.PlainText
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: i18nc("Animation speed", "Instant")
                            textFormat: Text.PlainText
                        }
                    }
                }
            }

            Kirigami.FormEntry {
                //title: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
                // Click behavior settings
                contentItem: ColumnLayout {
                    QQC2.ButtonGroup { id: singleClickGroup }
                    Kirigami.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
                    Kirigami.FormData.buddyFor: doubleClick

                    Layout.fillWidth: true
                    spacing: 0

                    QQC2.RadioButton {
                        id: doubleClick
                        text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                        checked: !kcm.globalsSettings.singleClick
                        onToggled: kcm.globalsSettings.singleClick = false
                        QQC2.ButtonGroup.group: singleClickGroup

                        Accessible.description: i18n("Open by double-clicking instead")

                        KCMUtils.SettingStateBinding {
                            configObject: kcm.globalsSettings
                            settingName: "singleClick"
                            extraEnabledConditions: singleClick.enabled
                        }
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                            doubleClick.indicator.width + doubleClick.spacing : padding
                        rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                            doubleClick.indicator.width + doubleClick.spacing : padding
                        text: doubleClick.Accessible.description
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        font: Kirigami.Theme.smallFont
                    }
                }
            }

            Kirigami.FormEntry {
                // Click behavior settings
                contentItem: ColumnLayout {
                    Kirigami.FormData.buddyFor: singleClick
                    QQC2.RadioButton {
                        id: singleClick
                        Layout.topMargin: Kirigami.Units.mediumSpacing
                        text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                        checked: kcm.globalsSettings.singleClick
                        onToggled: kcm.globalsSettings.singleClick = true
                        QQC2.ButtonGroup.group: singleClickGroup

                        Accessible.description: i18n("Select by clicking on item's selection marker")

                        KCMUtils.SettingStateBinding {
                            configObject: kcm.globalsSettings
                            settingName: "singleClick"
                        }
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                            singleClick.indicator.width + singleClick.spacing : padding
                        rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                            singleClick.indicator.width + singleClick.spacing : padding
                        text: singleClick.Accessible.description
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        font: Kirigami.Theme.smallFont
                    }
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@title:group translate as short as possible", "More behavior settings:")
                contentItem: MostUsedIcon {
                    Kirigami.FormData.label: i18nc("@title:group translate as short as possible", "More behavior settings:")
                    Layout.preferredWidth: wallpaperKCMButton.width
                    kcmId: "kcm_workspace"
                    visible: kcmAction !== null
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18n("Most used pages:")
                contentItem: GridLayout {
                    id: mostUsedGrid
                    Kirigami.FormData.label: i18n("Most used pages:")
                    Kirigami.FormData.buddyFor: mostUsedGrid.children[1]

                    visible: recentlyUsedRepeater.count > 0
                    Layout.fillWidth: true
                    rows: 3
                    columns: 2
                    rowSpacing: Kirigami.Units.smallSpacing
                    columnSpacing: Kirigami.Units.smallSpacing

                    Kirigami.SizeGroup {
                        id: mostUsedPagesSizeGroup
                        mode: Kirigami.SizeGroup.Width
                    }

                    Repeater {
                        id: recentlyUsedRepeater

                        model: kcm.mostUsedModel
                        delegate: MostUsedIcon {
                            required property var model
                            Layout.fillWidth: true

                            kcmIcon: model.decoration
                            kcmName: model.display
                            onClicked: kcm.openKCM(model.kcmPlugin)
                        }

                        onItemAdded: (index, item) => {
                            if (index === 0) {
                                // Set buddy once, for an item in the first row.
                                // No, it doesn't work as a binding on children list.
                                mostUsedGrid.Kirigami.FormData.buddyFor = item;
                            }
                            mostUsedPagesSizeGroup.items.push(item);
                        }
                    }
                }
            }
        }
    }
}
