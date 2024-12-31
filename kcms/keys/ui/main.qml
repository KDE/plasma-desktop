/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQml
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.private.kcms.keys as Private

KCM.AbstractKCM {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 33

    framedView: false

    // order must be in sync with ComponentType enum in basemodel.h
    readonly property var sectionNames: [i18n("Applications"), i18n("Commands"), i18n("System Services"), i18n("Common Actions")]

    property alias exportActive: exportInfo.visible
    readonly property bool errorOccured: kcm.lastError !== ""

    Connections {
        target: kcm
        function onShowComponent(row) {
            components.currentIndex = row
        }
    }

    actions: [
        Kirigami.Action {
            enabled: !exportActive
            icon.name: "document-import-symbolic"
            text: i18nc("@action:button Import shortcut scheme", "Import…")
            // TODO: enable once Kirigami actions can have Accessible.name set (QT 6.8)
            // Accessible.name: i18nc("@action:button accessible", "Import shortcut scheme")
            onTriggered: importSheet.open()
        }, Kirigami.Action {
            icon.name: exportActive ? "dialog-cancel-symbolic" : "document-export-symbolic"
            text: exportActive
                  ? i18nc("@action:button", "Cancel Export")
                  : i18nc("@action:button Export shortcut scheme", "Export…")
            // TODO: enable once Kirigami actions can have Accessible.name set  (QT 6.8)
            // Accessible.name: exportActive
            //     ? text
            //     : i18nc("@action:button accessible", "Export shortcut scheme")
            onTriggered: {
                if (exportActive) {
                    exportActive = false
                } else if (kcm.needsSave) {
                    exportWarning.visible = true
                } else {
                    search.text = ""
                    exportActive = true
                }
            }
        }
    ]

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: ColumnLayout {
        spacing: 0

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: errorOccured
            text: kcm.lastError
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
        }
        Kirigami.InlineMessage {
            id: exportWarning
            Layout.fillWidth: true
            text: i18n("Cannot export scheme while there are unsaved changes")
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Warning
            showCloseButton: true
            Binding on visible {
                when: exportWarning.visible
                value: kcm.needsSave
                restoreMode: Binding.RestoreNone
            }
        }
        Kirigami.InlineMessage {
            id: exportInfo
            Layout.fillWidth: true
            text: i18n("Select the components below that should be included in the exported scheme")
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    icon.name: "document-export-symbolic"
                    text: i18nc("@action:button Export shortcut scheme", "Export")
                    // TODO: enable once Kirigami actions can have Accessible.name set  (QT 6.8)
                    // Accessible.name: i18nc("@action:button accessible", "Export shortcut scheme")

                    onTriggered: {
                        shortcutSchemeFileDialogLoader.save = true
                        shortcutSchemeFileDialogLoader.active = true
                        exportActive = false
                    }
                }
            ]
        }
        Kirigami.SearchField  {
            id: search
            enabled: !errorOccured && !exportActive
            Layout.fillWidth: true
            // Equal to the margins removed by disabling header padding
            Layout.margins: Kirigami.Units.mediumSpacing
            Binding {
                target: kcm.filteredModel
                property: "filter"
                value: search.text
                restoreMode: Binding.RestoreBinding
            }
        }
    }

    // Since we disabled the scroll views' frame and background, we're responsible
    // for setting the background color ourselves, because the background color
    // of the page it sits on top of doesn't have the right color for these views.
    Rectangle {
        anchors.fill: parent
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor

        RowLayout  {
            anchors.fill: parent
            enabled: !errorOccured
            spacing: 0

            QQC2.ScrollView {
                id: categoryList

                Layout.preferredWidth: Kirigami.Units.gridUnit * 14
                Layout.fillHeight: true
                clip: true

                ListView {
                    id: components
                    clip: true
                    model: kcm.filteredModel
                    activeFocusOnTab: true
                    add: Transition {
                        id: transition
                        PropertyAction {
                            target: components
                            property: "currentIndex"
                            value: transition.ViewTransition.index
                        }
                    }

                    headerPositioning: ListView.OverlayHeader
                    header: Kirigami.InlineViewHeader {
                        width: ListView.view.width
                        Kirigami.ActionToolBar {
                            alignment: Qt.AlignRight
                            enabled: !exportActive

                            actions: Kirigami.Action {
                                text: i18nc("@action:button Add new shortcut", "Add New")
                                icon.name: "list-add-symbolic"
                                displayHint: Kirigami.DisplayHint.KeepVisible

                                Kirigami.Action {
                                    icon.name: "applications-all-symbolic"
                                    text: i18nc("@action:menu End of the sentence 'Add New Application…'", "Application…")
                                    Accessible.name: i18nc("@action:menu accessible", "Add new application")
                                    Accessible.role: Accessible.MenuItem
                                    onTriggered: kcm.addApplication(root)
                                }

                                Kirigami.Action {
                                    icon.name: "scriptnew-symbolic"
                                    text: i18nc("@action:menu End of the sentence 'Add New Command or Script…'", "Command or Script…")
                                    Accessible.name: i18nc("@action:menu accessible", "Add new command or script")
                                    Accessible.role: Accessible.MenuItem
                                    onTriggered: addCommandDialog.open()
                                }
                            }
                        }
                    }

                    delegate: QQC2.ItemDelegate {
                        id: componentDelegate
                        width: ListView.view.width

                        text: model.display
                        icon.name: model.decoration

                        Accessible.onPressAction: clicked()
                        KeyNavigation.right: shortcutsList

                        onClicked: ListView.view.currentIndex = index
                        highlighted: ListView.isCurrentItem

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing
                            Kirigami.IconTitleSubtitle {
                                id: label

                                icon.name: componentDelegate.icon.name
                                title: componentDelegate.text
                                Layout.fillWidth: true
                                opacity: model.pendingDeletion ? 0.5 : 1.0
                                selected: componentDelegate.highlighted || componentDelegate.down
                            }

                            QQC2.CheckBox {
                                checked: model.checked
                                visible: exportActive
                                Accessible.name: i18nc("@option:check accessible export shortcuts %1 is application/group name", "Export %1", model.display)
                                onToggled: model.checked = checked
                            }
                            QQC2.Button {
                                id: editButton

                                implicitHeight: label.implicitHeight
                                implicitWidth: implicitHeight

                                Accessible.name: i18nc("@action:button accessible %1 is the name of a custom command", "Edit command for %1", model.display)

                                visible: model.section === Private.ComponentType.Command
                                         && !exportActive
                                         && !model.pendingDeletion
                                         && (componentDelegate.hovered || componentDelegate.ListView.isCurrentItem)
                                icon.name: "edit-rename"
                                onClicked: {
                                    addCommandDialog.editing = true;
                                    addCommandDialog.componentName = model.component;
                                    addCommandDialog.name = model.display;
                                    addCommandDialog.oldExec = kcm.getCommand(model.component);
                                    addCommandDialog.commandListItemDelegate = componentDelegate;
                                    addCommandDialog.open();
                                }
                                QQC2.ToolTip {
                                    text: i18nc("@info:tooltip %1 is the text of a custom command", "Edit command for %1", model.display)
                                }
                            }
                            QQC2.Button {
                                id: deleteButton

                                implicitHeight: label.implicitHeight
                                implicitWidth: implicitHeight
                                Accessible.name: i18nc("@action:button accessible %1 is the name of a shortcut category", "Remove all shortcuts for %1", model.display)

                                visible: model.section !== Private.ComponentType.CommonAction
                                         && model.isRemovable
                                         && !exportActive
                                         && !model.pendingDeletion
                                         && (componentDelegate.hovered || componentDelegate.ListView.isCurrentItem)
                                icon.name: "edit-delete"
                                onClicked: model.pendingDeletion = true
                                QQC2.ToolTip {
                                    text: i18nc("@info:tooltip %1 is the text of a shortcut category", "Remove all shortcuts for %1", model.display)
                                }
                            }
                            QQC2.Button {
                                implicitHeight: label.implicitHeight
                                implicitWidth: implicitHeight

                                Accessible.name: i18nc("@action:button accessible", "Undo deletion")
                                visible: !exportActive && model.pendingDeletion
                                icon.name: "edit-undo"
                                onClicked: model.pendingDeletion = false
                                QQC2.ToolTip {
                                    text: i18nc("@info:tooltip", "Undo deletion")
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
                        }
                    }

                    section.property: "section"
                    section.delegate: Kirigami.ListSectionHeader {
                        label: root.sectionNames[section]
                        width:  components.width
                        QQC2.CheckBox {
                            id: sectionCheckbox
                            Layout.alignment: Qt.AlignRight
                            // width of indicator + layout spacing
                            Layout.rightMargin: kcm.defaultsIndicatorsVisible ? Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing : 0
                            visible: exportActive
                            onToggled: {
                                const checked = sectionCheckbox.checked
                                const startIndex = kcm.shortcutsModel.index(0, 0)
                                const indices = kcm.shortcutsModel.match(startIndex, Private.BaseModel.SectionRole, section, -1, Qt.MatchExactly)
                                for (const index of indices) {
                                    kcm.shortcutsModel.setData(index, checked, Private.BaseModel.CheckedRole)
                                }
                            }
                            Connections {
                                enabled: exportActive
                                target: kcm.shortcutsModel
                                function onDataChanged (topLeft, bottomRight, roles) {
                                    const startIndex = kcm.shortcutsModel.index(0, 0)
                                    const indices = kcm.shortcutsModel.match(startIndex, Private.BaseModel.SectionRole, section, -1, Qt.MatchExactly)
                                    sectionCheckbox.checked = indices.reduce((acc, index) => acc && kcm.shortcutsModel.data(index, Private.BaseModel.CheckedRole), true)
                                }
                            }
                        }
                    }

                    onCurrentItemChanged: dm.rootIndex = kcm.filteredModel.index(currentIndex, 0)
                    onCurrentIndexChanged: {
                        shortcutsList.selectedIndex = -1;
                    }

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: components.count === 0 && search.text.length > 0
                        text: i18n("No items matched the search terms")
                    }
                }
            }

            Kirigami.Separator {
                Layout.fillHeight: true
            }

            QQC2.ScrollView  {
                enabled: !exportActive
                id: shortcutsScroll
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true

                ListView {
                    clip:true
                    id: shortcutsList
                    property int selectedIndex: -1
                    model: DelegateModel {
                        id: dm
                        model: rootIndex.valid ?  kcm.filteredModel : undefined
                        delegate: ShortcutActionDelegate {
                            showExpandButton: shortcutsList.count > 1
                        }
                        KeyNavigation.left: components
                    }

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: components.currentIndex == -1
                        text: i18n("Select an item from the list to view its shortcuts here")
                    }
                }
            }
        }
    }

    Loader {
        id: shortcutSchemeFileDialogLoader
        active: false
        property bool save
        sourceComponent: FileDialog {
            title: save ? i18n("Export Shortcut Scheme") : i18n("Import Shortcut Scheme")
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            nameFilters: [ i18nc("Template for file dialog","Shortcut Scheme (*.kksrc)") ]
            defaultSuffix: ".kksrc"
            fileMode: shortcutSchemeFileDialogLoader.save ? FileDialog.SaveFile : FileDialog.OpenFile
            Component.onCompleted: open()
            onAccepted: {
                if (save) {
                    kcm.writeScheme(selectedFile)
                } else {
                    var schemes = schemeBox.model
                    schemes.splice(schemes.length - 1, 0, {name: kcm.urlFilename(selectedFile), url: selectedFile})
                    schemeBox.model = schemes
                    schemeBox.currentIndex = schemes.length - 2
                }
                shortcutSchemeFileDialogLoader.active = false
            }
            onRejected: shortcutSchemeFileDialogLoader.active = false
        }
    }

    Kirigami.PromptDialog {
        id: addCommandDialog
        property bool editing: false
        property string componentName: ""
        property string oldExec: ""
        property string name: ""
        property Item commandListItemDelegate: null

        title: editing ? i18n("Edit Command") : i18n("Add Command")
        iconName: 'folder-script-symbolic'

        onVisibleChanged: {
            if (visible) {
                cmdField.clear();
                nameField.clear();
                cmdField.forceActiveFocus();
                if (editing) {
                    cmdField.text = oldExec;
                    nameField.text = name;
                    cmdField.selectAll();
                }
            }
        }
        onRejected: {
            if (addCommandDialog.editing) {
                addCommandDialog.editing = false;
            }
        }

        property Kirigami.Action addCommandAction: Kirigami.Action {
            text: addCommandDialog.editing ? i18n("Save") : i18n("Add")
            // TODO: enable once Kirigami actions can have Accessible.name set  (QT 6.8)
            // Accessible.Name: addCommandDialog.editing
            //                  ? i18nc("@action:button accessible", "Save command")
            //                  : i18nc("@action:button accessible", "Add command")
            icon.name: addCommandDialog.editing ? "dialog-ok" : "list-add"
            enabled: cmdField.length > 0
            onTriggered: {
                if (addCommandDialog.editing) {
                    const newLabel = kcm.editCommand(addCommandDialog.componentName, nameField.text, cmdField.text);
                    if (addCommandDialog.commandListItemDelegate) {
                        addCommandDialog.commandListItemDelegate.text = newLabel;
                    }
                } else {
                    kcm.addCommand(cmdField.text, nameField.text);
                }
                addCommandDialog.editing = false;
                addCommandDialog.close();
            }
        }

        standardButtons: Kirigami.Dialog.Cancel

        customFooterActions: [addCommandAction]

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Enter a command or choose a script file:")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            Kirigami.FormLayout {

                RowLayout {
                    Layout.fillWidth: true
                    Kirigami.FormData.label: i18nc("@label:textbox A shell command", "Command:")
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.TextField {
                        id: cmdField
                        Layout.fillWidth: true
                        Accessible.name: i18nc("@label:textbox accessible", "Command")

                        font.family: "monospace"
                        onAccepted: addCommandDialog.addCommandAction.triggered()
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        text: i18nc("@action:button", "Choose…")
                        Accessible.name: i18nc("@action:button accessible", "Choose script file")
                        onClicked: {
                            openScriptFileDialogLoader.active = true
                        }
                    }
                }
                QQC2.TextField {
                    id: nameField
                    Kirigami.FormData.label: i18nc("@label:textfield Human-readable name given to a shell command", "Name:")
                    placeholderText: i18nc("@info:placeholder", "Enter name here")

                    onAccepted: addCommandDialog.addCommandAction.triggered()
                }
            }
        }
    }

    Loader {
        id: openScriptFileDialogLoader
        active: false
        sourceComponent: FileDialog {
            title: i18nc("@title:window", "Choose Script File")
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            nameFilters: [ i18nc("Filename filters for file dialog. Do not translate any of the filename extensions","Script files (.sh, .bash, .zsh, etc.) (*.*sh)") ]
            Component.onCompleted: open()
            onAccepted: {
                cmdField.text = selectedFile
                cmdField.text = kcm.quoteUrl(selectedFile)
                nameField.text = kcm.findBaseName(cmdField.text)
                openScriptFileDialogLoader.active = false
            }
            onRejected: openScriptFileDialogLoader.active = false
        }
    }

    Kirigami.Dialog {
        id: importSheet

        title: i18n("Import Shortcut Scheme")

        width: Math.max(Math.round(root.width / 2), Kirigami.Units.gridUnit * 24)

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Select the scheme to import:")
                textFormat: Text.PlainText
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            QQC2.ComboBox {
                id: schemeBox

                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing

                readonly property bool customSchemeSelected: currentIndex == count - 1
                property string url: ""
                currentIndex: count - 1
                textRole: "name"
                onActivated: url = model[index]["url"]
                Component.onCompleted: {
                    var defaultSchemes = kcm.defaultSchemes()
                    defaultSchemes.push({name: i18n("Custom Scheme"), url: "unused"})
                    model = defaultSchemes
                }
            }
        }

         customFooterActions: [
             Kirigami.Action {
                text: schemeBox.customSchemeSelected ? i18n("Select File…") : i18n("Import")
                // TODO: enable once Kirigami actions can have Accessible.name set  (QT 6.8)
                // Accessible.Name: schemeBox.customSchemeSelected
                //                  ? i18nc("@action:button accessible", "Select shortcut scheme file")
                //                  : i18nc("@action:button accessible", "Add shortcut scheme")
                // Accessible.Role: Accessible.MenuItem

                onTriggered: {
                    if (schemeBox.customSchemeSelected) {
                        shortcutSchemeFileDialogLoader.save = false;
                        shortcutSchemeFileDialogLoader.active = true;
                    } else {
                        kcm.loadScheme(schemeBox.model[schemeBox.currentIndex]["url"])
                        importSheet.close()
                    }
                }
            }
        ]
    }
}

