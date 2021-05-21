/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as QQC2

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kquickcontrols 2.0
import org.kde.kcm 1.5 as KCM
import org.kde.private.kcms.keys 2.0 as Private


Kirigami.AbstractListItem {
    id: root
    highlighted: false
    hoverEnabled: true
    width: shortcutsList.width
    action: QQC2.Action {
        id: expandAction
        onTriggered: root.state == 'expanded' ?  shortcutsList.selectedIndex = -1 : shortcutsList.selectedIndex = index
    }
    Accessible.name: root.state == 'expanded' ? i18n("Editing shortcut: %1", displayLabel.text) : displayLabel.text + keySequenceList.text
    contentItem: ColumnLayout {
        clip: true
        Item {
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: topRow.implicitHeight
            Layout.fillWidth: true
            RowLayout {
                id: topRow
                anchors.fill: parent
                Kirigami.Heading {
                    id: displayLabel
                    text: i18nc("%1 is the name action that is triggered by the key sequences following after :", "%1:", model.display)
                    level: 5
                }
                QQC2.Label {
                    id: keySequenceList
                    Layout.fillWidth: true
                    color: model.activeShortcuts.length != 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                    text: {
                        if (model.activeShortcuts.length != 0) {
                            return model.activeShortcuts.map(s => kcm.keySequenceToString(s)).join(", ")
                        } else {
                            return i18n("No active shortcuts")
                        }
                    }
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
                readonly property var originalIndex : {
                    const concatenatedIndex = kcm.filteredModel.mapToSource(dm.modelIndex(index))
                    return kcm.shortcutsModel.mapToSource(concatenatedIndex)
                }
                spacing: 0
                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: parent.width * 0.5
                    Kirigami.Heading {
                        level: 4
                        text: model.defaultShortcuts &&  model.defaultShortcuts.length != 0 ? 
                            i18ncp("%1 decides if singular or plural will be used", "Default shortcut",
                            "Default shortcuts", model.defaultShortcuts.length) :
                            i18n("No default shortcuts")
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    Repeater {
                        model: defaultShortcuts
                        QQC2.CheckBox {
                            Accessible.name: checked ? i18n("Default shortcut %1 is enabled.", modelData) : i18n("Default shortcut %1 is disabled.", modelData)
                            checked: activeShortcuts.indexOf(modelData) != -1
                            text: modelData
                            onToggled: {
                                if (checked) {
                                     kcm.requestKeySequence(this, originalIndex, modelData)
                                } else {
                                    originalIndex.model.disableShortcut(originalIndex, modelData)
                                }
                            }
                            KCM.SettingHighlighter {
                                highlight: !checked
                            }
                        }
                    }
                }
                ColumnLayout {
                    Layout.preferredWidth: parent.width * 0.5
                    Layout.alignment: Qt.AlignTop
                    Kirigami.Heading {
                        level: 4
                        text: i18n("Custom shortcuts")
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    Repeater {
                        model: customShortcuts
                        RowLayout {
                            KeySequenceItem {
                                keySequence: modelData
                                showClearButton: false
                                multiKeyShortcutsAllowed: supportsMultipleKeys
                                checkForConflictsAgainst: ShortcutType.None
                                onCaptureFinished: {
                                    kcm.requestKeySequence(this, originalIndex, keySequence, modelData)
                                }
                                KCM.SettingHighlighter {
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
                        icon.name: "list-add"
                        onClicked: {
                            this.visible = false
                            var newKeySequenceItem = newKeySequenceComponent.createObject(parent)
                            for (var i = 0; i < newKeySequenceItem.children.length; i++) {
                                if (newKeySequenceItem.children[i] instanceof KeySequenceItem) {
                                    var keySequenceItem = newKeySequenceItem.children[i]
                                }
                            }
                            newKeySequenceItem.finished.connect(() => {
                                newKeySequenceItem.destroy()
                                this.visible = true
                            })
                            keySequenceItem.startCapturing()
                        }
                    }
                    Component {
                        id: newKeySequenceComponent
                        RowLayout {
                            signal finished
                            KeySequenceItem {
                                showClearButton: false
                                multiKeyShortcutsAllowed: model.supportsMultipleKeys
                                checkForConflictsAgainst: ShortcutType.None
                                onCaptureFinished: {
                                    kcm.requestKeySequence(this, originalIndex, keySequence)
                                    parent.finished()
                                }
                            }
                            QQC2.Button {
                                icon.name: "dialog-cancel"
                                onClicked: parent.finished()
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
            when: shortcutsList.selectedIndex == index || shortcutsList.count == 1
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
