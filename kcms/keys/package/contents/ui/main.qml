/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtCore
import QtQuick 2.15
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3 as QQC2
import QtQml 2.15
import QtQml.Models 2.3

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcm 1.4 as KCM
import org.kde.private.kcms.keys 2.0 as Private

KCM.AbstractKCM {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 33

    framedView: false

    // order must be in sync with ComponentType enum in basemodel.h
    readonly property var sectionNames: [i18n("Applications"), i18n("Commands"), i18n("System Settings"), i18n("Common Actions")]

    property alias exportActive: exportInfo.visible
    readonly property bool errorOccured: kcm.lastError != ""

    Connections {
        target: kcm
        function onShowComponent(row) {
            components.currentIndex = row
        }
    }

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: errorOccured
            text: kcm.lastError
            type: Kirigami.MessageType.Error
        }
        Kirigami.InlineMessage {
            id: exportWarning
            Layout.fillWidth: true
            text: i18n("Cannot export scheme while there are unsaved changes")
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
            type: Kirigami.MessageType.Information
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    iconName: "document-save"
                    text: i18n("Save scheme")
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

                Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                Layout.fillHeight: true

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

                    // Not using CheckableListItem despite having a checkbox because
                    // the list item is not always in a checkable state, so the checkbox
                    // does not always need to be visible, but CheckableListItem
                    // makes that assumption.
                    delegate: Kirigami.BasicListItem {
                        id: componentDelegate

                        height: deleteButton.height + (Kirigami.Units.smallSpacing * 2)

                        fadeContent: model.pendingDeletion

                        KeyNavigation.right: shortcutsList

                        icon.name: model.decoration
                        label: model.display

                        trailing: RowLayout {
                            spacing: Kirigami.Units.smallSpacing

                            QQC2.CheckBox {
                                checked: model.checked
                                visible: exportActive
                                onToggled: model.checked = checked
                            }
                            QQC2.Button {
                                id: editButton

                                implicitHeight: Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing
                                implicitWidth: implicitHeight

                                visible: model.section == Private.ComponentType.Command
                                         && !exportActive
                                         && !model.pendingDeletion
                                         && (componentDelegate.containsMouse || componentDelegate.ListView.isCurrentItem)
                                icon.name: "edit-rename"
                                onClicked: {
                                    addCommandDialog.editing = true;
                                    addCommandDialog.componentName = model.component;
                                    // for commands, Name == Exec
                                    addCommandDialog.oldExec = model.display;
                                    addCommandDialog.commandListItemDelegate = componentDelegate;
                                    addCommandDialog.open();
                                }
                                QQC2.ToolTip {
                                    text: i18nc("@tooltip:button %1 is the text of a custom command", "Edit command for %1", model.display)
                                }
                            }
                            QQC2.Button {
                                id: deleteButton

                                implicitHeight: Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing
                                implicitWidth: implicitHeight

                                visible: model.section != Private.ComponentType.CommonAction
                                         && !exportActive
                                         && !model.pendingDeletion
                                         && (componentDelegate.containsMouse || componentDelegate.ListView.isCurrentItem)
                                icon.name: "edit-delete"
                                onClicked: model.pendingDeletion = true
                                QQC2.ToolTip {
                                    text: i18n("Remove all shortcuts for %1", model.display)
                                }
                            }
                            QQC2.Button {
                                implicitHeight: Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing
                                implicitWidth: implicitHeight

                                visible: !exportActive && model.pendingDeletion
                                icon.name: "edit-undo"
                                onClicked: model.pendingDeletion = false
                                QQC2.ToolTip {
                                    text: i18n("Undo deletion")
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

    footer: RowLayout {
        enabled: !errorOccured

        GridLayout {
            id: addButtonsLayout
            // if the left-hand-side components view (which is bound to the width of this) is getting too wide, switch to vertical stack
            readonly property bool useStackedLayout: addAppButton.implicitWidth + addCommandButton.implicitWidth >= categoryList.width
            rows: 2
            columns: 2
            flow: useStackedLayout ? GridLayout.TopToBottom : GridLayout.LeftToRight
            Layout.alignment: Qt.AlignRight
            Layout.maximumWidth: categoryList.width - (root.margins * 2)

            QQC2.Button {
                id: addAppButton
                Layout.fillWidth: true
                enabled: !exportActive
                icon.name: "list-add"
                text: i18nc("@action:button Keep translated text as short as possible", "Add Application…")
                onClicked: {
                    kcm.addApplication(this)
                }
            }
            QQC2.Button {
                id: addCommandButton
                Layout.fillWidth: true
                enabled: !exportActive
                icon.name: "list-add"
                text: i18nc("@action:button Keep translated text as short as possible", "Add Command…")
                onClicked: {
                    addCommandDialog.open()
                }
            }
        }

        // To tighten up the button groups
        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            spacing: Kirigami.Units.smallSpacing

            QQC2.Button {
                enabled: !exportActive
                icon.name: "document-import"
                text: i18n("Import Scheme…")
                onClicked: importSheet.open()
            }
            QQC2.Button {
                icon.name: exportActive ? "dialog-cancel" : "document-export"
                text: exportActive ? i18n("Cancel Export") : i18n("Export Scheme…")
                onClicked: {
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
        property Item commandListItemDelegate: null

        width: Math.max(root.width / 2, Kirigami.Units.gridUnit * 24)

        title: editing ? i18n("Edit Command") : i18n("Add Command")

        onVisibleChanged: {
            if (visible) {
                cmdField.clear();
                cmdField.forceActiveFocus();
                if (editing) {
                    cmdField.text = oldExec;
                }
            }
        }

        property Kirigami.Action addCommandAction: Kirigami.Action {
            text: addCommandDialog.editing ? i18n("Save") : i18n("Add")
            icon.name: addCommandDialog.editing ? "dialog-ok" : "list-add"
            onTriggered: {
                if (addCommandDialog.editing) {
                    const newLabel = kcm.editCommand(addCommandDialog.componentName, cmdField.text);
                    if (addCommandDialog.commandListItemDelegate) {
                        addCommandDialog.commandListItemDelegate.label = newLabel;
                    }
                } else {
                    kcm.addCommand(cmdField.text);
                }
                addCommandDialog.editing = false;
                addCommandDialog.close();
            }
        }

        standardButtons: Kirigami.Dialog.NoButton

        customFooterActions: [addCommandAction]

        ColumnLayout {
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Enter a command or choose a script file:")
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.TextField {
                    id: cmdField
                    Layout.fillWidth: true
                    font.family: "monospace"
                    onAccepted: addCommandDialog.addCommandAction.triggered()
                }
                QQC2.Button {
                    icon.name: "document-open"
                    text: i18nc("@action:button", "Choose…")
                    onClicked: {
                        openScriptFileDialogLoader.active = true
                    }
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
            nameFilters: [ i18nc("Template for file dialog","Script file (*.*sh)") ]
            Component.onCompleted: open()
            onAccepted: {
                cmdField.text = selectedFile
                cmdField.text = kcm.quoteUrl(selectedFile)
                openScriptFileDialogLoader.active = false
            }
            onRejected: openScriptFileDialogLoader.active = false
        }
    }

    Kirigami.OverlaySheet {
        id: importSheet

        title: i18n("Import Shortcut Scheme")

        ColumnLayout {
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Select the scheme to import:")
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.ComboBox {
                    id: schemeBox
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
                QQC2.Button {
                    text: schemeBox.customSchemeSelected ? i18n("Select File…") : i18n("Import")
                    onClicked: {
                        if (schemeBox.customSchemeSelected) {
                            shortcutSchemeFileDialogLoader.save = false;
                            shortcutSchemeFileDialogLoader.active = true;
                        } else {
                            kcm.loadScheme(schemeBox.model[schemeBox.currentIndex]["url"])
                            importSheet.close()
                        }
                    }
                }
            }
        }
    }
}

