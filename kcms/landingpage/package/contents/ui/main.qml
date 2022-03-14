/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019-2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.4 as KCM

KCM.SimpleKCM {
    id: root

    Kirigami.FormLayout {
        width: parent.width

        anchors {
            top: parent.top
            topMargin: Kirigami.Units.largeSpacing
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Theme:")

            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.gridUnit * 2

            QQC2.ButtonGroup { id: themeGroup } // needed?

            Thumbnail {
                imageSource: kcm.defaultLightLookAndFeel.thumbnail
                text: i18nc("Adjective; as in, 'light theme'", "Light")
                checked: kcm.globalsSettings.lookAndFeelPackage === kcm.defaultLightLookAndFeel.id
                QQC2.ButtonGroup.group: themeGroup

                onToggled: kcm.globalsSettings.lookAndFeelPackage = kcm.defaultLightLookAndFeel.id

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "lookAndFeelPackage"
                }
            }
            Thumbnail {
                imageSource: kcm.defaultDarkLookAndFeel.thumbnail
                text: i18nc("Adjective; as in, 'dark theme'", "Dark")
                checked: kcm.globalsSettings.lookAndFeelPackage === kcm.defaultDarkLookAndFeel.id
                QQC2.ButtonGroup.group: themeGroup

                onToggled: kcm.globalsSettings.lookAndFeelPackage = kcm.defaultDarkLookAndFeel.id

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "lookAndFeelPackage"
                }
            }
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case
        ColumnLayout {
            Kirigami.FormData.label: i18n("Animation speed:")
            Kirigami.FormData.buddyFor: slider

            // Match width of feedback slider for aesthetics
            Layout.minimumWidth: feedbackLoader.active ? feedbackLoader.item.sliderWidth : implicitWidth
            Layout.maximumWidth: feedbackLoader.active ? feedbackLoader.item.sliderWidth : implicitWidth

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 0.5
                snapMode: QQC2.Slider.SnapAlways
                onMoved: {
                    if(value === to) {
                        kcm.globalsSettings.animationDurationFactor = 0;
                    } else {
                        kcm.globalsSettings.animationDurationFactor = 1.0 / Math.pow(2, value);
                    }
                }
                value: if (kcm.globalsSettings.animationDurationFactor === 0) {
                    return slider.to;
                } else {
                    return -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2));
                }

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "animationDurationFactor"
                }
            }
            RowLayout {
                QQC2.Label {
                    text: i18nc("Animation speed", "Slow")
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Animation speed", "Instant")
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        RowLayout {
            QQC2.Button {
                icon.name: "preferences-desktop-wallpaper"
                text: i18n("Change Wallpaper…")
                onClicked: kcm.openWallpaperDialog()
            }

            QQC2.Button {
                // This button deliberately does not start with a verb to save space
                // so that translations don't overflow, as horizontal space is limited
                text: i18n("More Appearance Settings…")
                icon.name: "preferences-desktop-theme-global"
                onClicked: kcm.openKCM("kcm_lookandfeel")
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        // Click behavior settings
        QQC2.ButtonGroup { id: singleClickGroup }

        QQC2.RadioButton {
            id: singleClick
            Kirigami.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
            checked: kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = true
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Select by clicking on item's selection marker")
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        QQC2.RadioButton {
            id: doubleClick
            text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
            checked: !kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = false
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
                extraEnabledConditions: singleClick.enabled
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Open by double-clicking instead")
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // This is in a loader because the import FeedbackControls uses won't always exist being KUserFeedback optional
        Loader {
            id: feedbackLoader
            visible: item !== null
            active: kcm.hasOwnProperty("feedbackEnabled")
            Kirigami.FormData.label: item ? i18n("Send User Feedback:") : ""
            Kirigami.FormData.buddyFor: item ? item.slider : null
            source: Qt.resolvedUrl("FeedbackControls.qml")
        }

        Item {
            visible: feedbackLoader.visible
            Kirigami.FormData.isSection: false
        }

        QQC2.Button {
            // This button deliberately does not start with a verb to save space
            // so that translations don't overflow, as horizontal space is limited
            text: i18n("More Behavior Settings…")
            icon.name: "preferences-desktop"
            onClicked: kcm.openKCM("kcm_workspace")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        GridLayout {
            Kirigami.FormData.label: i18n("Most Used Pages:")
            Kirigami.FormData.buddyFor: children[1] // 0 is the Repeater

            Layout.fillWidth: true
            rows: 3
            columns: 2
            rowSpacing: 0
            columnSpacing: 0

            Repeater {
                id: recentlyUsedRepeater

                readonly property int widestButton: {
                    let currentWidest = 0;
                    for (let i = 0; i < count; i++) {
                        if (itemAt(i).implicitWidth > currentWidest) {
                            currentWidest = itemAt(i).implicitWidth;
                        }
                    }
                    return currentWidest;
                }

                model: kcm.mostUsedModel
                delegate: MostUsedIcon {
                    Layout.preferredWidth: recentlyUsedRepeater.widestButton
                    kcmIcon: model.decoration
                    kcmName: model.display
                    onClicked: kcm.openKCM(model.kcmPlugin)
                }
            }
        }
    }
}
