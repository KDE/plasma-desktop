/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
import org.kde.kitemmodels as KItemModels
import org.kde.kirigamiaddons.tableview as TableView
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

ColumnLayout {
    spacing: Kirigami.Units.smallSpacing

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop

        Kirigami.FormLayout {
            Kirigami.Separator {
                Kirigami.FormData.label: i18nc("@title:group", "Shortcuts for Switching Layout")
                Kirigami.FormData.isSection: true
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Kirigami.FormData.label: i18nc("@option:textbox", "Main shortcuts:")
                Layout.fillWidth: true

                // TODO: SettingStateBinding and default value
                QQC2.Button {
                    id: mainShortcutButton
                    icon.name: "configure-symbolic"
                    text: kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName)

                    onClicked: {
                        if (!kcm.keyboardSettings.resetOldXkbOptions) {
                            kcm.keyboardSettings.resetOldXkbOptions = true;
                        }

                        root.goToAdvanced(__internal.mainShortcutGroupName)
                    }
                }

                QQC2.Button {
                    icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                                                                                   "edit-clear-locationbar-ltr-symbolic"
                    onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.mainShortcutGroupName);
                }
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Kirigami.FormData.label: i18nc("@option:textbox", "3rd level shortcuts:")
                Layout.fillWidth: true

                // TODO: SettingStateBinding and default value
                QQC2.Button {
                    id: lvl3lShortcutButton
                    icon.name: "configure-symbolic"
                    text: kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName)

                    onClicked: {
                        if (!kcm.keyboardSettings.resetOldXkbOptions) {
                            kcm.keyboardSettings.resetOldXkbOptions = true;
                        }

                        root.goToAdvanced(__internal.lvl3ShortcutGroupName)
                    }
                }

                QQC2.Button {
                    icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                                                                                   "edit-clear-locationbar-ltr-symbolic"
                    onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.lvl3ShortcutGroupName)
                }
            }

            KQuickControls.KeySequenceItem {
                id: alternativeShortcut
                Kirigami.FormData.label: i18nc("@option:textbox", "Alternative shortcut:")
                modifierlessAllowed: false
                modifierOnlyAllowed: true

                keySequence: kcm.shortcutHelper.alternativeShortcut
                onKeySequenceModified: kcm.shortcutHelper.alternativeShortcut = keySequence

                Binding {
                    target: alternativeShortcut
                    property: "keySequence"
                    value: kcm.shortcutHelper.alternativeShortcut
                }

                KCM.SettingHighlighter {
                    target: alternativeShortcut
                    highlight: alternativeShortcut.keySequence !== kcm.shortcutHelper.defaultAlternativeShortcut()
                }
            }

            KQuickControls.KeySequenceItem {
                id: lastUsedShortcut
                Kirigami.FormData.label: i18nc("@option:textbox", "Last used shortcuts:")
                modifierlessAllowed: false
                modifierOnlyAllowed: true

                keySequence: kcm.shortcutHelper.lastUsedShortcut
                onKeySequenceModified: kcm.shortcutHelper.lastUsedShortcut = keySequence

                Binding {
                    target: lastUsedShortcut
                    property: "keySequence"
                    value: kcm.shortcutHelper.lastUsedShortcut
                }

                KCM.SettingHighlighter {
                    target: lastUsedShortcut
                    highlight: lastUsedShortcut.keySequence !== kcm.shortcutHelper.defaultLastUsedShortcut()
                }
            }

            QQC2.CheckBox {
                text: i18nc("@option:checkbox", "Show a popup on layout changes")
                checked: kcm.workspaceOptions.osdKbdLayoutChangedEnabled
                onToggled: kcm.workspaceOptions.osdKbdLayoutChangedEnabled = checked

                KCM.SettingStateBinding {
                    configObject: kcm.workspaceOptions
                    settingName: "osdKbdLayoutChangedEnabled"
                }
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Kirigami.Separator {
                Kirigami.FormData.label: i18nc("@title:group", "Switching Policy")
                Kirigami.FormData.isSection: true
            }

            Repeater {
                model: [
                    {
                        "text": i18nc("@option:radio", "Global"),
                        "key": "Global",
                    },
                    {
                        "text": i18nc("@option:radio", "Desktop"),
                        "key": "Desktop",
                    },
                    {
                        "text": i18nc("@option:radio", "Application"),
                        "key": "WinClass",
                    },
                    {
                        "text": i18nc("@option:radio", "Window"),
                        "key": "Window",
                    },
                ]

                delegate: QQC2.RadioButton {
                    text: modelData.text
                    checked: kcm.keyboardSettings.switchMode === modelData.key
                    onToggled: kcm.keyboardSettings.switchMode = modelData.key

                    KCM.SettingStateBinding {
                        configObject: kcm.keyboardSettings
                        settingName: "switchMode"
                    }
                }
            }
        }
    }

    QQC2.CheckBox {
        Layout.alignment: Qt.AlignLeft
        text: i18nc("@option:checkbox", "Configure Layouts")
        checked: kcm.keyboardSettings.configureLayouts
        onToggled: kcm.keyboardSettings.configureLayouts = checked

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "configureLayouts"
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        enabled: kcm.keyboardSettings.configureLayouts

        QQC2.Button {
            text: i18nc("@action:button", "Add")
            icon.name: "list-add-symbolic"
            onClicked: _cLayoutDialog.incubateObject(root, {}, Qt.Asynchronous)
        }

        QQC2.Button {
            text: i18nc("@action:button", "Remove")
            icon.name: "list-remove-symbolic"
            enabled: tableView.selectionModel.hasSelection
            onClicked: kcm.userLayoutModel.removeSelected()
        }

        QQC2.Button {
            text: i18nc("@action:button", "Move Up")
            icon.name: "arrow-up-symbolic"
            onClicked: kcm.userLayoutModel.moveSelectedLayouts(-1)
            enabled: {
                if (!tableView.selectionModel.hasSelection) {
                    return false;
                }

                let selected = Array(...tableView.selectionModel.selectedIndexes);
                selected.sort();

                return selected[0].row > 0;
            }
        }

        QQC2.Button {
            text: i18nc("@action:button", "Move Down")
            icon.name: "arrow-down-symbolic"
            onClicked: kcm.userLayoutModel.moveSelectedLayouts(1)
            enabled: {
                if (!tableView.selectionModel.hasSelection) {
                    return false;
                }

                let selected = Array(...tableView.selectionModel.selectedIndexes);
                selected.sort();

                return selected[selected.length - 1].row < kcm.userLayoutModel.rowCount() - 1;
            }
        }

        QQC2.Button {
            text: i18nc("@action:button", "Preview")
            icon.name: "input-keyboard-virtual-symbolic"
            enabled: tableView.selectionModel.hasSelection && tableView.selectionModel.selectedIndexes.length < 2
            onClicked: tableView.preview()
        }
    }

    Component {
        id: _cLayoutDialog

        LayoutDialog {
            visible: false

            Component.onCompleted: open()
            onAccepted: destroy()
            onRejected: destroy()
        }
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Component.onCompleted: {
            if (background) {
                background.visible = true
            }
        }

        TableView.ListTableView {
            id: tableView

            clip: true
            enabled: kcm.keyboardSettings.configureLayouts

            model: kcm?.userLayoutModel ?? []
            selectionModel: kcm?.userLayoutModel.selectionModel ?? undefined
            alternatingRows: false

            function preview() {
                const selected = selectionModel.selectedIndexes[0];
                const layout = model.data(selected, Qt.UserRole + 1);
                const variant = model.data(selected, Qt.UserRole + 3);
                const variantName = model.data(selected, Qt.UserRole + 4);

                kcm.requestPreview(kcm.keyboardSettings.keyboardModel, layout, variant, variantName);
            }

            headerComponents: [
                TableView.HeaderComponent {
                    width: 20
                    title: ""
                    textRole: "layout"
                    resizable: false
                    draggable: false

                    itemDelegate: Item {
                        Rectangle {
                            anchors.centerIn: parent
                            visible: layoutLoopingCheckBox.checked && (row + 1) > layoutLoopCountSpinBox.value
                            radius: width * 0.5
                            implicitWidth: Kirigami.Units.largeSpacing
                            implicitHeight: Kirigami.Units.largeSpacing
                            color: Kirigami.Theme.neutralTextColor
                        }
                    }
                },
                TableView.HeaderComponent {
                    width: 70
                    resizable: true
                    draggable: false
                    title: i18nc("@title:column", "Map")
                    textRole: "layout"
                },
                TableView.HeaderComponent {
                    width: 100
                    resizable: true
                    draggable: false
                    title: i18nc("@title:column", "Label")
                    textRole: "layout"

                    itemDelegate: Kirigami.Icon {
                        source: KCMKeyboard.Flags.getIcon(modelData)
                    }
                },
                TableView.HeaderComponent {
                    width: 150
                    title: i18nc("@title:column", "Layout")
                    textRole: "layoutName"
                },
                TableView.HeaderComponent {
                    width: 150
                    title: i18nc("@title:column", "Variant")
                    textRole: "variantName"
                },
                TableView.HeaderComponent {
                    width: 150
                    title: i18nc("@title:column", "Shortcut")
                    textRole: "shortcut"

                    itemDelegate: Item {
                        id: shortcutColumn

                        property bool editable: false

                        QQC2.Label {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignLeft
                            visible: !shortcutColumn.editable
                        }

                        MouseArea {
                            anchors.fill: parent
                            visible: !shortcutColumn.editable
                            propagateComposedEvents: true
                            onDoubleClicked: {
                                shortcutColumn.editable = true;
                                shortcutEdit.startCapturing();
                            }

                            onClicked: mouse => mouse.accepted = false
                        }

                        KQuickControls.KeySequenceItem {
                            id: shortcutEdit
                            anchors.verticalCenter: parent.verticalCenter
                            modifierlessAllowed: false
                            visible: shortcutColumn.editable
                            keySequence: modelData

                            // NOTE: Use cutom buttons to control shortcutColumn.editable mode
                            QQC2.Button {
                                icon.name: "dialog-ok-symbolic"

                                onClicked: {
                                    shortcutColumn.editable = false;
                                    if (shortcutEdit.keySequence !== model.shortcut) {
                                        model.shortcut = shortcutEdit.keySequence;
                                    }
                                }

                                Accessible.name: i18nc("@action:button", "Reassign shortcut")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.text: Accessible.name
                            }

                            // NOTE: Use cutom buttons to control shortcutColumn.editable mode
                            QQC2.Button {
                                icon.name: "dialog-cancel-symbolic"

                                onClicked: {
                                    shortcutColumn.editable = false;
                                    if (shortcutEdit.keySequence !== model.shortcut) {
                                        shortcutEdit.keySequence = model.shortcut;
                                    }
                                }

                                Accessible.name: i18nc("@action:button", "Cancel assignment")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.text: Accessible.name
                            }
                        }
                    }
                }
            ]
        }
    }

    QQC2.CheckBox {
        id: layoutLoopingCheckBox
        text: i18nc("@option:checkbox", "Spare layouts")
        onToggled: {
            const count = checked ? __internal.minimumAvailableLoops : __internal.noLooping;
            kcm.keyboardSettings.layoutLoopCount = count;
        }

        checked: {
            // If the configureLayouts is turned off or the available value is less than the minimum, then the option is disabled
            if (!kcm.keyboardSettings.configureLayouts || __internal.minimumAvailableLoops < __internal.minimumLoopingCount) {
                return false;
            }

            // If the number of layouts is not the default value
            // or the available number of layouts is greater than the limit from X.org, then enable the option
            return kcm.keyboardSettings.layoutLoopCount !== __internal.noLooping
                || __internal.minimumAvailableLoops >= kcm.maxGroupCount
        }

        enabled: {
            // If the configureLayouts is turned off, then the element is disabled
            if (!kcm.keyboardSettings.configureLayouts) {
                return false;
            }

            // If the number of layouts is within the range of acceptable values, then enable the element
            return __internal.minimumAvailableLoops >= __internal.minimumLoopingCount
                && __internal.minimumAvailableLoops < kcm.maxGroupCount
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.Label {
            // Why org.kde.desktop doesn't support opacity when Label is disabled?
            enabled: layoutLoopCountSpinBox.enabled
            opacity: enabled ? 1 : 0.6
            text: i18nc("@label:spinbox", "Main layout count:")
        }

        QQC2.SpinBox {
            id: layoutLoopCountSpinBox
            Layout.minimumWidth: Kirigami.Units.gridUnit * 4

            enabled: layoutLoopingCheckBox.checked && kcm.keyboardSettings.configureLayouts
            to: Math.max(from, __internal.minimumAvailableLoops)
            from: {
                if (layoutLoopingCheckBox.checked) {
                    return __internal.minimumLoopingCount;
                }

                return __internal.noLooping;
            }

            value: {
                if (!kcm.keyboardSettings.configureLayouts) {
                    return __internal.noLooping;
                }

                if (kcm.keyboardSettings.layoutLoopCount > to){
                    return to
                }

                return kcm.keyboardSettings.layoutLoopCount;
            }

            textFromValue: (value, locale) => value < 0 ? "" : Number(value).toLocaleString(locale, "f", 0)

            onValueModified: kcm.keyboardSettings.layoutLoopCount = value;
        }
    }

    // Update main & 3dLvl shortcuts text
    Connections {
        target: kcm.xkbOptionsModel

        function onDataChanged(): void {
            mainShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName);
            lvl3lShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName);
        }

        function onModelReset(): void {
            mainShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName);
            lvl3lShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName);
        }
    }

    QtObject {
        id: __internal

        readonly property string mainShortcutGroupName: "grp"
        readonly property string lvl3ShortcutGroupName: "lv3"

        // Default value for layoutLoopCount (e.g. we cannot use layout looping feature)
        readonly property int noLooping: -1

        // Minimum number of layouts that can be used with layout looping feature
        readonly property int minimumLoopingCount: 2

        // Minimum number of layouts currently available (X.org only allows to have 4 layouts)
        readonly property int minimumAvailableLoops: Math.min(kcm.maxGroupCount, tableView.rowCount - 1)
    }
}
