/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

QQC2.ItemDelegate {
    id: root

    property bool showExpandButton: true

    readonly property bool selected: highlighted || down

    highlighted: false
    // If it's the only one in the list, clicking it won't do anything, so don't provide any visual feedback.
    hoverEnabled: showExpandButton
    down: showExpandButton ? undefined : false

    width: shortcutsList.width
    action: QQC2.Action {
        id: expandAction
        onTriggered: {
            shortcutsList.selectedIndex = (root.state === "expanded") ? -1 : index;
        }
    }
    Accessible.name: root.state === "expanded" ? i18n("Editing shortcut: %1", displayLabel.text) : displayLabel.text + keySequenceList.text
    contentItem: ColumnLayout {
        clip: true

        Kirigami.Theme.textColor: root.selected ? root.Kirigami.Theme.highlightedTextColor : undefined
        Kirigami.Theme.inherit: !root.selected

        spacing: Kirigami.Units.smallSpacing

        Item {
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: topRow.implicitHeight
            Layout.fillWidth: true
            RowLayout {
                id: topRow
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing
                Kirigami.Heading {
                    id: displayLabel
                    text: i18nc("%1 is the name action that is triggered by the key sequences following after :", "%1:", model.display)
                    textFormat: Text.PlainText
                    level: 5
                }
                QQC2.Label {
                    id: keySequenceList
                    Layout.fillWidth: true

                    color: {
                        if (root.selected) {
                            return Kirigami.Theme.highlightedTextColor;
                        } else if (model?.activeShortcuts?.length !== 0) {
                            return Kirigami.Theme.textColor;
                        } else {
                            return Kirigami.Theme.disabledTextColor;
                        }
                    }

                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                    text: {
                        if (model?.activeShortcuts?.length !== 0) {
                            return model?.activeShortcuts?.map(s => kcm.keySequenceToString(s)).join(", ")
                        } else {
                            return i18n("No active shortcuts")
                        }
                    }
                    textFormat: Text.PlainText
                }
                Rectangle {
                    id: defaultIndicator
                    radius: width * 0.5
                    implicitWidth: Kirigami.Units.largeSpacing
                    implicitHeight: Kirigami.Units.largeSpacing
                    visible: kcm.defaultsIndicatorsVisible
                    opacity: !model.isDefault
                    color: Kirigami.Theme.neutralTextColor
                }
                QQC2.ToolButton {
                    Layout.alignment: Qt.AlignRight
                    id: expandButton
                    visible: root.showExpandButton
                    icon.name: "expand"
                    activeFocusOnTab: false
                    onClicked: expandAction.trigger()
                }
            }
        }
        Loader {
            id: editLoader
            active: false
            visible: false
            Layout.fillWidth: true
            sourceComponent: RowLayout {
                readonly property var originalIndex: {
                    const concatenatedIndex = kcm.filteredModel.mapToSource(dm.modelIndex(index))
                    return kcm.shortcutsModel.mapToSource(concatenatedIndex)
                }
                spacing: 0

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width * 0.5
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.Heading {
                        level: 4
                        text: model.defaultShortcuts && model.defaultShortcuts.length !== 0
                            ? i18ncp("%1 decides if singular or plural will be used", "Default shortcut",
                                "Default shortcuts", model.defaultShortcuts.length)
                            : i18n("No default shortcuts")
                        textFormat: Text.PlainText
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    Repeater {
                        model: defaultShortcuts
                        QQC2.CheckBox {
                            Accessible.name: checked ? i18n("Default shortcut %1 is enabled.", modelData) : i18n("Default shortcut %1 is disabled.", modelData)
                            checked: activeShortcuts.indexOf(modelData) !== -1
                            text: modelData
                            onToggled: {
                                if (checked) {
                                     kcm.requestKeySequence(this, originalIndex, modelData)
                                } else {
                                    originalIndex.model.disableShortcut(originalIndex, modelData)
                                }
                            }
                            KCMUtils.SettingHighlighter {
                                highlight: !checked
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.Heading {
                        level: 4
                        Layout.alignment: Qt.AlignRight
                        text: i18n("Custom shortcuts")
                        textFormat: Text.PlainText
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    Repeater {
                        model: customShortcuts
                        RowLayout {
                            spacing: Kirigami.Units.smallSpacing
                            Layout.alignment: Qt.AlignRight
                            KQuickControls.KeySequenceItem {
                                Layout.alignment: Qt.AlignRight
                                keySequence: modelData
                                showClearButton: false
                                modifierOnlyAllowed: true
                                multiKeyShortcutsAllowed: supportsMultipleKeys
                                checkForConflictsAgainst: KQuickControls.ShortcutType.None
                                onCaptureFinished: {
                                    kcm.requestKeySequence(this, originalIndex, keySequence, modelData)
                                }
                                KCMUtils.SettingHighlighter {
                                    highlight: true
                                }
                            }
                            QQC2.Button {
                                icon.name: "edit-delete"
                                onClicked: originalIndex.model.disableShortcut(originalIndex, modelData)
                                QQC2.ToolTip {
                                    text: i18n("Delete this shortcut")
                                }
                            }
                        }
                    }
                    QQC2.Button {
                        text: i18n("Add custom shortcut")
                        icon.name: "list-add-symbolic"
                        Layout.alignment: Qt.AlignRight
                        onClicked: {
                            visible = false;
                            const newKeySequenceItem = newKeySequenceComponent.createObject(parent);
                            newKeySequenceItem.captureFinished.connect(() => {
                                newKeySequenceItem.destroy();
                                visible = true;
                            });
                            newKeySequenceItem.startCapturing();
                        }
                    }
                    Component {
                        id: newKeySequenceComponent
                        RowLayout {
                            id: keySequenceComponent

                            readonly property alias keySequenceItem: keySequenceItem

                            function startCapturing(): void {
                                keySequenceItem.startCapturing();
                            }

                            signal captureFinished()

                            Layout.alignment: Qt.AlignRight
                            spacing: Kirigami.Units.smallSpacing

                            KQuickControls.KeySequenceItem {
                                id: keySequenceItem

                                showClearButton: false
                                modifierOnlyAllowed: true
                                multiKeyShortcutsAllowed: model.supportsMultipleKeys
                                checkForConflictsAgainst: KQuickControls.ShortcutType.None

                                onCaptureFinished: {
                                    kcm.requestKeySequence(this, originalIndex, keySequence)
                                    keySequenceComponent.captureFinished()
                                }
                            }
                            QQC2.Button {
                                icon.name: "dialog-cancel"
                                onClicked: keySequenceComponent.captureFinished()
                                QQC2.ToolTip {
                                    text: i18n("Cancel capturing of new shortcut")
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    states: [
        State {
            name: "expanded"
            when: shortcutsList.selectedIndex === index || shortcutsList.count === 1
            PropertyChanges {
                target: root
                hoverEnabled: false
            }
            PropertyChanges {
                target: displayLabel
                level: 3
                Layout.fillWidth: true
            }
            PropertyChanges {
                target: keySequenceList
                visible: false
            }
            PropertyChanges {
                target: expandButton
                icon.name: "collapse"
            }
            PropertyChanges {
                target: editLoader
                active: true
                visible: true
            }
        }
    ]
    Behavior on height {
        NumberAnimation {
            properties: "height"
            duration: Kirigami.Units.shortDuration
        }
    }
}
