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

import org.kde.plasma.core as PlasmaCore

KCMUtils.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 33

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group for light/dark quick switch", "Theme:")
                contentItem: RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: Kirigami.Units.largeSpacing

                    QQC2.ButtonGroup { id: themeGroup }

                    LookAndFeelBox {
                        id: lightLookAndFeelBox

                        packageId: kcm.globalsSettings.defaultLightLookAndFeel
                        variant: LookAndFeel.Variant.Light

                        checked: !kcm.globalsSettings.automaticLookAndFeel && kcm.globalsSettings.lookAndFeelPackage === kcm.globalsSettings.defaultLightLookAndFeel
                        group: themeGroup

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
                        variant: LookAndFeel.Variant.Dark

                        checked: !kcm.globalsSettings.automaticLookAndFeel && kcm.globalsSettings.lookAndFeelPackage === kcm.globalsSettings.defaultDarkLookAndFeel
                        group: themeGroup

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

                    LookAndFeelBox {
                        id: automaticLookAndFeelBox
                        popupEnabled: false

                        group: themeGroup

                        text: i18nc("Switch between dark and light look and feel packages automatically", "Automatic")

                        checked: kcm.globalsSettings.automaticLookAndFeel
                        onToggled: kcm.globalsSettings.automaticLookAndFeel = true;

                        preview: SplitView {
                            first: lightLookAndFeelBox.previewImage
                            second: darkLookAndFeelBox.previewImage
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

                        KCMUtils.SettingHighlighter {
                            highlight: kcm.globalsSettings.automaticLookAndFeel || kcm.globalsSettings.lookAndFeelPackage != kcm.defaultLookAndFeelPackage
                        }

                        QQC2.ToolTip.text: i18nc("@info:tooltip 1 is the name of a light global theme, 2 is the name of a dark global theme", "Use “%1” during the day and “%2” at night", lightLookAndFeelBox.text, darkLookAndFeelBox.text)
                        QQC2.ToolTip.visible: automaticLookAndFeelBox.hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormEntry {
                title: i18nc("@title:group translate as short as possible", "More appearance settings:")
                contentItem: RowLayout {
                    id: appearanceButtonsRow
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

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

                        Accessible.name: i18nc("@title:slider", "Animation speed:")

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

            Kirigami.FormSeparator {}

            Kirigami.FormEntry {
                title: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
                subtitle: doubleClick.Accessible.description
                // Click behavior settings
                contentItem: QQC2.RadioButton {
                    id: doubleClick
                    text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                    checked: !kcm.globalsSettings.singleClick
                    onToggled: kcm.globalsSettings.singleClick = false
                    QQC2.ButtonGroup { id: singleClickGroup }
                    QQC2.ButtonGroup.group: singleClickGroup

                    Accessible.description: i18nc("@info:usagetip", "Open by double-clicking instead")

                    KCMUtils.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "singleClick"
                        extraEnabledConditions: singleClick.enabled
                    }
                }
            }

            Kirigami.FormEntry {
                // Click behavior settings
                subtitle: singleClick.Accessible.description
                contentItem: QQC2.RadioButton {
                    id: singleClick
                    Layout.topMargin: Kirigami.Units.mediumSpacing
                    text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                    checked: kcm.globalsSettings.singleClick
                    onToggled: kcm.globalsSettings.singleClick = true
                    QQC2.ButtonGroup.group: singleClickGroup

                    Accessible.description: i18nc("@info:usagetip", "Select by clicking on item's selection marker")

                    KCMUtils.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "singleClick"
                    }
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                title: i18nc("@title:group translate as short as possible", "More behavior settings:")
                action: Kirigami.Action {
                    readonly property PlasmaCore.Action kcmAction: kcm.kcmAction("kcm_workspace")
                    fromQAction: kcmAction
                    onTriggered: kcmAction.trigger();
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group for grid of buttons opening kcms", "Most used pages:")
                contentItem: GridLayout {
                    id: mostUsedGrid
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
